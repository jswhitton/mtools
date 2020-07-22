/*
 * file:     mputraw.c
 * project:  Mtools
 *
 * Purpose : download raw file to controller via SCI
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "mtools.h"

/*{{{  help message*/
#define HELP "\
-------------------- MTOOLS -------------------\n\
Written by >> Mountcomp Ltd << England\n\
Function: mputctrl V1.00,June, 1995 for %s\n\
\n\
Required parameter - host filename to send\n\
Qualifiers as follows  ... \n\
\tH this help screen\n\
\tV verbose mode\n\
"
/*}}}*/

/*{{{  option variables and structure*/
int   opt_verbose;

option_struct options[] = {
	/* param     type      pointer    min,max  required-mask */
	{ "verbose", boolean, &opt_verbose},
	{ NULL }
};
/*}}}*/

int main(int argc, char *argv[])
{
	char *param;
	FILE *fp;
	char request[SCI_BUFSIZ];
	
	mtl_environment();
	param = mtl_commandline(argc, argv, options,0, HELP, 999);

	if (!param || !(fp=fopen(param,"r")))
		FATAL(MTL__CANTOPENIN);

	sprintf(request+2, "%02d", SCI_CMD_PUTRAW);
			
	sci_open();

	write_raw(fp, request, request+4);

	sci_close();

	return SUCCESS;
}
