/*
 * file:     mgetraw.c
 * project:  Mtools
 *
 * Purpose : upload raw config from controller via SCI
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "mtools.h"

/*{{{  help*/
#define HELP "\
-------------------- MTOOLS -------------------\n\
Written by >> Mountcomp Ltd << England\n\
Function: mgetctrl V1.00,June, 1995 for %s\n\
\nHelp screen, Qualifiers as follows  ... \n\
\tL lower limit of range of nodes (default 1)\n\
\tU upper limit of range of nodes (default 32)\n\
\tH this help screen\n\
\tO output file (destination) \n\
\tV verbose mode\n\
"
/*}}}*/

/*{{{  variables and structure for options*/
char *opt_output;
int   opt_verbose;
int   opt_lower=1;
int   opt_upper=32;

option_struct options[] = {
	/* param     type      pointer    min,max  required-mask */
	{ "lower",   integer, &opt_lower, 1,992},
	{ "upper",   integer, &opt_upper, 1,992},	
	{ "output",  string,  &opt_output,0,999},
	{ "verbose", boolean, &opt_verbose},
	{ NULL }
};
/*}}}*/


/* find out what controller types are on a given node
 * return pointer to (local) int array, or NULL
 */
/*{{{  int *read_types(int node)*/
int *read_types(int node)
{
	static int types[8];
	
	/* here, we dont let sci_ftransact check error code for us
	 * since we anticipate error number 12 (no such node) or
	 * 18 (not a control file)
	 */
		
	int controller;
	int *t;
	char *reply = sci_ftransact(0,"%02d%04d", SCI_CMD_CFS, node);
		
	/*{{{  check status*/
	{
		int status;
	
		if (sscanf(reply, "%3d", &status) != 1)
			FATAL(MTL__GARBAGE);
	
		if (status==12 || status==18)
			return NULL; /* nothing here */
	
		if (status != 0)
			mtl_sci_error(status); /* any other error response is bad news */
	}
	/*}}}*/

	reply += 33; /* skip header stuff */
		
	/*{{{  loop over controllers*/
	for (controller=0, t=types; controller<8; ++controller, ++t)
	{
		if ( sscanf(reply,"%2d", t) != 1)
			FATAL(MTL__GARBAGE);
	
		if (*t>14)
			*t=0;
	
		reply += 27;
	}
	/*}}}*/

	return types;	
}
/*}}}*/
 
/*{{{  main*/
int main(int argc, char *argv[])
{
	int node;
	char request[SCI_BUFSIZ];
	FILE *outfile;

	mtl_environment();
	mtl_commandline(argc, argv, options, 3, HELP, 0);

	/*{{{  open output file, if given*/
	if (opt_output)
	{
		if (!(outfile=fopen(opt_output, "w")))
			FATAL(MTL__CANTOPENOUT);
	}
	else
		outfile = stdout;
	/*}}}*/

	sci_open();

	/* leave room for mid */
	sprintf(request+2, "%02d", SCI_CMD_GETCTRL);

	for (node=opt_lower; node <= opt_upper; ++node)
	{
		int controller;
		int *type;

		if (!(type=read_types(node)))
			continue; /* nothing here */

		for (controller='A'; controller <= 'H'; ++controller, ++type)
		{
			switch (*type)
			{
				case 0:
					continue; /* no controller in slot */
				case 5:
					read_plc(node, controller, outfile, request, request+4);
					break;

				case 8:
				case 14:
					read_mpc(node, controller, outfile, request, request+4);
					break;

				default:
					WARN(MTL__BADCONTROL);
					continue;
			}
		}
	}

	sci_close();
	return SUCCESS;
}
/*}}}*/
