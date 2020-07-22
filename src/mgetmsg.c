/*
 * file:     mgetmsg.c
 * project:  Mtools
 *
 * Purpose : get message-pairs from RS3 system
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mtools.h"

/*{{{  help message*/
#define HELP "\
-------------------- MTOOLS -------------------\n\
Written by >> Mountcomp Ltd << England\n\
Function: mgetmsg V1.00,June, 1995 for %s\n\
\nHelp screen, Qualifiers as follows  ... \n\
\tL lower limit of range of messages\n\
\tU upper limit of range of messages\n\
\tH this help screen\n\
\tO output file (destination) \n\
\tV verbose mode\n\
"
/*}}}*/

/*{{{  option variables and structure*/
char *opt_output=NULL;
int   opt_verbose=0;
int   opt_lower=1;
int   opt_upper=255;

option_struct options[] = {
	{ "lower",  integer, &opt_lower,1,255},
	{ "upper",  integer, &opt_upper,1,255},
	{ "verbose",boolean, &opt_verbose},
	{ "output", string,  &opt_output,0,99},
	{ NULL }
};
/*}}}*/
 
int main(int argc, char *argv[])
{
	int min;
	int chunk = (SCI_MAXMSG - 50)/17;  /* how many can we fit into one message */
	FILE *outfile;
	
	mtl_environment();
	mtl_commandline(argc, argv, options, 0, HELP, 0);
	
	/*{{{  open output file if necessary*/
	if (opt_output)
	{
		if (!(outfile=fopen(opt_output, "w")))
			FATAL(MTL__CANTOPENOUT);
	}
	else
		outfile = stdout;
	/*}}}*/

	min = opt_lower;

	sci_open();

	/* send more than one request if necessary */

	while (min <= opt_upper)
	{
		char *reply;
		int max;
		
		if ( (max=min+chunk) > opt_upper)
			max = opt_upper;

		reply = sci_ftransact(0,"%02d%03d%03d", SCI_CMD_MSGPAIR, min, max);

		while ( (min <= max) && *reply)
		{
			char save = reply[8]; /* preserve first character of OFF field */

			if (reply[16] != ',')
				FATAL(MTL__GARBAGE);

			reply[8] = reply[16] = 0; /* terminate the two strings */
			
			fprintf(outfile, "%c%-3d: %c%-9s%-10s\n",
			  min<=100 ? '*' : ' ', min<=100 ? min : min-100,
			  save, reply+9, reply);
			++min;
			reply += 17;
		}

		/* min is now max+1 */
	}
	
	sci_close();
	
	return SUCCESS;
}
