/*
 * file:     mgetasc.c
 * project:  Mtools
 *
 * Purpose : upload ascii file from SCI
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "mtools.h"

/*{{{  help message*/
#define HELP "\
-------------------- MTOOLS -------------------\n\
Written by >> Mountcomp Ltd << England\n\
Function: mgetasc V1.00,June, 1995 for %s\n\
\nNo parameters. Qualifiers as follows  ... \n\
\tA target node address in decimal\n\
\tB SCSI bus address in decimal\n\
\tN name of the target node (required)\n\
\tF target Filename as text (required)\n\
\tH this help screen\n\
\tO output file (destination) \n\
\tV verbose mode\n\
\tT file type (required). type is a number 0 .. 13 as follows ... \n\
\t   File in ASCII Files Folder\n\
\t    0 = Script in ASCII format,  1 = Script file parse errors\n\
\t    2 = ASCII Batch Ops. Table,  3 = Batch Ops. Table parse errors\n\
\t    4 = ASCII Batch Unit Table,  5 = Batch Unit Table parse errors\n\
\t    6 = ASCII Batch Mat. Table,  7 = Batch Mat. Table parse errors\n\
\t    8 = ASCII Master Recipe,     9 = Master Recipe parse errors\n\
\t   10 = ASCII Control Recipe,   11 = Control Recipe parse errors\n\
\t   12 = ASCII Working Recipe,   13 = Working Recipe parse errors\n\
"
/*}}}*/

/*{{{  options variables and structure*/
int   opt_address=0; /* defaults if they are not specified */
int   opt_bus=0;     /* actually, c defaults all vars to 0 anyway */
int   opt_filetype;
char *opt_filename;
char *opt_nodename;
char *opt_output;
int   opt_verbose;

option_struct options[] = {
	/* param     type      pointer    min,max  required-mask */
	{ "address", integer, &opt_address,1,992},
	{ "bus",     integer, &opt_bus,1,2},
	{ "type",    integer, &opt_filetype,0,22,  1},
	{ "filename",string,  &opt_filename,0,9,   2},
	{ "nodename",string,  &opt_nodename,0,10,  4},
	{ "output",  string,  &opt_output,0,999},
	{ "verbose", boolean, &opt_verbose},
	{ NULL }
};
/*}}}*/
 
int main(int argc, char *argv[])
{
	unsigned int offset=0, next_offset;

	/* datestamp too big to fit in 32 bits, so use strings */
	static char datestamp[13] = "";

	FILE *outfile;

	long written=0; /* to check that no data gets omitted */

	mtl_environment();
	mtl_commandline(argc, argv, options,7, HELP, 0);

	/*{{{  open output file, if given*/
	if (opt_output)
	{
		/* if empty string was specified, use same as /filename */
		
		if (!*opt_output)
			opt_output=opt_filename;
			
		if (!(outfile=fopen(opt_output, "w")))
			FATAL(MTL__CANTOPENOUT);
	}
	else
		outfile = stdout;
	/*}}}*/
 
	sci_open();

	do
	{
		/*{{{  one transaction*/
		
		char *reply = sci_ftransact(1,"%02d%-10s%03d%1d22%02d%-9s0%08X000",
		   SCI_CMD_GETASC,
			opt_nodename,
			opt_address,
			opt_bus,
			opt_filetype,
			opt_filename,
			offset);
		
		int size;
		
		if (*datestamp && (strncmp(reply, datestamp,12)) != 0)
			WARN(MTL__FILECHANGED);
		
		strncpy(datestamp, reply, 12);
		/* not sure if strncpy terminates dest string, but if not,
		 * dont worry, since there is always a 0 at [12]
		 */
		
		if (sscanf(reply+=12, "%8x%2x", &next_offset, &size) < 2)
			FATAL(MTL__GARBAGE);
		
		reply += 10;
		
		if (size==0)
			size=256;  /* so it says in the protocol description */
			
		while (size)
			/*{{{  process one packet*/
			{
				int offset;
				int len;
			
				if (size <= 6)
				{
					/* there's no room in this packet for any data - something is up */
					FATAL(MTL__TOOSHORT);
					break;
				}
			
				if (sscanf(reply, "%8x%4x", &offset, &len) < 2)
					FATAL(MTL__GARBAGE);
			
				if (offset != written)
					FATAL(MTL__BADOFFSET);
			
				reply += 12;
			
				mtl_hex2bin(reply, reply,len); /* inline decode of hex data */
			
				fputs(reply, outfile);
				written += len;
			
				reply += 2*len;
				size -= (len+6); /* 6 bytes header (4 for offset, 2 for len) */
			}
			/*}}}*/
		/*}}}*/
	}	
	while ( (offset=next_offset) != 0 );
	
	sci_close();
	return SUCCESS;
}
