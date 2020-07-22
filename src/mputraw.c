/*
 * file:     mputraw.c
 * project:  Mtools
 *
 * Purpose : download raw file to SCI
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "mtools.h"

/*{{{  help message*/
#define HELP "\
-------------------- MTOOLS -------------------\n\
Written by >> Mountcomp Ltd << England\n\
Function: mputraw V1.00,June, 1995 for %s\n\
\n\
Required parameter - host filename to send\n\
Qualifiers as follows  ... \n\
\tA target node address in decimal\n\
\tB SCSI bus address in decimal\n\
\tN name of the target node (required)\n\
\tF target RS3 Filename as text (required)\n\
\tH this help screen\n\
\tV verbose mode\n\
"
/*}}}*/

/*{{{  option variables and structure*/
int   opt_address=0; /* defaults if they are not specified */
int   opt_bus=0;     /* actually, c defaults all vars to 0 anyway */
char *opt_filename;
char *opt_nodename;
int   opt_verbose;

option_struct options[] = {
	/* param     type      pointer    min,max  required-mask */
	{ "address", integer, &opt_address,1,992},
	{ "bus",     integer, &opt_bus,1,2},
	{ "filename",string,  &opt_filename,0,9,   1},
	{ "nodename",string,  &opt_nodename,0,10,  2},
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
	param = mtl_commandline(argc, argv, options,3, HELP, 999);

	if (!param || !(fp=fopen(param,"r")))
		FATAL(MTL__CANTOPENIN);

	sprintf(request+2, "%02d%-10s%03d%1d%-9s",
		SCI_CMD_PUTRAW,
		opt_nodename,
		opt_address,
		opt_bus,
		opt_filename);
			
	sci_open();

	write_raw(fp, request, request+27);

	sci_close();

	return SUCCESS;
}
