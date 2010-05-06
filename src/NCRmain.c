#include <stdio.h>
#include <string.h>
#include <cm.h>
#include <ncr.h>

int main (int argc, char *argv []) {
	int argPos, argNum = argc, nThreads = 1;
	char *netName = (char *) NULL;
	size_t timeStepNum, timeStep, timeHour;
	NCRnetworkCell_t *firstCell, *lastCell;
	void *runoff, *outflow;
	int dt = 6;

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
	if ((firstCell = NCRnetworkLoad (netName)) == (NCRnetworkCell_t *) NULL) return (-1);
	lastCell = NCRnetworkLastCell (firstCell);
	NCRroutingInitialize (firstCell,(float) dt);

	CMmsgPrint (CMmsgInfo, "Opening runoff\n");
	if ((runoff = NCRinputOpen (firstCell, lastCell->Id, argv [1], "runoff")) == (void *) NULL) {
		NCRnetworkCellFree (firstCell);
		return (-1);
	}
	if ((timeStepNum = NCRinputTimeStepNum (runoff)) == 0) {
		NCRnetworkCellFree (firstCell);
		NCRinputClose (runoff);
		return (-1);
	}

	CMmsgPrint (CMmsgInfo, "Opening outflow\n");
	if ((outflow = NCRoutputOpen (firstCell, argv [2], "outflow")) == (void *) NULL) {
		NCRnetworkCellFree (firstCell);
		return (-1);
	}
	
	CMmsgPrint (CMmsgInfo, "Start routing\n");
	for (timeStep = 0;timeStep < timeStepNum; ++timeStep) {
		NCRinputLoad   (runoff,  timeStep, firstCell);
		for (timeHour = 0;timeHour < 24;timeHour += dt) NCRrouting (lastCell,(float) dt);
// TODO:	printf ("Timestep: %4d\n",(int) timeStep);
		if (NCRoutputWrite (outflow, timeStep, firstCell) != true) {
			goto Stop;
		}
	}
	NCRoutputCopyInput (runoff, outflow);
	NCRinputClose  (runoff);
Stop:
	 NCRoutputClose (outflow);
	return (0);
}
