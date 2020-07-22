/*
 * file:     mgetraw.c
 * project:  Mtools
 *
 * Purpose : upload raw config file from SCI
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "mtools.h"

/*{{{  help*/
#define HELP "\
-------------------- MTOOLS -------------------\n\
Written by >> Mountcomp Ltd << England\n\
Function: mgetraw V1.00,June, 1995 for %s\n\
\n\
Qualifiers as follows  ... \n\
\tA target node address in decimal\n\
\tB SCSI bus address in decimal\n\
\tC configuration file (required)\n\
\tN name of the target node (required)\n\
\tF target RS3 Filename as text (required)\n\
\tH this help screen\n\
\tO output file (destination) \n\
\tV verbose mode\n\
"
/*}}}*/

/*{{{  variables and structure for options*/
int   opt_address=0; /* defaults if they are not specified */
int   opt_bus=0;     /* actually, c defaults all vars to 0 anyway */
char *opt_config;
char *opt_filename;
char *opt_nodename;
char *opt_output;
int   opt_verbose;

option_struct options[] = {
	/* param     type      pointer    min,max  required-mask */
	{ "address", integer, &opt_address,1,992},
	{ "bus",     integer, &opt_bus,1,2},
	{ "config",  string,  &opt_config,0,999,   1},
	{ "filename",string,  &opt_filename,0,9,   2},
	{ "nodename",string,  &opt_nodename,0,10,  4},
	{ "output",  string,  &opt_output,0,999},
	{ "verbose", boolean, &opt_verbose},
	{ NULL }
};
/*}}}*/

/*{{{  main*/
int main(int argc, char *argv[])
{
	FILE *outfile;

	FILE *configfile;
	char cfg_line[20];
	char request[SCI_BUFSIZ];
	
	mtl_environment();
	mtl_commandline(argc, argv, options,7, HELP, 0);

	/*{{{  open output file, if given*/
	if (opt_output)
	{
		if (!(outfile=fopen(opt_output, "w")))
			FATAL(MTL__CANTOPENOUT);
	}
	else
		outfile = stdout;
	/*}}}*/

	/*{{{  open config file*/
	if (!(configfile=fopen(opt_config, "r")))
	{
		FATAL(MTL__CANTOPENIN);
	}
	
	
	/*}}}*/

	sprintf(request+2, "%02d%-10s%03d%1d%-9s",
	   SCI_CMD_GETRAW,
	   opt_nodename,
	   opt_address,
	   opt_bus,
	   opt_filename);
	   
	sci_open();

	while (fgets(cfg_line, 20, configfile))
		/*{{{  process config line*/
		{
			char *cfg;
			int node, controller;
			int n;
			
			if (sscanf(cfg_line, "%d %n", &node, &n) < 1)
				FATAL(MTL__BADCONFIG);
		
			cfg = cfg_line+n; /* point at first type */
		
			for (controller='A'; controller <= 'H'; ++controller, ++cfg)
			{
				switch (*cfg)
				{
					case '-':
						break;
						
					case 'M':
						read_mpc(node, controller, outfile, request, request+27);
						break;
						
					case 'P':
						read_plc(node, controller, outfile, request, request+27);
						break;
		
					default:
						WARN(MTL__BADCONTROL);
						break;
				}
			}
		}
		/*}}}*/
		
	sci_close();
	return SUCCESS;
}
/*}}}*/
