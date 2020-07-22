/*
 * file:     mconfig.c
 * project:  Mtools
 *
 * Purpose : create a configuration file, identifying controller
 *           id's on nodes, for use by mgetraw.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mtools.h"

/*{{{  help message*/
#define HELP "\
\n\
-------------------- MTOOLS -------------------\n\
Written by >> Mountcomp Ltd << England\n\
Function: mconfig V1.00,June, 1995 for %s\n\
\nHelp screen,\n\
\n\
 Qualifiers as follows  ... \n\
\tL lower node (default 1)\n\
\tU upper node (default 32)\n\
\tH this help screen\n\
\tV verbose mode\n\
\n\
"
/*}}}*/

/*{{{  option variables and structure*/
int   opt_verbose;
char *opt_output;
int   opt_lower=1;
int   opt_upper=32;

option_struct options[] = {
	{ "verbose", boolean, &opt_verbose },
	{ "output",  string,  &opt_output,0,99 },
	{ "lower",   integer, &opt_lower, 1,992},
	{ "upper",   integer, &opt_upper, 1,992},
	{ NULL }
};
/*}}}*/

int main(int argc, char *argv[])
{
	FILE *outfile;
	int node;
	
	mtl_environment();
	mtl_commandline(argc, argv, options,0, HELP,0 );

	/*{{{  open output file*/
	if (opt_output)
	{
		if (!(outfile=fopen(opt_output, "w")))
			FATAL(MTL__CANTOPENOUT);
	}
	else
		outfile = stdout;
	/*}}}*/

	sci_open();

	/*{{{  loop over nodes*/
	for (node=opt_lower; node<=opt_upper; ++node)
	{
		/* here, we dont let sci_ftransact check error code for us
		 * since we anticipate error number 12 (no such node) or
		 * 18 (not a control file)
		 */
	
		int controller;
		char *reply = sci_ftransact(0,"%02d%04d", SCI_CMD_CFS, node);
	
		/*{{{  check status*/
		{
			int status;
		
			if (sscanf(reply, "%3d", &status) != 1)
				FATAL(MTL__GARBAGE);
		
			if (status==12 || status==18)
				continue; /* skip this node */
		
			if (status != 0)
				mtl_sci_error(status); /* any other error response is bad news */
		}
		/*}}}*/
	
		fprintf(outfile, "%4d ", node);
	
		reply += 33; /* skip header stuff */
	
		/*{{{  loop over controllers*/
		for (controller=0; controller<8; ++controller)
		{
			int type;
			if ( sscanf(reply,"%2d", &type) != 1)
				FATAL(MTL__GARBAGE);
		
			if (type>14)
				type=0;
		
			/* A constant string is a pointer, and therefore can be
			 * used with [] - this just picks out the type-th character
			 * from the string (ie 0 => '-', 5 => 'P', etc)
			 */
			putc( "-****P**M*****M"[type], outfile);
		
			reply += 27;
		}
		/*}}}*/
	
		putc('\n', outfile);
	
	}
	/*}}}*/
			
	sci_close();
	
	return SUCCESS;
}
