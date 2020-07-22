/*
 * file:     mputasc.c
 * project:  Mtools
 *
 * Purpose : download ascii file to RS3 system via SCI
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "mtools.h"

/*{{{  help message*/
#define HELP "\
-------------------- MTOOLS -------------------\n\
Written by >> Mountcomp Ltd << England\n\
Function: mputasc V1.00,June, 1995 for %s\n\
\n\
Required parameter - host filename to send\n\
Qualifiers as follows  ... \n\
\tA target node address in decimal\n\
\tB SCSI bus address in decimal\n\
\tN name of the target node (required)\n\
\tF target RS3 Filename as text (required)\n\
\tH this help screen\n\
\tV verbose mode\n\
\tT file type (required). type is a number 0 .. 13 as follows ... \n\
\t   File in ASCII Files Folder\n\
\t    0 = Script in ASCII format,\n\
\t    2 = ASCII Batch Ops. Table,\n\
\t    4 = ASCII Batch Unit Table,\n\
\t    6 = ASCII Batch Mat. Table,\n\
\t    8 = ASCII Master Recipe,\n\
\t   10 = ASCII Control Recipe,\n\
\t   12 = ASCII Working Recipe,\n\
"
/*}}}*/

/*{{{  option variables and structure*/
int   opt_address; /* defaults if they are not specified */
int   opt_bus;     /* actually, c defaults all vars to 0 anyway */
int   opt_filetype;
char *opt_filename;
char *opt_nodename;
int   opt_verbose;

option_struct options[] = {
	{ "address", integer, &opt_address, 1, 992},
	{ "bus",     integer, &opt_bus,     1, 2},
	{ "type",    integer, &opt_filetype,0,13,   1 },
	{ "filename",string,  &opt_filename,0,9,    2 },
	{ "nodename",string,  &opt_nodename,0,10,   4 },
	{ "verbose", boolean, &opt_verbose},
	{ NULL }
};
/*}}}*/
 
int main(int argc, char *argv[])
{
	FILE *fp;
	char *param;
	unsigned int offset=0;
	static char datestamp[13];
	char chunk[SCI_BUFSIZ];

	/* to save the overhead of copying strings back and forth,
	 * we convert binary to hex into the transaction buffer.
	 * We keep one request string, and dont modify the part
	 * of the request that is fixed
	 */
	char request[SCI_BUFSIZ];

	/* max chars we can send in one message */
	
	mtl_environment();
	param = mtl_commandline(argc, argv, options,7, HELP, 999);
	
	if (!param || !(fp=fopen(param,"r")))
		FATAL(MTL__CANTOPENIN);

	sprintf(request+2,"%02d%-10s%03d%1d22%02d%-9s0",
	   SCI_CMD_PUTASC,
	   opt_nodename,
	   opt_address,
	   opt_bus,
	   opt_filetype,
	   opt_filename);

	sci_open();

	while (fgets(chunk, 255, fp))
	{
		/*{{{  fill and send one chunk*/
		char *reply;
		
		int size=strlen(chunk);
		
		/*{{{  try to fill the packet*/
		while (size < 250 && fgets(chunk+size, 255-size, fp))
		{
			size += strlen(chunk+size);
		}
		/*}}}*/
		
		/*{{{  make the request string*/
		sprintf(request+32, "%08X%02X", offset, size);
		
		/* convert binary data to hex string
		 * according to manual, all packets must end if 0A or 0D,
		 * but that should not be counted in size. So just append
		 * that here
		 */
		
		chunk[size]=0x0d; /* chunk does not have to be a null-terminated string */
		
		mtl_bin2hex(request+42,chunk,size+1);
		
		/* bin2hex does terminate the output string */
		/*}}}*/
		
		/*{{{  send the packet and verify the reply*/
		
		reply=sci_transact(1,request);
		
		if (*datestamp && (strncmp(datestamp,reply,12) != 0))
			FATAL(MTL__FILECHANGED);
		
		strncpy(datestamp, reply+12, 12);
		
		offset += size;
		/*}}}*/
		/*}}}*/
	}
	
	sci_close();
	return SUCCESS;
}
