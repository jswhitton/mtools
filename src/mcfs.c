/*
 * file:     mcfs.c
 * project:  Mtools
 *
 * Purpose : fetch and display ControlFile status from RS3 system.
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
Function: mcfs V1.00,June, 1995 for %s\n\
\nHelp screen,\n\
\n\
 No parameters. Qualifiers as follows  ... \n\
\tA target node address in decimal (required)\n\
\tH this help screen\n\
\tO output file (destination) \n\
\tV verbose mode\n\
\n\
"
/*}}}*/

/*{{{  DO() macro - output one line of table*/
#define DO(title, format, expr) { \
 int i; \
 fprintf(outfile,"|%-12s|", title);\
 for (i=0; i<8; ++i)                \
  if (i<slots && slot[i].type)       \
   fprintf(outfile,format, expr);     \
  else                       \
   fputs("       |", outfile);\
 putc('\n',outfile);           \
}
/*}}}*/

/*{{{  option variables and structure*/
/* help is done as a special case */

int   opt_verbose;
char *opt_output;
int   opt_address;

option_struct options[] = {
	{ "verbose", boolean, &opt_verbose },
	{ "output",  string,  &opt_output,0,99 },
	{ "address", integer, &opt_address,0,9999, 1 },
	{ NULL }
};
/*}}}*/

int main(int argc, char *argv[])
{
	/*{{{  variables, lookup-tables, etc*/
	char *reply;
	FILE *outfile;
	
	typedef char string[10];
	
	/*{{{  tables of proctype, modes, alarms*/
	static char proctype[][8] = {
	   " None  ",
		"  MLC  ",
		"   CC  ",
		"  SSC  ",
		"  MUX  ",
		"  PLC  ",
		" SMART ",
		"  RBLC ",
		"  MPC  ",
		" ATMLC ",
		" MPTUN ",
		" MPCAS ",
		" MPCAP ",
		" MPCAT ",
		"  MPC2 "
	};
	
	static char *modes[] = { "norm", "stby" };
	static char *alarms[] = { "no", "yes"};
	static char *act[]={ "Left ","Right","none"};
	/*}}}*/
	
	/*{{{  variables for the header stuff*/
	string idle;
	int available;
	int bubble_active;
	string bubble_free;
	string leftCP, rightCP, CPrev;
	int CPactive;
	string control_rev;
	/*}}}*/
	
	/*{{{  structure for info from each slot*/
	/* not sure how floats are formatted, so store and print as strings */
	struct slot_struct
	{
		int type;
		string bootrev, idle, free;
		int links,redundancy,mode,alarm;
		string scan,timeout;
	} slot[8];
	/*}}}*/
	
	int slots=0;
	/*}}}*/

	mtl_environment();
	mtl_commandline(argc, argv, options,1, HELP, 0);

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

	reply = sci_ftransact(1,"%02d%04d", SCI_CMD_CFS, opt_address);

	/*{{{  parse header stuff*/
	if (sscanf(reply, "%3s%2d%1d%3s%5s%5s%5s%1d%5s",
	   idle, &available,
	   &bubble_active, bubble_free,
	   leftCP, rightCP, CPrev,
	   &CPactive, control_rev)     != 9)
	{
		FATAL(MTL__GARBAGE);
	}
	/*}}}*/
	
	/*{{{  parse slot information into structure*/
	{	struct slot_struct *s = slot;
	
		for (reply += 30, slots=0; *reply; reply += 27,++slots, ++s)
		{
			if (sscanf(reply, "%2d%5s%3s%3s%2d%1d%1d%1d%4s%5s",
			   &s->type, s->bootrev,s->idle,s->free,
				&s->links,&s->redundancy,
			   &s->mode,&s->alarm,s->scan,s->timeout) != 10)
			   FATAL(MTL__GARBAGE);
	
			 if (s->type > 14) s->type = 0;
		}
	}
	/*}}}*/

	/*{{{  display header stuff*/
	fprintf(outfile, "\n\n%50s\n\n", "CONTROL FILE STATUS");
	
	fprintf(outfile, "Node Address >%2d\n", opt_address);
	
	fprintf(outfile, "Left  CP:");
	if (CPactive==0)
		fprintf(outfile, "   Boot %s  Prgm %s   Avail Links %d   Idle time %s %%", leftCP, CPrev, available, idle);
	
	fprintf(outfile, "\nRight CP:");
	if (CPactive==1)
		fprintf(outfile, "   Boot %s  Prgm %s   Avail Links %d   Idle time %s", rightCP, CPrev, available, idle);
	
	fprintf(outfile, "\n%s Program NVM Free %s %%\n", act[bubble_active], bubble_free);
	/*}}}*/
 
	/*{{{  output slot info*/
	fputs("\n+--------------  A  ---  B  ---  C  ---  D  ---  E  ---  F  ---  G  ---  H  -+\n", outfile);
	
	/* DO iterates over slots, displaying 3rd param with format of 2nd param */
	
	DO("Control Type", "%s|", proctype[slot[i].type])
	DO("Boot Rev",     " %5s |", slot[i].bootrev)
	DO("Prgm Rev",     " %5s |", control_rev)
	DO("Idle Time",    "%5s%% |", slot[i].idle)
	DO("Free Space",   "%5s%% |", slot[i].free)
	DO("Avail Links",  "%5d  |", slot[i].links)
	DO("Redundancy",   "   %c   |", "-ABCDEFGH"[slot[i].redundancy])
	DO("Mode",         "%5s  |", modes[slot[i].mode&1])
	DO("Alarm Inhib",  "%5s  |", alarms[slot[i].alarm&1])
	DO("Scan Time",  "%5sS |", slot[i].scan)
	DO("SC Timeout", " %5s |", slot[i].timeout)
	
	fputs(  "+----------------------------------------------------------------------------+\n\n", outfile);
	
	/*}}}*/
	
	sci_close();
	
	return SUCCESS;
}
