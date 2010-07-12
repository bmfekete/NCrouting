#include <math.h>
#include <ncr.h>
#include <cm.h>

#define B_COEFF  2.0       /* parabola exponent */
#define C_COEFF  22.2222   /* 1/n Manning */
#define D_COEFF  (2.0/3.0) /* Manning exponent */
#define E_COEFF  0.5       /* Slope exponent */
#define Rm_COEFF 0.0229    /* depth/width ratio */

void NCRroutingInitialize (NCRnetwork_t *network, float dt) {
	size_t cellId;
	float cross_area, width, celerity;
	float courant;
	float reynolds;
	NCRnetworkCell_t *cell;
 
//	printf ("\"CellID\"\t\"Slope\"\t\"CrossArea\"\t\"Width\"\t\"Celerity\"\t\"Courant\"\t\"Reynolds\"\t\"C0\"\t\"C1\"\t\"C2\"\t\"SumC\"\n");
	for (cellId = 0;cellId < network->CellNum; cellId++) {
		cell = network->Cells [cellId];
		if ((cell->MeanDischarge > 0.0) && (cell->CellLength > 0.0)) {
			cross_area = C_COEFF * pow (B_COEFF / (1.0 + B_COEFF),D_COEFF / 2.0) * pow (Rm_COEFF, D_COEFF / 2.0);
			cross_area = pow (cell->MeanDischarge / (cross_area * pow (cell->Slope / 1000.0, E_COEFF)), 1.0 / (1.0 + D_COEFF / 2.0));
			width      = sqrt (cross_area * (B_COEFF + 1.0) / (B_COEFF * Rm_COEFF));
			celerity   = (cell->MeanDischarge / cross_area) * 13.0 / 9.0;

			courant    = celerity *  dt * 3600.0 / (cell->CellLength * 1000.0);
			reynolds   = cell->MeanDischarge / (cell->Slope * celerity * width * cell->CellLength);

			cell->c [0] = (-1.0 + courant + reynolds) / (1.0 + courant + reynolds);
			cell->c [1] = ( 1.0 + courant - reynolds) / (1.0 + courant + reynolds);
			cell->c [2] = ( 1.0 - courant + reynolds) / (1.0 + courant + reynolds);
//			printf ("%d\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\n",
//			        cur_cell->id, cur_cell->slope, cross_area, width, celerity, courant, reynolds,
//					  cur_cell->c [0],  cur_cell->c [1], cur_cell->c [2],  cur_cell->c [0] +  cur_cell->c [1] + cur_cell->c [2]);
		}
		else {
			cell->c [0] = 1.0;
			cell->c [1] = 0.0;
			cell->c [2] = 0.0;
		}
	}
}

void NCRroutingFunc (void *commonPtr,void *threadData, size_t cellId) {
	NCRnetwork_t *network = (NCRnetwork_t *) commonPtr;
	NCRnetworkCell_t *cell;

	cellId = network->CellNum - cellId - 1;
	cell = network->Cells [cellId];
	cell->Inflow [1] += cell->Runoff;
	cell->Outflow = cell->c [0] * cell->Inflow [1] + cell->c [1] * cell->Inflow [0] + cell->c [2] * cell->Outflow;
	if (cell->ToCellId > 0) {
		network->Cells [cell->ToCellId - 1]->Inflow [1] += cell->Outflow;
	}
	cell->Inflow [0] = cell->Inflow [1];
	cell->Inflow [1] = 0;
}

bool NCRrouting (NCRnetwork_t *network) {
	int cellId;
	NCRnetworkCell_t *cell;

	for (cellId = network->CellNum - 1;cellId >= 0; cellId--) {
		cell = network->Cells [cellId];
		cell->Inflow [1] += cell->Runoff;
		cell->Outflow = cell->c [0] * cell->Inflow [1] + cell->c [1] * cell->Inflow [0] + cell->c [2] * cell->Outflow;
		if (cell->ToCellId > 0) network->Cells [cell->ToCellId - 1]->Inflow [1] += cell->Outflow;
		cell->Inflow [0] = cell->Inflow [1];
		cell->Inflow [1] = 0;
	}
	return (true);
}
