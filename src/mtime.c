/*
 * file:     mtime.c
 * project:  Mtools
 *
 * Purpose : set time on RS3 system.
 *
 * parameters: time - defaults to current (host) system time
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
Function: mtime V1.00,June, 1995 for %s\n\
\nHelp screen,\n\
\n\
Optional parameter <time> in form YYMMDDHHMMSS\n\
- default is current host system time.\n\
\n\
 Qualifiers as follows  ... \n\
\tH this help screen\n\
\tV verbose mode\n\
\n\
"
/*}}}*/

int opt_verbose;

option_struct options[] = {
	{ "verbose", boolean, &opt_verbose },
	{ NULL }
};

int main(int argc, char *argv[])
{
	char *param;

	/* pick up SCI config from environment */
	mtl_environment();

	/* parse command-line switches as directed by options
	 * get optional parameter as return value
	 */
	param = mtl_commandline(argc, argv, options, 0, HELP, 12);

	sci_open();

	if (param)
		/*{{{  use given time*/
		sci_ftransact(1, "%02d%s", SCI_CMD_TIME, param);	
		/*}}}*/
	else
		/*{{{  no parameter - generate string for current time/date*/
		{
			time_t now;
			struct tm *p;
			
			time(&now);
			p=localtime(&now);
		
			sci_ftransact(1,"%02d%02d%02d%02d%02d%02d%02d",
				SCI_CMD_TIME,
				p->tm_year % 100, p->tm_mon+1, p->tm_mday,
				p->tm_hour, p->tm_min, p->tm_sec);
		}
		/*}}}*/

	/* sci_transact() will report any errors */

	sci_close();
	
	return SUCCESS;
}
