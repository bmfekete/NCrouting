#include <ncr.h>
#include <cm.h>
static NCRnetworkCell_t *network_cell_create () {
	NCRnetworkCell_t *cell;

	if ((cell = (NCRnetworkCell_t *) malloc (sizeof (NCRnetworkCell_t))) == (NCRnetworkCell_t *) NULL) {
		perror ("Memory allocation error in: network_cell_create ()");
		return ((NCRnetworkCell_t *) NULL);
	}

	cell->Id = -1;
	cell->NextCell = cell->PrevCell = cell->ToCell = (NCRnetworkCell_t *) NULL;
	cell->Runoff     = 0.0;
	cell->Inflow [0] = 0.0;
	cell->Inflow [1] = 0.0;
	cell->Outflow    = 0.0;
	cell->Storage    = 0.0;
	cell->c [0] = 1.0;
	cell->c [1] = 0.0;
	cell->c [2] = 0.0;
	return (cell);
}

void NCRnetworkCellFree (NCRnetworkCell_t *cur_cell) {
	NCRnetworkCell_t *next_cell;

	while (cur_cell != (NCRnetworkCell_t *) NULL) {
		next_cell = cur_cell->NextCell;
		free (cur_cell);
		cur_cell = next_cell;
	}
}

NCRnetworkCell_t *NCRnetworkLastCell (NCRnetworkCell_t *cur_cell) {
	if (cur_cell != (NCRnetworkCell_t *) NULL)
		while (cur_cell->NextCell != (NCRnetworkCell_t *) NULL) cur_cell = cur_cell->NextCell;
	return (cur_cell);
}

NCRnetworkCell_t *NCRnetworkLoad (const char *filename) {
	FILE *fp;
	int buffer_len = 0, cell_id, to_cell_id;
	char *buffer = (char *) NULL;
	NCRnetworkCell_t *first_cell = (NCRnetworkCell_t *) NULL;
	NCRnetworkCell_t *cur_cell   = (NCRnetworkCell_t *) NULL;
	NCRnetworkCell_t *next_cell, *to_cell;

	if ((fp = fopen (filename,"r")) == (FILE *) NULL) {
		perror ("File opening error in: network_load ()");
		return ((NCRnetworkCell_t *) NULL);
	}

	if ((buffer = CMbufGetLine (buffer, &buffer_len, fp)) == (char *) NULL) {
		perror ("File reading error in: network_load ()");
		return ((NCRnetworkCell_t *) NULL);
	}

	while ((buffer = CMbufGetLine (buffer, &buffer_len, fp)) != (char *) NULL) {
		if ((cur_cell = network_cell_create ()) == (NCRnetworkCell_t *) NULL) {
			NCRnetworkCellFree (first_cell);
			return ((NCRnetworkCell_t *) NULL);
		}
		
		if (first_cell == (NCRnetworkCell_t *) NULL) first_cell = next_cell = cur_cell;
		else { cur_cell->PrevCell = next_cell; next_cell->NextCell = cur_cell; next_cell = cur_cell; }

		if (sscanf (buffer,"%d %f %f %d %d %f %f %f %f\n",&cell_id, &cur_cell->Lon, &cur_cell->Lat, &cur_cell->BasinId,
		            &to_cell_id, &cur_cell->CellArea, &cur_cell->CellLength, &cur_cell->Slope, &cur_cell->MeanDischarge) != 9) {
			perror ("File reading error in: network_load ()");
			NCRnetworkCellFree (first_cell);
			return ((NCRnetworkCell_t *) NULL);
		}
		cur_cell->Id = cell_id;
		if (to_cell_id > -1) {
			to_cell = cur_cell;
			while ((to_cell != (NCRnetworkCell_t *) NULL) && (to_cell->Id != to_cell_id)) to_cell = to_cell->PrevCell;
			if (to_cell == (NCRnetworkCell_t *) NULL) {
				fprintf (stderr,"Corrupt network in: network_load ()\n");
				NCRnetworkCellFree (first_cell);
				return ((NCRnetworkCell_t *) NULL);
			}
			else cur_cell->ToCell = to_cell;
		}
	}
	fclose (fp);
	return (first_cell);
}
