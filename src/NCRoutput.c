#include <ncr.h>
#include <netcdf.h>
#include <math.h>
#include <cm.h>

typedef struct NCRoutput_s {
	int     NCid;
	int     VarId;
	int     TimeId;
	int     LonId;
	int     LatId;
	size_t  RowNum, ColNum;

	float dX,    dY;
	float Xmin, Ymin;
	float *Array;
} NCRoutput_t;

void NCRoutputClose (void *ptr) {
	NCRoutput_t *output = (NCRoutput_t *) ptr;

	if (output->NCid != -1) nc_close (output->NCid);
	free (output->Array);
	free (output);
}

void *NCRoutputOpen (NCRnetwork_t *network, const char *target, const char *varName) {
	int status, ncid, dimids [3], timeId, lonId, latId, varId;
	size_t colNum, rowNum, col, row, cellId;
	float dx, dy, xMin, yMin, xMax, yMax;
	float *array, missingVal = -9999.0;
	NCRnetworkCell_t *cell, *toCell;
	NCRoutput_t *output;

	dx = dy =
	xMin = yMin =  HUGE_VAL;
	xMax = yMax = -HUGE_VAL;
	for (cellId = 0;cellId < network->CellNum; cellId++) {
		cell = network->Cells [cellId];
		if (xMin > cell->Lon) xMin = cell->Lon;
		if (yMin > cell->Lat) yMin = cell->Lat;
		if (xMax < cell->Lon) xMax = cell->Lon;
		if (yMax < cell->Lat) yMax = cell->Lat;
		if (cell->ToCellId > 0) {
			toCell = network->Cells [cell->ToCellId - 1];
	   	if ((fabs (cell->Lon - toCell->Lon) > 0.0) && (fabs (cell->Lon - toCell->Lon) < dx)) dx = fabs (cell->Lon - toCell->Lon);
		if ((fabs (cell->Lat - toCell->Lat) > 0.0) && (fabs (cell->Lat - toCell->Lat) < dy)) dy = fabs (cell->Lat - toCell->Lat);
		}
	}
	colNum = (int) ((xMax - xMin) / dx) + 1;
	rowNum = (int) ((yMax - yMin) / dy) + 1;

	if ((array = (float *) calloc (colNum * rowNum, sizeof (float))) == (float *) NULL) {
		CMmsgPrint (CMmsgSysError,"Memory allocation error in: %s:%d\n",__FILE__,__LINE__);
		return ((void *) NULL);
	}

	if ((status = nc_create (target, NC_CLOBBER, &ncid))   != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		free (array);
		return ((void *)  NULL);
	}
	if ((status = nc_def_dim (ncid,"time", NC_UNLIMITED,  dimids + 0)) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		nc_close (ncid);
		free (array);
		return ((void *) NULL);
	}
	if ((status = nc_def_dim (ncid,"longitude",  colNum, dimids + 2)) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		nc_close (ncid);
		free (array);
		return ((void *) NULL);
	}
	if ((status = nc_def_dim (ncid,"latitude",   rowNum, dimids + 1)) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		nc_close (ncid);
		free (array);
		return ((void *) NULL);
	}
	if ((status = nc_def_var (ncid, "time",      NC_FLOAT, 1, dimids + 0, &timeId)) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		nc_close (ncid);
		free (array);
		return ((void *) NULL);
	}
	if ((status = nc_def_var (ncid, "longitude", NC_FLOAT, 1, dimids + 2, &lonId)) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		nc_close (ncid);
		free (array);
		return ((void *) NULL);
	}
	if ((status = nc_def_var (ncid, "latitude",  NC_FLOAT, 1, dimids + 1, &latId)) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		nc_close (ncid);
		free (array);
		return ((void *) NULL);
	}
	if ((status = nc_def_var (ncid, varName,    NC_FLOAT, 3, dimids + 0, &varId))  != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		nc_close (ncid);
		free (array);
		return ((void *) NULL);
	}
	if ((status = nc_put_att_float (ncid, varId, "_FillValue",  NC_FLOAT, 1, &missingVal)) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		nc_close (ncid);
		free (array);
		return ((void *) NULL);
	}
	if ((status = nc_put_att_float (ncid, varId, "missing_value",  NC_FLOAT, 1, &missingVal)) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		nc_close (ncid);
		free (array);
		return ((void *) NULL);
	}
	if ((status = nc_put_att_text (ncid, varId, "units", strlen ("m3/s"), "m3/s")) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		nc_close (ncid);
		free (array);
		return ((void *) NULL);
	}

	if ((status = nc_enddef (ncid)) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		nc_close (ncid);
		free (array);
		return ((void *) NULL);
	}

	for (col = 0;col < colNum; ++col) array [col] = xMin + dx * col;
	if ((status = nc_put_var_float (ncid, lonId, array)) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		nc_close (ncid);
		free (array);
		return ((void *) NULL);
	}
	for (row = 0;row < rowNum; ++row) array [row] = yMin + dy * row;
	if ((status = nc_put_var_float (ncid, latId, array)) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		nc_close (ncid);
		free (array);
		return ((void *) NULL);
	}

	if ((output = malloc (sizeof (NCRoutput_t))) == (NCRoutput_t *) NULL) {
		CMmsgPrint (CMmsgSysError,"Memory allocation error in: %s:%d\n",__FILE__,__LINE__);
		nc_close (ncid);
		free (array);
		return ((void *) NULL);
	}

	output->NCid   = ncid;
	output->TimeId = timeId;
	output->LonId  = lonId;
	output->LatId  = latId;
	output->VarId  = varId;
	output->ColNum = colNum;
	output->RowNum = rowNum;
	output->Array  = array;
	output->Xmin   = xMin;
	output->Ymin   = yMin;
	output->dX     = dx;
	output->dY     = dy;

	return (output);
}

bool NCRoutputWrite (void *ptr, size_t time_step, NCRnetwork_t *network) {
	int status;
	size_t start [3], count [3], col, row, cellId;
	NCRoutput_t *output = (NCRoutput_t *) ptr;
	NCRnetworkCell_t *cell;

	if (output == (NCRoutput_t *) NULL) return (false);
	start [0] = time_step;  count [0] = 1;
	start [1] = (size_t) 0; count [1] = output->RowNum;
	start [2] = (size_t) 0; count [2] = output->ColNum;
	for (row = 0;row < output->RowNum; ++row)
		for (col = 0;col < output->ColNum; ++col) output->Array [output->ColNum * row + col] = -9999.0;

	for (cellId = 0; cellId < network->CellNum; cellId++) {
		cell = network->Cells [cellId];
		col = (int) ((cell->Lon - output->Xmin) / output->dX);
		row = (int) ((cell->Lat - output->Ymin) / output->dY);
		output->Array [output->ColNum * row + col] =  cell->Outflow;
	}
	if ((status = nc_put_vara_float (output->NCid, output->VarId, start, count, output->Array)) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n",nc_strerror(status),__FILE__,__LINE__); return (false);
	}
	return (true);
}

bool NCRoutputCopyInput (void *inputPtr, void *outputPtr) {
	float *array;
	int status;
	NCRoutput_t *output = (NCRoutput_t *) outputPtr;

	if ((array = NCRinputTimeArray (inputPtr)) == (float *) NULL) return (false);
	if ((status = nc_put_var_float (output->NCid, output->TimeId, array)) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n",nc_strerror(status),__FILE__,__LINE__);
		return (false);
	}
	if ((status = nc_redef (output->NCid)) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		return (false);
	}
	if (NCRinputCopyAttributes (inputPtr,"time", output->NCid,output->TimeId) == false) return (false);
	if (NCRinputCopyAttributes (inputPtr,"lon",  output->NCid,output->LonId)  == false) return (false);
	if (NCRinputCopyAttributes (inputPtr,"lat",  output->NCid,output->LatId)  == false) return (false);
	if ((status = nc_enddef (output->NCid)) != NC_NOERR) {
		CMmsgPrint (CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status),__FILE__,__LINE__);
		return (false);
	}
	return (true);
}
