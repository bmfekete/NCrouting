#include<stdio.h>
#include<string.h>
#include<cm.h>
#include<ncr.h>

int main (int argc, char *argv []) {
	int argPos, argNum = argc;
	char *netName = (char *) NULL;
	size_t timeStepNum, timeStep, timeHour;
	NetworkCell_t *firstCell, *lastCell;
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
		if (CMargTest (argv [argPos],"-h","--help")) {
			CMmsgPrint (CMmsgInfo,"%s [options] <nc grid> <nc grid>\n", CMprgName(argv [0]));
			CMmsgPrint (CMmsgInfo,"     -n,--network\n");
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
	if ((firstCell = NetworkLoad (netName)) == (NetworkCell_t *) NULL) return (-1);
	lastCell = NetworkLastCell (firstCell);
	routingInitialize (firstCell,(float) dt);

	CMmsgPrint (CMmsgInfo, "Opening runoff\n");
	if ((runoff = inputOpen (firstCell, lastCell->Id, argv [1], "runoff")) == (void *) NULL) {
		NetworkCellFree (firstCell);
		return (-1);
	}
	if ((timeStepNum = inputTimeStepNum (runoff)) == 0) {
		NetworkCellFree (firstCell);
		inputClose (runoff);
		return (-1);
	}

	CMmsgPrint (CMmsgInfo, "Opening outflow\n");
	if ((outflow = outputOpen (firstCell, argv [2], "outflow")) == (void *) NULL) {
		NetworkCellFree (firstCell);
		return (-1);
	}
	
	CMmsgPrint (CMmsgInfo, "Start routing\n");
	for (timeStep = 0;timeStep < timeStepNum; ++timeStep) {
		inputLoad   (runoff,  timeStep, firstCell);
		for (timeHour = 0;timeHour < 24;timeHour += dt) Routing (lastCell,(float) dt);
		printf ("Timestep: %4d\n",(int) timeStep);
		if (outputWrite (outflow, timeStep, firstCell) != true) {
			goto Stop;
		}
	}
	outputCopyInput (runoff, outflow);
	inputClose  (runoff);
Stop:
	outputClose (outflow);
	return (0);
}
