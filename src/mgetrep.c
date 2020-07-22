/*
 * file:     mgetrep.c
 * project:  Mtools
 *
 * Purpose : upload ascii report from SCI
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "mtools.h"

/*{{{  help message*/
#define HELP "\
-------------------- MTOOLS -------------------\n\
Written by >> Mountcomp Ltd << England\n\
Function: mgetrep V1.00,June, 1995 for %s\n\
\nHelp screen, Qualifiers as follows  ... \n\
\tA target node address in decimal\n\
\tB SCSI bus address in decimal\n\
\tN name of the target node (required)\n\
\tF target Filename as text (required)\n\
\tH this help screen\n\
\tO output file (destination) \n\
\tV verbose mode\n\
\tR report number (*)   Exactly one of R and D\n\
\tD report date   (*)   is required\n\
"
/*}}}*/

/*{{{  options variables and structure*/
int   opt_address;
int   opt_bus;
char *opt_filename=NULL;
char *opt_nodename=NULL;
char *opt_output=NULL;
int   opt_verbose=0;
char *opt_date="000000000000";
int   opt_report=0;

option_struct options[] = {
	{ "address", integer, &opt_address,0,992},
	{ "bus",     integer, &opt_bus,0,1},
	{ "filename",string,  &opt_filename,0,9,  1},
	{ "nodename",string,  &opt_nodename,0,10, 2},
	{ "report",  integer, &opt_report,0,999,  4},
	{ "date",    string,  &opt_date,0,12,     4},
	{ "output",  string,  &opt_output,0,999},
	{ "verbose", boolean, &opt_verbose},
	{ NULL }
};
/*}}}*/
 
int main(int argc, char *argv[])
{
	char *reply;
	int seq=1;
	FILE *outfile;

	mtl_environment();
	mtl_commandline(argc, argv, options, 7, HELP, 0);

	/*{{{  open outfile*/
	if (opt_output)
	{
		if (!(outfile=fopen(opt_output, "w")))
			FATAL(MTL__CANTOPENOUT);
	}
	else
		outfile = stdout;
	/*}}}*/
 
	sci_open();

	/* first request has extra info. After the %04d, it goes
	 *  3d - timeout - 0 means one at a time
	 *  1d - generation check - 0 means check at end
	 *  1d - line transmit - 0 means can send partial lines
	 *  1d - multi - 0 means can put multiple lines in message
	 *  1d - blank - 0 means ignore blank lines
	 *  1d - compression - 0 means can compress
	 *  1c - compress char
	 *  1d - compress code - 0 => code is no of spaces
	 *                       1 =>         next column
	 *  1d - repeat - 1 => repeat the compression char if it appears in text
	 */
	
	reply = sci_ftransact(1,"%02d0001%-10s%03d%1d%-9s%05d%-12s%04d00001011001",
	   SCI_CMD_REPORT, opt_nodename,opt_address,opt_bus, opt_filename,
	   opt_report, opt_date, SCI_MAXMSG - 20);

	for (;;) /* we break when we want to leave */
	{
		int count;
		if (sscanf(reply,"%4d%3d", &seq, &count) != 2)
			FATAL(MTL__GARBAGE);

		reply += 7;

		while (--count >= 0)
			/*{{{  write next line to outfile*/
			{
				int len;
				if (sscanf(reply += 5 /* skip row */, "%3d", &len) != 1)
					FATAL(MTL__GARBAGE);
			
				reply += 3; /* skip len */
				reply[len]=0; /* truncate string at len - dont care about row */
				fputs(reply, outfile);
				putc('\n', outfile);
				reply += len;
			}
			/*}}}*/

		if (seq==0)
			break;  /* we're done */

		/* ask for next chunk */
		reply = sci_ftransact(1,"%02d%04d", SCI_CMD_REPORT, seq);
	}

	sci_close();
	return SUCCESS;
}
