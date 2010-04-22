#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

char *get_line (char *, int *, FILE *);

typedef struct GFDLnetworkCell_s
{
	size_t Id;
	int   BasinId;
	float Lon;
	float Lat;
	float CellArea;
	float CellLength;

	struct GFDLnetworkCell_s *NextCell;
	struct GFDLnetworkCell_s *PrevCell;
	struct GFDLnetworkCell_s *ToCell;
	float Slope;
	float MeanDischarge;
	float Runoff;
	float Inflow [2];
	float Outflow;
	float Storage;
	float c [3];
} NetworkCell_t;

NetworkCell_t *NetworkLoad (const char *);
void NetworkCellFree (NetworkCell_t *);
NetworkCell_t *NetworkLastCell (NetworkCell_t *);

int    NCIgetDimension    (int, const char *);
float *NCgetFloatVector (int, const char *, size_t *);

float  sphericalDistance (float, float, float, float);
void  *inputOpen   (NetworkCell_t *,size_t, const char *, const char *);
void   inputClose  (void *);
bool   inputLoad   (void *, size_t, NetworkCell_t *);
float *inputTimeArray (void *);
bool   inputCopyAttributes (void *,const char *, int, int);
size_t inputTimeStepNum (void *);
void  *outputOpen  (NetworkCell_t *, const char *, const char *);
void   outputClose (void *);
bool   outputWrite (void *, size_t, NetworkCell_t *);
bool   outputCopyInput (void *, void *);

void   routingInitialize (NetworkCell_t *, float);
bool   Routing (NetworkCell_t *, float dt);
