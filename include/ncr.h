#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

char *get_line (char *, int *, FILE *);

typedef struct NCRnetworkCell_s {
	size_t Id;
	int   ToCellId;
	int   BasinId;
	float Lon;
	float Lat;
	float CellArea;
	float CellLength;

	float Slope;
	float MeanDischarge;
	float Runoff;
	float Inflow [2];
	float Outflow;
	float Storage;
	float c [3];
} NCRnetworkCell_t;

typedef struct NCRnetwork_s {
	size_t CellNum;
	NCRnetworkCell_t **Cells;
} NCRnetwork_t;

NCRnetwork_t *NCRnetworkLoad (const char *);
void NCRnetworkFree (NCRnetwork_t *);

int    NCIgetDimension    (int, const char *);
float *NCgetFloatVector (int, const char *, size_t *);

float  NCRsphericalDistance (float, float, float, float);
void  *NCRinputOpen   (NCRnetwork_t *, const char *, const char *);
void   NCRinputClose  (void *);
bool   NCRinputLoad   (void *, size_t, NCRnetwork_t *);
float *NCRinputTimeArray (void *);
bool   NCRinputCopyAttributes (void *,const char *, int, int);
size_t NCRinputTimeStepNum (void *);
void  *NCRoutputOpen  (NCRnetwork_t *, const char *, const char *);
void   NCRoutputClose (void *);
bool   NCRoutputWrite (void *, size_t, NCRnetwork_t *);
bool   NCRoutputCopyInput (void *, void *);

void   NCRroutingInitialize (NCRnetwork_t *, float);
bool   NCRrouting (NCRnetwork_t *, float dt);
void   NCRroutingFunc (void *, void *,void *, size_t);
