#include<math.h>
#include<ncr.h>

#define RADIUS 6371.2213

float sphericalDistance (float lon0, float lat0, float lon1, float lat1) {
	double cosC, sinC;
	double distance;

	lon0 = lon0 * M_PI / 180.0; lat0 = lat0 * M_PI / 180.0;
	lon1 = lon1 * M_PI / 180.0; lat1 = lat1 * M_PI / 180.0;

	cosC = sin (lat0) * sin (lat1) + cos (lat0) * cos (lat1) * cos (lon1 - lon0);

	if (cosC > 0.9999) {
		sinC = sqrt (cos (lat1) * cos (lat1) * sin (lon1 - lon0) * sin (lon1 - lon0) +
		            (cos (lat0) * sin (lat1) - sin (lat0) * cos (lat1) * cos (lon1 - lon0)) *
		            (cos (lat0) * sin (lat1) - sin (lat0) * cos (lat1) * cos (lon1 - lon0)));
		distance = RADIUS * atan (sinC/cosC);
	}
	else distance = RADIUS * acos (cosC);

	return (distance);
}
