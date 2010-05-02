#include<math.h>
#include<ncr.h>

#define B_COEFF  2.0       /* parabola exponent */
#define C_COEFF  22.2222   /* 1/n Manning */
#define D_COEFF  (2.0/3.0) /* Manning exponent */
#define E_COEFF  0.5       /* Slope exponent */
#define Rm_COEFF 0.0229    /* depth/width ratio */

void NCRroutingInitialize (NCRnetworkCell_t *first_cell, float dt) {
	float cross_area, width, celerity;
	float courant;
	float reynolds;

	NCRnetworkCell_t *cur_cell;
 
//	printf ("\"CellID\"\t\"Slope\"\t\"CrossArea\"\t\"Width\"\t\"Celerity\"\t\"Courant\"\t\"Reynolds\"\t\"C0\"\t\"C1\"\t\"C2\"\t\"SumC\"\n");
	for (cur_cell = first_cell;cur_cell != (NCRnetworkCell_t *) NULL; cur_cell = cur_cell->NextCell) {
		if ((cur_cell->MeanDischarge > 0.0) && (cur_cell->CellLength > 0.0)) {
			cross_area = C_COEFF * pow (B_COEFF / (1.0 + B_COEFF),D_COEFF / 2.0) * pow (Rm_COEFF, D_COEFF / 2.0);
			cross_area = pow (cur_cell->MeanDischarge / (cross_area * pow (cur_cell->Slope / 1000.0, E_COEFF)), 1.0 / (1.0 + D_COEFF / 2.0));
			width      = sqrt (cross_area * (B_COEFF + 1.0) / (B_COEFF * Rm_COEFF));
			celerity   = (cur_cell->MeanDischarge / cross_area) * 13.0 / 9.0;

			courant    = celerity *  dt * 3600.0 / (cur_cell->CellLength * 1000.0);
			reynolds   = cur_cell->MeanDischarge / (cur_cell->Slope * celerity * width * cur_cell->CellLength); 

			cur_cell->c [0] = (-1.0 + courant + reynolds) / (1.0 + courant + reynolds);
			cur_cell->c [1] = ( 1.0 + courant - reynolds) / (1.0 + courant + reynolds);
			cur_cell->c [2] = ( 1.0 - courant + reynolds) / (1.0 + courant + reynolds);
//			printf ("%d\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\n",
//			        cur_cell->id, cur_cell->slope, cross_area, width, celerity, courant, reynolds,
//					  cur_cell->c [0],  cur_cell->c [1], cur_cell->c [2],  cur_cell->c [0] +  cur_cell->c [1] + cur_cell->c [2]);
		}
		else {
			cur_cell->c [0] = 1.0;
			cur_cell->c [1] = 0.0;
			cur_cell->c [2] = 0.0;
		}
	}
}

bool NCRrouting (NCRnetworkCell_t *last_cell, float dt) {
	NCRnetworkCell_t *cur_cell;

	for (cur_cell = last_cell;cur_cell != (NCRnetworkCell_t *) NULL; cur_cell = cur_cell->PrevCell) {
		cur_cell->Inflow [1] += cur_cell->Runoff;
		cur_cell->Outflow = cur_cell->c [0] * cur_cell->Inflow [1] + cur_cell->c [1] * cur_cell->Inflow [0] + cur_cell->c [2] * cur_cell->Outflow;
		if (cur_cell->ToCell != (NCRnetworkCell_t *) NULL) cur_cell->ToCell->Inflow [1] += cur_cell->Outflow;
		cur_cell->Inflow [0] = cur_cell->Inflow [1];
		cur_cell->Inflow [1] = 0;
	}
	return (true);
}
