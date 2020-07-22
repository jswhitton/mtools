/*
 * file:     mtools.c
 * project:  Mtools
 *
 * Purpose : utilities, option parsing, fetch environmental variables,
 *           define global opt_* variables.
 *           Hide VMS / generic differences behind a common function
 *           interface.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mtools.h"

/*{{{  global variables opt_*, for command-line options / environment vars*/
/* default settings for SCI communications parameters are in mtools.h
 * Can be overridden in environment [ mtl_environment() ]
 */

int opt_crc     = SCI_CRC;
char *opt_stfr  = SCI_STFR;
char *opt_endfr = SCI_ENDFR;

#ifdef DEBUG
char *opt_debug = NULL;
#endif
/*}}}*/

/*{{{  mtl_strdup*/
/* some libraries come with strdup, some dont. It's easy enough to
 * provide it
 */
 
char *mtl_strdup(char *orig)
{
	char *p;

	/* return immediately if there is no string to duplicate */
	if (!orig) return NULL;

	if ( !(p=malloc(strlen(orig)+1)) ) FATAL(MTL__OUTOFSTORE);
	return strcpy(p, orig);
}
/*}}}*/

/*{{{  mtl_environment*/
/* pick up options from the environment */
void mtl_environment()
{
	char *p;

	/* notice that these are assignments in the if */

	if ( (p=getenv("MTL_STFR")) ) opt_stfr = p;
	if ( (p=getenv("MTL_ENDFR")) ) opt_endfr = p;

	if ( (p=getenv("MTL_CHKSM")) )
	{
		/* if it doesn't begin y or Y, its 'no' */
		opt_crc = (*p=='y' || *p=='Y');
	}

#ifdef DEBUG
	/* value does not matter - NULL or not NULL is enough */
	opt_debug = getenv("MTL_DEBUG");
#endif

}
/*}}}*/

/*{{{  static int mtl_hexchar*/
static int mtl_hexchar(int c)
{
	if (c >= '0' && c <= '9')
		return c-'0';

	if (c >= 'a' && c <= 'z')
		return c-('a'-10);

	if (c >= 'A' && c <= 'Z')
		return c-('A'-10);

	FATAL(MTL__BADHEX);
}
/*}}}*/

/*{{{  mtl_hex2bin*/
/* 'to' can be same as 'from' on entry, if desired
 * transfer until count reaches 0 or end of from[]
 */
void mtl_hex2bin(unsigned char *to, char *from, int count)
{
	while (--count >= 0 && *from)
	{
		if (!from[1])
			FATAL(MTL__NOTEVEN);

		*to++ = 16*mtl_hexchar(from[0]) + mtl_hexchar(from[1]);
		from += 2;
	}

	*to=0; /* terminate string */
}
/*}}}*/

/*{{{  mtl_bin2hex*/
/* to must not be same as from, since string will grow
 * (but we could go backwards so that we dont overwrite what
 * is still to be done, if that is required
 * (we cant treat from[] as a c string here, since 0 is valid binary data)
 */
void mtl_bin2hex(char *to, unsigned char *from, int count)
{
	while (--count >= 0)
	{
		static char table[]="0123456789ABCDEF";
		*to++ = table[*from >> 4];
		*to++ = table[*from & 15];
		++from;
	}
	*to = 0;
}
/*}}}*/

#if defined(VMS) && defined(VMS_MSG) 

/*{{{  versions for VMS message utility*/
extern unsigned int sys$putmsg();

/* output the message associated with the given integer message code */

/*{{{  mtl_putmsg*/
void mtl_putmsg(int status)
{
	static int desc[3] = { 1,0,0 };
   desc[1] = status;
   sys$putmsg(desc);
}
/*}}}*/

/*{{{  mtl_fatal*/
/* exits with a fatal status - VMS will output the message for us
 * - we trap re-entrant calls made by routines called by atexit(),
 *   simply outputting those messages
 * switch severity (bottom 3 bits) to fatal (4)
 */

void mtl_fatal(int message)
{
	/* guard against re-entrant calls */
	static int already_called=0;

	if (already_called)
		/* just display the message, with severity=fatal */
		mtl_putmsg( (message&~7) | 4);
	else
	{
		already_called=1;
		exit((message & ~7) | 4);  /* make severity=fatal */

		/* sci_close will be called by the atexit() mechanism */
	}
}	
/*}}}*/

/*{{{  mtl_sci_error*/
void mtl_sci_error(int number)
{
	if (number < 0 || number > 115)
		number=75;

	/* need cast because of feature of gnu c globalvalue
	 * implementation
	 * We choose the message numbers in messages.msg so that
	 * this would work
	 */

	mtl_fatal((message_type)(MTL__SCI000) + 8*number);
}
/*}}}*/
/*}}}*/

#else

/*{{{  generic versions*/
void mtl_fatal(char *x)
{
	/* trap possible fatal calls during fns called by atexit() */
	static int already_called=0;

	fprintf(stderr, "Fatal error: %s\n",x);

	if (!already_called)
	{
		already_called=1;
		exit(EXIT_FAILURE);
	}
}

void mtl_sci_error(int number)
{
	if (number < 0 || number > 115)
		number=75;
		
	mtl_fatal( sci_messages[number] );
}
/*}}}*/

#endif


#if defined(VMS) && defined(VMS_CLD)

/*{{{  VMS CLD version of mtl_commandline*/
/* mtl_commandline
 *
 * - parse commandline as directed by the options parameter
 * return parameter as char* to static store, or NULL if no parameter.
 *
 * VMS version ignores argc and argv, but they are supplied in order
 * to give a common interface
 *
 * treat /HELP as a special case
 */

/*{{{  includes and externs*/
#include <descrip.h>

extern unsigned int str$append();

extern unsigned int cli$present();
extern unsigned int cli$get_value();
extern unsigned int cli$get_value();

extern void *       lib$establish();
extern unsigned int lib$sig_to_ret();
/*}}}*/

/* support fns (inspired by gawk port to VMS)
 * if we ask cli to parse something that is is not defined in
 * the .cld file it will raise a fatal signal. We can establish
 * a standard signal handler (in stack frame => for this function only)
 * which traps a signal and makes it the return value of the function.
 * In fact, this is probably no longer necessary - initial implementation
 * parsed all mtools qualifiers for all programs, and the .CLD file
 * defined which were actually allowed. Now we only ask for the
 * appropriate subset, so any fatal errors are probably justfied.
 *
 * VMS passes strings by descriptor - simply a structure containing
 * stuff like length and pointer to first character. Most VMS languages
 * do all strings in this form, but of course C has a well-defined
 * storage format for strings. All a bit of a pain, but...
 */

/*{{{  cli_present*/
/* return bottom bit set if item present on command line */
static int cli_present(char *item)
{
	static $DESCRIPTOR(item_desc, "");
	item_desc.dsc$w_length = strlen(item);
	item_desc.dsc$a_pointer = item;

	lib$establish(lib$sig_to_ret);
	return cli$present(&item_desc);
}
/*}}}*/

/*{{{  cli__get__value*/
/* interface to VMS layer, so must return an int status
 * for use by cli_get_value.
 * result must point to a string descriptor, but there is more than
 * one kind of string descriptor, so just keep it generic (ie void *)
 */
 
static int cli__get__value(char *item, void *result)
{
	static $DESCRIPTOR(item_desc, "");
	
	item_desc.dsc$w_length = strlen(item);
	item_desc.dsc$a_pointer = item;

	lib$establish(lib$sig_to_ret);
	return cli$get_value(&item_desc, result);
}
/*}}}*/

/*{{{  cli_get_string*/
/* return a pointer to static store if option given
 * else return NULL. Must mtl_strdup() the string if you want
 * to keep it, since each call will overwrite the string
 */
 
static char *cli_get_string(char *item)
{
	static struct { unsigned short curlen; char body[256]; } result;
	static struct dsc$descriptor_s res_desc = { 256,DSC$K_DTYPE_VT, DSC$K_CLASS_VS, (void *)&result};
	
	int status = cli__get__value(item, &res_desc);
	if (!(status&1)) return NULL;

	result.body[result.curlen]=0;

	return result.body;
}
/*}}}*/


/* we dont need to check that required qualifiers are given
 * (ie the mask stuff) since DCL will do that for us (via .CLD file)
 */
char *mtl_commandline(int argc, char *argv[], option_struct *option, 
  int mask, char *help, int maxparam)
{
	char *p1;

	/* help is a special case */

	if (cli_present("HELP")&1)
	{
		fprintf(stderr, help, OS);
		exit(SUCCESS);
	}

	while (option->name)
	{
		switch(option->type)
		{
			case boolean:
				*(int *)option->var = cli_present(option->name)&1;
				break;
			case integer:
			{
				char *p = cli_get_string(option->name);
				if (p)
				{
					int val = atoi(p);
					if (val < option->min || val > option->max)
						FATAL(MTL__OUTOFRANGE);
					*(int *)option->var = val;
				}
				break;
			}
			case string:
			{
				char *val = cli_get_string(option->name);
				if (val)
				{
					if (strlen(val) > option->max)
						FATAL(MTL__TOOLONG);
					*(char **)option->var = mtl_strdup(val);
				}
				break;
			}
		}
		++option;
	}
	/* return command-line parameter, or NULL if none */

	if (maxparam)
	{
		char *p1 = cli_get_string("P1");
		if (!p1)
			return NULL;

		if (strlen(p1) > maxparam)
			FATAL(MTL__PARAMTOOLONG);
		return p1;
	}
	else
		return NULL;
}
/*}}}*/

#else

/*{{{  generic version of mtl_commandline*/
/*{{{  almost_equals()*/
/* compare x with string - accept any valid abbreviation of 'string', and
 * return the first character that did not match in x
 */
 
char *almost_equals(char *x, char *string)
{
	/* treat first character specially */

	if (*x != *string)
		return NULL;

	/* okay, it is a valid abbreviation - see how much they gave */

	while (*x && *x==*string++)
		++x;

	return x;
}
/*}}}*/
		

char *mtl_commandline(int argc, char *argv[], option_struct *option, int mask, char *help, int maxparam)
{
	char *param=NULL;
	
	while (--argc > 0 && (*++argv)[0] == SWITCH)
	{
		option_struct *opt;

		if (almost_equals(argv[0]+1, "help"))
		{
			fprintf(stderr, help, OS);
			exit(SUCCESS);
		}

		for (opt=option; opt->name; ++opt)
			/*{{{  see if it's this option*/
			{
				char *p = almost_equals(argv[0]+1,opt->name);
			
				if (!p)
					continue;
					
				/* get here => arg is a valid abbreviation of option */
			
				/* check that bit in mask not already cleared - mutually exclusive
				 * options, for example
				 */
			
				if (opt->mask && !(mask & opt->mask))
					FATAL(MTL__EXCLUSIVEOPT);
			
				/* make a note that we've met this option */
				mask &= ~(opt->mask);
			
				if (opt->type == boolean)
					/*{{{  check there's no value, then set flag and goto next*/
					{
						if (*p) FATAL(MTL__NOVALUE);
						*(int *)(opt->var) = 1;
						goto matched_opt;
					}
					/*}}}*/
				
				/* look for a value either immediately, after an =, or in next argv */
			
				if (*p == '=')
					/*{{{  use value after =*/
					{
						if (!*++p)
							FATAL(MTL__NEEDVALUE);
					}
					/*}}}*/
				else if (!*p)
					/*{{{  get value from next argv[]*/
					{
						if (--argc < 1)
							FATAL(MTL__NEEDVALUE);
						p = *++argv;
					}
					/*}}}*/
				/* else p already pointing at value */
			
				/*fprintf(stderr, "Value %s\n", p);*/
			
				/*{{{  store value according to opt->type*/
				switch(opt->type)
				{
					case boolean:
						abort(); /* cant get here */
					case integer:
						{
							char *end;
							int val = (int) strtol(p, &end, 0);
							if (*end)
								FATAL(MTL__BADNUMBER);
							if (val < opt->min || val > opt->max)
								FATAL(MTL__OUTOFRANGE);
							*(int *)opt->var = val;
						}
						break;
					case string:
						if (strlen(p) > opt->max)
							FATAL(MTL__TOOLONG);
						*(char **)opt->var = p;
						break;
					default:
						abort(); /* cant happen */
				}
				/*}}}*/
													
				goto matched_opt;	  /* I prefer this to setting and testing flags */
			}
			/*}}}*/

		/* get here => not matched */

		FATAL(MTL__BADOPT);
			
		matched_opt:
			; /* ANSI requires this null statement */

	}

	/* check that all required options have been given */

	if (mask)
		FATAL(MTL__MISSINGOPT);
		/* we could go through opt list and identify the ones we need */

	if (maxparam && argc)
	{
		param = *++argv;
		--argc;

		if (strlen(param) >= maxparam)
			FATAL(MTL__PARAMTOOLONG);
	}

	if (argc)
		WARN(MTL__CLGARBAGE);

	return (param);
}
/*}}}*/

#endif


/*{{{  raw transfer routines shared by mgetraw and mgetctrl*/
/*{{{  static void read_block()*/
/* generic routine to read one block
 * We're going to be sending a lot of very similar requests, so to
 * save shuffling a lot of strings back and forth, we take a preformed
 * request string as a parameter, and we tweak the hunk number as
 * required - in many cases, each reply comes in one hunk
 * offset of hunk number depends on getraw or getctrl, so take a pointer
 * Similarly, we bypass sci_ftransact(), and go directly to sci_transact(),
 * which will make it slightly more efficient.
 * Note that sci_transact() may append a CRC to the request message, but
 * when we update the hunk number, the request string will be truncated at
 * that point.
 * request should be an array of size SCI_BUFSIZ, with hunk number
 * already set to 01
 */

static void read_block(FILE *outfile, char *request, char *hp)
{
	int hunk=1, total;

	/* Remember to leave first two bytes free for mid
	 * Also recall that sci_transact() may append a CRC to
	 * the request, but that does not matter - when we
	 * sprintf the hunk number to the end of the string, that
	 * will truncate the string
	 */

	for(;;) /* we'll break out when we need to */
	{
		/* dont let sci_transact check error code, since it may
		 * be error 17 = no such block
		 */

		char *reply = sci_transact(0, request);
		int status;

		/*{{{  check status*/
		/* this is possibly non-portable - it could give
		 * alignment errors on some machines, and incorrect
		 * results on machine with different endian-ness
		 * We interpret first four bytes of reply as an integer,
		 * which allows us to check for 017 very quickly
		 * string will be "017\000" and, on vax, this is
		 * 0x00373130
		 */
		
		if ( (*(int *)reply) == 0x00373130)
			return; /* no such block/link */
			
		if ( sscanf(reply, "%3d", &status) != 1)
			FATAL(MTL__GARBAGE);
		
		/* it's quite cheap to check status again - it
		 * is probably valid data, so it will take us some
		 * time to process anyway
		 */
			
		if (status==17)
			return;  /* no such block/link */
			
		if (status != 0)
			mtl_sci_error(status); /* any other error is bad news */
		/*}}}*/

		if ( sscanf(reply+5, "%4d", &total) != 1)
			FATAL(MTL__GARBAGE);

		if (hunk==1)
		{	/* only now do we know that this is a valid block
			 * also, we now know the size, and it simplifies the
			 * raw-write if we store size here, at beginning of
			 * block of data in the file.  %.10s means print at
			 * most 10 characters
			 */
			fprintf(outfile, "%04d %.10s\n",total,hp-14);
		}
	
		fputs(reply+9, outfile);
		putc('\n', outfile);

		if (hunk*512 >= total)
			break;  /* we've got all data for this block */

		/* update hunk number part of request string */
		sprintf(hp, "%02d", ++hunk);
	} /* this is just a goto, to the for(;;) statement ! */

	putc('\n', outfile);
}
/*}}}*/

/* read all blocks for an MPC */
/*{{{  void read_mpc()*/
void read_mpc(int node, char controller, FILE *outfile, char *request, char *bp)
{
	/* for maximum efficiency, we only update parts of SCI request
	 * message that changes with each inner loop. request should
	 * be a buffer of size SCI_BUFSIZ, and for mgetraw, it should
	 * already have the file info filled in. bp points at the
	 * block address part of the request
	 */
	 
	/*{{{  read i/o area*/
	{
		char rack;  /* A to D */
		int card;   /* 1 to 8 */
		int point;   /* 1 to 32 */
	
		sprintf(bp,"=%02d%c",
			node,
			controller);
		
		for (rack='A'; rack <= 'D'; ++rack)
		{
			bp[4]=rack;
			for (card=1; card <= 8; ++card)
			{
				if (opt_verbose)
					fprintf(stderr, "\r=%02d%c%c%1d\r", node, controller, rack, card);
				bp[5] = '0' + card;
				for (point=1; point<=32; ++point)
				{	sprintf(bp+6, "%02d  051201", point);
					read_block(outfile, request, bp+14);
				}
			}
		}
	}
	/*}}}*/

	/*{{{  read block area*/
	{
		int block;
	
		/* prepare the part of the request that doesn't change */
		
		sprintf(bp,"=%02d%c-",
	   	node,
			controller);
	
		if (opt_verbose)
			fprintf(stderr, "\r=%02d%c- \r", node, controller);
	
		for (block=1; block <= 99; ++block)
		{
			sprintf(bp+5, "%02d   051201", block);
			read_block(outfile, request, bp+14);
		}
	}
	/*}}}*/
}
/*}}}*/

/* read all blocks for a PLC */
/*{{{  void read_plc()*/
void read_plc(int node, char controller, FILE *outfile, char *request, char *bp)
{
	/* for maximum efficiency, we only update parts of SCI request
	 * message that changes with each inner loop
	 */
	 
	/*{{{  read io area*/
	{
		int block;
	
		/* prepare the part of the request that doesn't change */
		
		sprintf(bp,"=%02d%c",
	   	node,
			controller);
	
		if (opt_verbose)
			fprintf(stderr, "\r=%02d%c%A  \r", node, controller);
	
		for (block=1; block <= 256; ++block)
		{
			sprintf(bp+4, "%03d   051201", block);
			read_block(outfile,request, bp+14);
		}
	}
	/*}}}*/

	/*{{{  read block area*/
	{
		int block;
	
		/* prepare the part of the request that doesn't change */
		
		sprintf(bp,"=%02d%c-",
	   	node,
			controller);
	
		if (opt_verbose)
			fprintf(stderr, "\r=%02d%c-  \r", node, controller);
	
		for (block=1; block <= 99; ++block)
		{
			sprintf(bp+5, "%02d   051201", block);
			read_block(outfile,request,bp+14);
		}
	}
	/*}}}*/
}
/*}}}*/

/*}}}*/

/*{{{  write_raw() - shared by putraw and putctrl*/
void write_raw(FILE *fp, char *request, char *bp)
{
	/* this does _all_ the work of writing a raw file, since it
	 * is virtually the same for mputraw and mputctrl
	 */
	 
	char line[50];

	/* I never trust fscanf */
	while (fgets(line, 20, fp))
	{
		int size;
		char *block=line+5;
		int hunk=1;
		char data[515];
	
		if ( sscanf(line, "%4d", &size) != 1)
			FATAL(MTL__BADRAWFILE);
	
		/*{{{  get rid of trailing \n on block address*/
		{
			int len = strlen(line);
			if (len<6 || line[len-1] != '\n')
				FATAL(MTL__BADRAWFILE); /* probably line too long */
			line[len-1]=0;
		}
		/*}}}*/
		
		/*{{{  send the data for this block address*/
		while (fgets(data,515,fp))
		{
			if (*data=='\n') /* blank line */
				break; /* got all data for this block - get next one */
				
			/*{{{  get rid of trailing \n on line*/
			{
				int len = strlen(data);
				/* fgets reads the trailing carriage-return
				 * check it's there, then remove it
				 */
				if (len<1 || data[len-1] != '\n')
					FATAL(MTL__BADRAWFILE);
				data[len-1]=0;
			}
			/*}}}*/
		
			/* SCI reply message has nothing that interests us
			 * (transact will check the error status)
			 */
			sprintf(bp,"%-10.10s%04d0512%02d%s",
			  block,
			  size,
			  hunk,
			  data);
		
			 ++hunk;
		}
		/*}}}*/
	}
}
/*}}}*/
