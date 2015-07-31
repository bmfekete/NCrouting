#include <ncr.h>
#include <cm.h>

static NCRnetwork_t *_NCRnetworkCreate() {
    NCRnetwork_t *network;

    if ((network = (NCRnetwork_t *) malloc(sizeof(NCRnetwork_t))) == (NCRnetwork_t *) NULL) {
        CMmsgPrint(CMmsgAppError, "Memory allocation error in: %s:%d\n", __FILE__, __LINE__);
        return ((NCRnetwork_t *) NULL);
    }
    network->CellNum = 0;
    network->Cells = (NCRnetworkCell_t **) NULL;
    return (network);
}

static NCRnetworkCell_t *_NCRnetworkCellAdd(NCRnetwork_t *network) {

    if ((network->Cells = realloc(network->Cells, (network->CellNum + 1) * sizeof(NCRnetworkCell_t *))) ==
        (NCRnetworkCell_t **) NULL) {
        CMmsgPrint(CMmsgAppError, "Memory allocation error in: %s:%d\n", __FILE__, __LINE__);
        return ((NCRnetworkCell_t *) NULL);
    }

    if ((network->Cells[network->CellNum] = (NCRnetworkCell_t *) malloc(sizeof(NCRnetworkCell_t))) ==
        (NCRnetworkCell_t *) NULL) {
        CMmsgPrint(CMmsgAppError, "Memory allocation error in: %s:%d\n", __FILE__, __LINE__);
        return ((NCRnetworkCell_t *) NULL);
    }

    network->Cells[network->CellNum]->Id = -1;
    network->Cells[network->CellNum]->Runoff = 0.0;
    network->Cells[network->CellNum]->Inflow[0] = 0.0;
    network->Cells[network->CellNum]->Inflow[1] = 0.0;
    network->Cells[network->CellNum]->Outflow = 0.0;
    network->Cells[network->CellNum]->Storage = 0.0;
    network->Cells[network->CellNum]->c[0] = 1.0;
    network->Cells[network->CellNum]->c[1] = 0.0;
    network->Cells[network->CellNum]->c[2] = 0.0;
    network->CellNum += 1;
    return (network->Cells[network->CellNum - 1]);
}

void NCRnetworkFree(NCRnetwork_t *network) {
    size_t cellId;;

    for (cellId = 0; cellId < network->CellNum; ++cellId) free(network->Cells[cellId]);
    free(network->Cells);
    free(network);
}

NCRnetwork_t *NCRnetworkLoad(const char *filename) {
    FILE *fp;
    int buffer_len = 0, cellId, toCellId;
    char *buffer = (char *) NULL;
    NCRnetwork_t *network;
    NCRnetworkCell_t *cell;

    if ((fp = fopen(filename, "r")) == (FILE *) NULL) {
        CMmsgPrint(CMmsgAppError, "File opening error in: %s:%d\n", __FILE__, __LINE__);
        return ((NCRnetwork_t *) NULL);
    }

    if ((buffer = CMbufGetLine(buffer, &buffer_len, fp)) == (char *) NULL) {
        CMmsgPrint(CMmsgAppError, "File reading error in: %s:%d\n", __FILE__, __LINE__);
        fclose(fp);
        return ((NCRnetwork_t *) NULL);
    }
    if ((network = _NCRnetworkCreate()) == (NCRnetwork_t *) NULL) {
        CMmsgPrint(CMmsgAppError, "Network creation error in: %s:%d\n", __FILE__, __LINE__);
        free(buffer);
        fclose(fp);
        return ((NCRnetwork_t *) NULL);
    }
    while ((buffer = CMbufGetLine(buffer, &buffer_len, fp)) != (char *) NULL) {
        if ((cell = _NCRnetworkCellAdd(network)) == (NCRnetworkCell_t *) NULL) {
            NCRnetworkFree(network);
            fclose(fp);
            return ((NCRnetwork_t *) NULL);
        }

        if (sscanf(buffer, "%d %f %f %d %d %f %f %f %f\n", &cellId, &cell->Lon, &cell->Lat, &cell->BasinId,
                   &toCellId, &cell->CellArea, &cell->CellLength, &cell->Slope, &cell->MeanDischarge) != 9) {
            CMmsgPrint(CMmsgAppError, "File reading error in: %s:%d\n", __FILE__, __LINE__);
            NCRnetworkFree(network);
            fclose(fp);
            return ((NCRnetwork_t *) NULL);
        }
        cell->Id = cellId;
        cell->ToCellId = toCellId;
    }
    fclose(fp);
    return (network);
}
