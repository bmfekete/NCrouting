#include <ncr.h>
#include <cm.h>
static NetworkCell_t *network_cell_create ()
{
	NetworkCell_t *cell;

	if ((cell = (NetworkCell_t *) malloc (sizeof (NetworkCell_t))) == (NetworkCell_t *) NULL)
	{ perror ("Memory allocation error in: network_cell_create ()"); return ((NetworkCell_t *) NULL); }

	cell->Id = -1;
	cell->NextCell = cell->PrevCell = cell->ToCell = (NetworkCell_t *) NULL;
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

void NetworkCellFree (NetworkCell_t *cur_cell)
{
	NetworkCell_t *next_cell;

	while (cur_cell != (NetworkCell_t *) NULL)
	{
		next_cell = cur_cell->NextCell;
		free (cur_cell);
		cur_cell = next_cell;
	}
}

NetworkCell_t *NetworkLastCell (NetworkCell_t *cur_cell)
{
	if (cur_cell != (NetworkCell_t *) NULL)
		while (cur_cell->NextCell != (NetworkCell_t *) NULL) cur_cell = cur_cell->NextCell;
	return (cur_cell);
}

NetworkCell_t *NetworkLoad (const char *filename)
{
	FILE *fp;
	int buffer_len = 0, cell_id, to_cell_id;
	char *buffer = (char *) NULL;
	NetworkCell_t *first_cell = (NetworkCell_t *) NULL;
	NetworkCell_t *cur_cell   = (NetworkCell_t *) NULL;
	NetworkCell_t *next_cell, *to_cell;

	if ((fp = fopen (filename,"r")) == (FILE *) NULL)
	{ perror ("File opening error in: network_load ()"); return ((NetworkCell_t *) NULL); }

	if ((buffer = CMbufGetLine (buffer, &buffer_len, fp)) == (char *) NULL)
	{ perror ("File reading error in: network_load ()"); return ((NetworkCell_t *) NULL); }

	while ((buffer = CMbufGetLine (buffer, &buffer_len, fp)) != (char *) NULL)
	{
		if ((cur_cell = network_cell_create ()) == (NetworkCell_t *) NULL)
		{
			NetworkCellFree (first_cell);
			return ((NetworkCell_t *) NULL);
		}
		
		if (first_cell == (NetworkCell_t *) NULL) first_cell = next_cell = cur_cell;
		else { cur_cell->PrevCell = next_cell; next_cell->NextCell = cur_cell; next_cell = cur_cell; }

		if (sscanf (buffer,"%d %f %f %d %d %f %f %f %f\n",&cell_id, &cur_cell->Lon, &cur_cell->Lat, &cur_cell->BasinId,
		            &to_cell_id, &cur_cell->CellArea, &cur_cell->CellLength, &cur_cell->Slope, &cur_cell->MeanDischarge) != 9)
		{
			perror ("File reading error in: network_load ()");
			NetworkCellFree (first_cell);
			return ((NetworkCell_t *) NULL);
		}
		cur_cell->Id = cell_id;
		if (to_cell_id > -1)
		{
			to_cell = cur_cell;
			while ((to_cell != (NetworkCell_t *) NULL) && (to_cell->Id != to_cell_id)) to_cell = to_cell->PrevCell;
			if (to_cell == (NetworkCell_t *) NULL)
			{
				fprintf (stderr,"Corrupt network in: network_load ()\n");
				NetworkCellFree (first_cell);
				return ((NetworkCell_t *) NULL);
			}
			else cur_cell->ToCell = to_cell;
		}
	}
	fclose (fp);
	return (first_cell);
}
