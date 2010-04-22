#include<stdio.h>
#include<stdlib.h>
#include<netcdf.h>

int NCIgetDimension (int ncid, const char *dim_name)
{
	int status, dimid;
	size_t len;

	if ((status = nc_inq_dimid (ncid, dim_name, &dimid)) != NC_NOERR)
	{ fprintf (stderr, "NCError \"%s\" in:  nci_dimension ()\n",nc_strerror(status)); return (-1); }

	if ((status = nc_inq_dimlen (ncid, dimid, &len)) != NC_NOERR)
	{ fprintf (stderr, "NCError \"%s\" in:  nci_dimension ()\n",nc_strerror(status)); return (-1); }
	
	return ((int) len);
}

float *NCgetFloatVector (int ncid, const char *var_name, size_t *ret)
{
	int status, varid, dimid, ndims;
	size_t len;
	float *vector;

	if ((status = nc_inq_varid (ncid, var_name, &varid)) != NC_NOERR)
	{ fprintf (stderr, "NCError \"%s\" in:  nc_get_float_vector ()\n",nc_strerror(status)); return ((float *) NULL); }

	if ((status = nc_inq_varndims (ncid, varid, &ndims)) != NC_NOERR)
	{ fprintf (stderr, "NCError \"%s\" in:  nc_get_float_vector ()\n",nc_strerror(status)); return ((float *) NULL); }

	if (ndims > 1)
	{ fprintf (stderr, "Variable [%s] has too many dimensions [%d] in: nc_get_float_vector ()\n",var_name, ndims); return ((float *) NULL); }

	if ((status = nc_inq_vardimid (ncid, varid, &dimid)) != NC_NOERR)
	{ fprintf (stderr, "NCError \"%s\" in:  nc_get_float_vector ()\n",nc_strerror(status)); return ((float *) NULL); }

	if ((status = nc_inq_dimlen (ncid, dimid, &len)) != NC_NOERR)
	{ fprintf (stderr, "NCError \"%s\" in:  nci_get_float_vector ()\n",nc_strerror(status)); return ((float *) NULL); }

	if ((vector = (float *) calloc (len, sizeof (float))) == (float *) NULL)
	{ perror ("Memory allocation error in: nci_get_float_vector ()"); return ((void *) NULL); }

	if ((status = nc_get_var_float (ncid, varid, vector)) != NC_NOERR)
	{ fprintf (stderr, "NCError \"%s\" in:  nc_get_float_vector ()\n",nc_strerror(status)); free (vector); return ((void *) NULL); }

	*ret = len;
	return (vector);
}

