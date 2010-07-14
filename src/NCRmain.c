#include <stdio.h>
#include <string.h>
#include <cm.h>
#include <ncr.h>

int main (int argc, char *argv []) {
	int argPos, argNum = argc, nThreads = 1;
	char *netName = (char *) NULL;
	size_t timeStepNum, timeStep, timeHour;
	NCRnetwork_t *network;
	void *runoff, *outflow;
	int dt = 6;
	int cellId;

	for (argPos = 1;argPos < argNum; ) {
		if (CMargTest (argv [argPos],"-n","--network")) {
			if ((argNum = CMargShiftLeft (argPos,argv,argNum)) <= argPos) {
				CMmsgPrint (CMmsgUsrError,"%s: Missing sampling coverage!\n",CMprgName (argv [0]));
				return (-1);
			}
			netName = argv [argPos];
			argNum = CMargShiftLeft (argPos,argv,argNum);
			continue;
		}
		if (CMargTest (argv [argPos],"-p","--threads")) {
			if ((argNum = CMargShiftLeft (argPos,argv,argNum)) <= argPos) {
				CMmsgPrint (CMmsgUsrError,"%s: Missing sampling coverage!\n",CMprgName (argv [0]));
				return (-1);
			}
			if (sscanf (argv [argPos],"%d",&nThreads) != 1) {
				CMmsgPrint (CMmsgUsrError,"%s: Invalid threads [%s]\n",CMprgName (argv [0]), argv [argPos]);
				return (-1);
			}
			argNum = CMargShiftLeft (argPos,argv,argNum);
			continue;
		}
		if (CMargTest (argv [argPos],"-h","--help")) {
			CMmsgPrint (CMmsgInfo,"%s [options] <nc grid> <nc grid>\n", CMprgName(argv [0]));
			CMmsgPrint (CMmsgInfo,"     -n,--network\n");
			CMmsgPrint (CMmsgInfo,"     -p,--threads\n");
			CMmsgPrint (CMmsgInfo,"     -h,--help\n");
			return (0);
		}
		if ((argv [argPos][0] == '-') && (strlen (argv [argPos]) > 1)) {
			fprintf (stderr,"Unknown option: %s!\n",argv [argPos]);
			return (-1);
		}
		argPos++;
	}
	if (argNum != 3) {
		CMmsgPrint (CMmsgInfo,"%s: %s!\n", CMprgName (argv [0]), argNum < 3 ? "Missing runoff" : "Extra arguments");
		return (-1);
	}

	CMmsgPrint (CMmsgInfo, "Loading cells\n");
	if ((network = NCRnetworkLoad (netName)) == (NCRnetwork_t *) NULL) return (-1);
	NCRroutingInitialize (network,(float) dt);

	CMmsgPrint (CMmsgInfo, "Opening runoff\n");
	if ((runoff = NCRinputOpen (network, argv [1], "runoff")) == (void *) NULL) {
		NCRnetworkFree (network);
		return (-1);
	}
	if ((timeStepNum = NCRinputTimeStepNum (runoff)) == 0) {
		NCRnetworkFree (network);
		NCRinputClose (runoff);
		return (-1);
	}

	CMmsgPrint (CMmsgInfo, "Opening outflow\n");
	if ((outflow = NCRoutputOpen (network, argv [2], "outflow")) == (void *) NULL) {
		NCRnetworkFree (network);
		return (-1);
	}
	
	CMmsgPrint (CMmsgInfo, "Start routing\n");
	if (nThreads == 1) {
		for (timeStep = 0;timeStep < timeStepNum; ++timeStep) {
			NCRinputLoad   (runoff,  timeStep, network);
			for (timeHour = 0;timeHour < 24;timeHour += dt)
				for (cellId = 0;cellId < network->CellNum; cellId++)
					NCRroutingFunc ((void *) network,(void *) NULL, cellId);
			if (NCRoutputWrite (outflow, timeStep, network) != true) goto Stop;
		}
	}
	else {
		size_t taskId, dLink;
		NCRnetworkCell_t *cell;
		CMthreadTeam_p team = CMthreadTeamCreate (nThreads);
		CMthreadJob_p  job;

		if ((job  = CMthreadJobCreate (team, (void *) network, network->CellNum, (CMthreadUserAllocFunc) NULL,NCRroutingFunc)) == (CMthreadJob_p) NULL) {
			CMmsgPrint (CMmsgAppError, "Job creation error in %s:%d\n",__FILE__,__LINE__);
			CMthreadTeamDestroy (team);
			goto Stop;
		}
		for (cellId = 0; cellId < network->CellNum; cellId++) {
			cell = network->Cells [cellId];
			taskId = network->CellNum - cellId - 1;
			dLink  = cell->ToCellId > 0 ? network->CellNum - cell->ToCellId : taskId;
			CMthreadJobTaskDependent (job, taskId, dLink);
		}
		for (timeStep = 0;timeStep < timeStepNum; ++timeStep) {
			NCRinputLoad   (runoff,  timeStep, network);
			for (timeHour = 0;timeHour < 24;timeHour += dt) CMthreadJobExecute (team, job);
			if (NCRoutputWrite (outflow, timeStep, network) != true) goto Stop;
		}
		CMthreadJobDestroy  (job,(CMthreadUserFreeFunc) NULL);
		CMthreadTeamDestroy (team);
	}
	NCRoutputCopyInput (runoff, outflow);
	NCRinputClose  (runoff);
Stop:
	NCRoutputClose (outflow);
	NCRnetworkFree (network);
	return (0);
}
