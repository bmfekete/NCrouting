#include <ncr.h>
#include <netcdf.h>
#include <math.h>
#include <cm.h>

typedef struct input_s {
    int NCid;
    int VarId;
    size_t TimeStepNum;
    size_t RowNum, ColNum;
    size_t *RowArray, *ColArray;

    float *array;
} input_t;

static input_t *inputCreate(size_t cell_num, size_t rowNum, size_t colNum) {
    input_t *input;
    if ((input = malloc(sizeof(input_t))) == (input_t *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory allocation error in: %s:%d\n", __FILE__, __LINE__);
        return ((input_t *) NULL);
    }

    if ((input->RowArray = (size_t *) calloc(cell_num, sizeof(size_t))) == (size_t *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory allocation error in: %s:%d\n", __FILE__, __LINE__);
        free(input);
        return ((input_t *) NULL);
    }
    if ((input->ColArray = (size_t *) calloc(cell_num, sizeof(size_t))) == (size_t *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory allocation error in: %s:%d\n", __FILE__, __LINE__);
        free(input->RowArray);
        free(input);
        return ((input_t *) NULL);
    }
    if ((input->array = (float *) calloc(rowNum * colNum, sizeof(float))) == (float *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory allocation error in: %s:%d\n", __FILE__, __LINE__);
        free(input->RowArray);
        free(input->ColArray);
        free(input);
        return ((input_t *) NULL);
    }
    input->RowNum = rowNum;
    input->ColNum = colNum;
    input->NCid = -1;
    input->TimeStepNum = 0;
    return (input);
}

void NCRinputClose(void *ptr) {
    input_t *input = (input_t *) ptr;

    if (input->NCid != -1) nc_close(input->NCid);
    free(input->RowArray);
    free(input->ColArray);
    free(input->array);
    free(input);
}

void *NCRinputOpen(NCRnetwork_t *network, const char *source, const char *varName) {
    int status, ncid, ndims, varid;
    size_t timeStepNum, colNum, rowNum, col, row, nearestCol, nearestRow, cellId;
    input_t *input;
    float *lons, *lats;
    float distance, minLon, minLat;
    NCRnetworkCell_t *cell;

    if ((status = nc_open(source, NC_NOWRITE, &ncid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return ((void *) NULL);
    }
    if ((status = nc_inq_varid(ncid, varName, &varid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return ((void *) NULL);
    }
    if ((status = nc_inq_varndims(ncid, varid, &ndims)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status), __FILE__, __LINE__);
        nc_close(ncid);
        return ((float *) NULL);
    }
    if (ndims != 3) {
        CMmsgPrint(CMmsgAppError, "Invalid number of dimensions in: %s:%d\n", __FILE__, __LINE__);
        nc_close(ncid);
        return ((float *) NULL);
    }
    if ((status = NCIgetDimension(ncid, "time")) == -1) {
        nc_close(ncid);
        return ((void *) NULL);
    }
    timeStepNum = (size_t) status;

    if ((lons = NCgetFloatVector(ncid, "lon", &colNum)) == (float *) NULL) {
        return ((void *) NULL);
    }
    if ((lats = NCgetFloatVector(ncid, "lat", &rowNum)) == (float *) NULL) {
        free(lons);
        return ((void *) NULL);
    }

    for (col = 0; col < colNum; ++col)
        if (lons[col] > 180.0) lons[col] = lons[col] - 360.0;
    if ((input = (input_t *) inputCreate(network->CellNum, rowNum, colNum)) == (input_t *) NULL) {
        nc_close(ncid);
        free(lons);
        free(lats);
        return ((void *) NULL);
    }
    input->NCid = ncid;

    for (cellId = 0; cellId < network->CellNum; cellId++) {
        minLon = minLat = HUGE_VAL;
        cell = network->Cells[cellId];
        for (row = 0; row < rowNum; ++row) {
            distance = fabs(cell->Lat - lats[row]);
            if (minLat > distance) {
                minLat = distance;
                nearestRow = row;
            }
        }
        for (col = 0; col < colNum; ++col) {
            distance = fabs(cell->Lon - lons[col]);
            if (minLon > distance) {
                minLon = distance;
                nearestCol = col;
            }
        }
        input->ColArray[cell->Id] = nearestCol;
        input->RowArray[cell->Id] = nearestRow;
    }

    input->TimeStepNum = timeStepNum;
    input->VarId = varid;

    free(lons);
    free(lats);
    return ((void *) input);
}

size_t NCRinputTimeStepNum(void *ptr) {
    input_t *input = (input_t *) ptr;

    if (input == (input_t *) NULL) return (0);
    return (input->TimeStepNum);
}

bool NCRinputLoad(void *ptr, size_t timeStep, NCRnetwork_t *network) {
    size_t cellId;
    int status;
    size_t start[3], count[3];
    input_t *input = (input_t *) ptr;
    NCRnetworkCell_t *cell;

    if (input == (input_t *) NULL) return (false);
    if (timeStep >= input->TimeStepNum) return (false);

    start[0] = timeStep;
    count[0] = 1;
    start[1] = (size_t) 0;
    count[1] = input->RowNum;
    start[2] = (size_t) 0;
    count[2] = input->ColNum;

    if ((status = nc_get_vara_float(input->NCid, input->VarId, start, count, input->array)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status), __FILE__, __LINE__);
        return (false);
    }

    for (cellId = 0; cellId < network->CellNum; cellId++) {
        cell = network->Cells[cellId];
        cell->Runoff = input->array[input->ColNum * input->RowArray[cellId] + input->ColArray[cellId]];
        if (cell->Runoff < 0.0) cell->Runoff = 0.0;
        else cell->Runoff = cell->Runoff * cell->CellArea * 1000.0;
    }
    return (true);
}

float *NCRinputTimeArray(void *ptr) {
    int status, timeId, timeStepNum;
    input_t *input = (input_t *) ptr;
    float *array;

    if ((status = nc_inq_varid(input->NCid, "time", &timeId)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status), __FILE__, __LINE__);
        return ((void *) NULL);
    }
    if ((status = NCIgetDimension(input->NCid, "time")) == -1)
        return ((float *) NULL);
    timeStepNum = (size_t) status;

    if ((array = calloc(timeStepNum, sizeof(float))) == (float *) NULL) {
        CMmsgPrint(CMmsgSysError, "Memory allocation error in %s:%d\n", __FILE__, __LINE__);
        return ((float *) NULL);
    }
    if ((status = nc_get_var_float(input->NCid, timeId, array)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NCError: %s in %s:%d\n", nc_strerror(status), __FILE__, __LINE__);
        free(array);
        return ((float *) NULL);
    }

    return (array);
}

static bool _CopyAttributes(in_ncid, in_varid, out_ncid, out_varid) {
    int status, att = 0;
    char att_name[NC_MAX_NAME];

    while ((status = nc_inq_attname(in_ncid, in_varid, att++, att_name)) == NC_NOERR) {
        if ((status = nc_copy_att(in_ncid, in_varid, att_name, out_ncid, out_varid)) != NC_NOERR) {
            CMmsgPrint(CMmsgAppError, "NetCDF attribute copying error \"%s\" in %s:%d!\n", nc_strerror(status),
                       __FILE__, __LINE__);
            return (false);
        }
    }
    return (true);
}

bool NCRinputCopyAttributes(void *ptr, const char *varName, int outNCid, int outVarId) {
    int status, varid;
    input_t *input = (input_t *) ptr;

    if ((status = nc_inq_varid(input->NCid, varName, &varid)) != NC_NOERR) {
        CMmsgPrint(CMmsgAppError, "NCError \"%s\" in: %s:%d\n", nc_strerror(status), __FILE__, __LINE__);
        return (false);
    }

    return (_CopyAttributes(input->NCid, varid, outNCid, outVarId));
}
