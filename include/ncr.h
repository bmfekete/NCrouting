#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

char *get_line (char *, int *, FILE *);

typedef struct NCRnetworkCell_s {
	size_t Id;
	int   BasinId;
	float Lon;
	float Lat;
	float CellArea;
	float CellLength;

	struct NCRnetworkCell_s *NextCell;
	struct NCRnetworkCell_s *PrevCell;
	struct NCRnetworkCell_s *ToCell;
	float Slope;
	float MeanDischarge;
	float Runoff;
	float Inflow [2];
	float Outflow;
	float Storage;
	float c [3];
} NCRnetworkCell_t;

NCRnetworkCell_t *NCRnetworkLoad (const char *);
void NCRnetworkCellFree (NCRnetworkCell_t *);
NCRnetworkCell_t *NCRnetworkLastCell (NCRnetworkCell_t *);

int    NCIgetDimension    (int, const char *);
float *NCgetFloatVector (int, const char *, size_t *);

float  NCRsphericalDistance (float, float, float, float);
void  *NCRinputOpen   (NCRnetworkCell_t *,size_t, const char *, const char *);
void   NCRinputClose  (void *);
bool   NCRinputLoad   (void *, size_t, NCRnetworkCell_t *);
float *NCRinputTimeArray (void *);
bool   NCRinputCopyAttributes (void *,const char *, int, int);
size_t NCRinputTimeStepNum (void *);
void  *NCRoutputOpen  (NCRnetworkCell_t *, const char *, const char *);
void   NCRoutputClose (void *);
bool   NCRoutputWrite (void *, size_t, NCRnetworkCell_t *);
bool   NCRoutputCopyInput (void *, void *);

void   NCRroutingInitialize (NCRnetworkCell_t *, float);
bool   NCRrouting (NCRnetworkCell_t *, float dt);
