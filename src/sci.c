/*
 * file:     sci.c
 * project:  Mtools
 *
 * Purpose : implement reliable link to SCI
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "mtools.h"

/*{{{  static variables and function prototypes*/
/* the next sci transaction message id */
static unsigned char sci_msg_id;

/* for converting integer to ascii-hex */
static char sci_hex_digits[]="0123456789ABCDEF";

static FILE *sci_id_fp=NULL;
/*}}}*/

/* load fetches the id from file. We leave file open and rewound,
 * so that there is less for sci_save to do - it may be called
 * as an exit handler, and I prefer to minimise work done during
 * exit
 */
 
/*{{{  sci_load*/
static void sci_load()
{
	char *p = getenv("MTL_MID");
	if (!p)
	{
		WARN(MTL__CANTCREATEID);
		return; /* not a lot we can do */
	}
	
	if ( (sci_id_fp = fopen("MTL_MID", "r+")) )
		/*{{{  read id, then rewind file*/
		{	int i;
			if (fscanf(sci_id_fp, "%x", &i) == 1)
				sci_msg_id = i;
			else
				WARN(MTL__BADIDFILE);
		
			/* to save having to do it in sci_save() */
			rewind(sci_id_fp);
		}
		/*}}}*/
	else
		/*{{{  moan, and try to create one*/
		{
			WARN(MTL__NOIDFILE);
		
			/* create it for sci_save */
			if ((sci_id_fp = fopen("MTL_MID", "w")))
				INFO(MTL__CREATEID);
			else
				WARN(MTL__CANTCREATEID);
		}
		/*}}}*/
}
/*}}}*/

/*{{{  sci_save*/
static void sci_save()
{
	if (sci_id_fp)  /* mtl_load will already have issued warnings */
	{
		/* sci_load has already rewound it - this may be called as
		 * part of exit handler, so we want to have to do as little
		 * as possible here
		 */
		 
		fprintf(sci_id_fp, "%02x\n", sci_msg_id);
	}
}
/*}}}*/

/* crc, as defined in PW manual */

/*{{{  sci_crc*/
unsigned short sci_crc(unsigned char *msg)
{
	unsigned short crc=0x5a5a;

	while (*msg)
	{
		crc ^= *msg;
		crc = (crc<<7) | ((crc >> 9)&0x7f);
		crc ^= *msg;
		++msg;
	}
	
	return crc;
}
/*}}}*/

/*{{{  sci_open*/
/* sci_open
 * - establishes a reliable channel to the SCI
 */

void sci_open()
{
	pkt_open();

	sci_load();

	/* make sure that sci_close is called, in order to save mid.sys
	 * it will sort out duplicate calls
	 */
	 
	atexit(sci_close);

	/*{{{  try to send a loopback message to SCI to establish comms*/
	{
		char *reply;
		char loopmsg[SCI_BUFSIZ];
		/* send loopback message with current time. This should
		 * distinguish legitimate reply from a repeat of a reply
		 * from SCI's cache
		 */
 
		sprintf(loopmsg+2, "%2d%08X", SCI_CMD_LOOPBACK, time(NULL));
		/* sprintf(loopmsg+2, "%2dhello", SCI_CMD_LOOPBACK); */
	
		reply = sci_transact(0,loopmsg);
	
		/* loopmsg has had CRC and terminator appended, but they're
		 * stripped from reply. Reply has been moved past message id,
		 * alarm message and type, but SCI has also included them
		 * in the data part of the reply. sci_transact wrote the
		 * message id to the start of loopmsg string
		 */
	
		if (strncmp(loopmsg, reply,12) != 0)
			FATAL(MTL__NOCOMMS);
	}
	/*}}}*/

	INFO(MTL__ESTABLISHED);
}
/*}}}*/

/*{{{  sci_transact*/
/* sci_transact()
 * - perform one transaction (one request and one reply) with the SCI
 * - first two bytes of msg will be overwritten with the message ID.
 * - there should storage available at the end of msg, so that the
 *   crc and terminator can be appended
 * - This routine checks for error status in the SCI reply. All replies
 *   except loopback provide an error field. If using loopback, it is
 *   up to caller to make the first 3 characters of the loopback = 0, so
 *   that sci_transact interprets the reply as successful.
 * - sci_transact will strip message type and error status from head
 *   of reply, and crc and terminator from tail.
 * last minute change - not all replies have error field, so make
 * that a parameter
 */

char *sci_transact(int error, char *msg)
{
	int retries = SCI_RETRIES; /* how many times we can re-request
	                            * send costs 4, listen costs 1
	                            */
	char *reply;
	int len=2+strlen(msg+2); /* first 2 bytes are garbage, possibly 0 */
	unsigned char this_id = sci_msg_id++;
	int timeout;

#ifdef DEBUG
	if (opt_debug) fprintf(stderr,">  %s\n", msg+2);
#endif

	/*{{{  fill in message id*/
	msg[0] = sci_hex_digits[this_id >> 4];
	msg[1] = sci_hex_digits[this_id & 15];
	/*}}}*/
	/*{{{  append a crc if necessary*/
	if (opt_crc)
	{
		int crc = sci_crc(msg);
		sprintf(msg+len, "%04X", crc);
		len += 4;
	}
	/*}}}*/
	/*{{{  append terminator*/
	/* we could just use the c NUL string terminator, but... */
	msg[len] = SCI_TERMINATOR;
	msg[++len] = 0; /* keep it a valid c string */
	/*}}}*/

#ifdef DEBUG
	if (opt_debug) fprintf(stderr,">> %s", msg);
#endif

	/* many people despise goto, but sometimes it is invaluable */

send:

	if ( (retries -= 4) < 0)
		FATAL(MTL__TOOMANYRETRIES);

	pkt_send(len, msg);
	timeout = SCI_TIMEOUT;

listen:

	if (--retries < 0)
		FATAL(MTL__TOOMANYRETRIES);

	reply = pkt_recv(timeout);
	/*{{{  check for timeout*/
	if (!reply)
	{
		INFO(MTL__TIMEOUT);
		goto send;
	}
	/*}}}*/

	timeout = SCI_CLEANUPTIME; /* in case we need to listen some more */

#ifdef DEBUG
	if (opt_debug) fprintf(stderr,"<< %s\n", reply);
#endif

	/*{{{  check and strip framing and crc*/
	{
		char *p = opt_stfr;
	
		/* no function to compare string one with string two for as
		 * long as there are characters in string one. No matter, it's
		 * very easy
		 */
		 
		while (*p)
		{
			if (*p++ != *reply++)
			{
				INFO(MTL__BADFRAME);
				goto listen;
			}
		}
	
		/* reply has now moved past the start frame */
	
		p = reply + strlen(reply) - strlen(opt_endfr);
		
		/* p now pointing at trailing frame marker in reply.
		 * Actually, we ought to check that there were at least that
		 * many characters in the reply - eg we might just have
		 * received (
		 */
	
		if (p <= reply)
		{	/* nothing there */
			INFO(MTL__BADFRAME);
			goto listen;
		}
		
		if (strcmp(p, opt_endfr) != 0)
		{	/* badly framed reply */
			INFO(MTL__BADFRAME);
			goto listen;
		}
	
		*p=0; /* terminate reply before trailing frame */
	
		if (opt_crc)
		{
			unsigned int their_crc, our_crc;
			p -= 4;  /* move to start of crc */
			if (sscanf(p, "%4x", &their_crc) != 1)
			{
				INFO(MTL__BADCRC);
				goto listen;
			}
			*p=0; /* terminate reply before crc */
			our_crc = sci_crc(reply);
			if (our_crc != their_crc)
			{
				INFO(MTL__BADCRC);
				goto listen; /* could probably goto send here */
			}
		}	
	}
	/*}}}*/
	/*{{{  check message ID and error status*/
	{
		int id;
	
		if (sscanf(reply, "%2x", &id) < 1)
		{
			INFO(MTL__NOID);
			goto listen;
		}
		
		if (id != this_id)
		{
			INFO(MTL__WRONGID);
			goto listen;
		}
	}
 
	reply += 5; /* the 2-digit message id, 1-digit alarm and 2-digit type */ 
 
	if (error)
	{
		int status;
		if (sscanf(reply, "%3d", &status) < 1)
		{
			INFO(MTL__NOSTATUS);
			goto listen;
		}
			
		if (status)
			mtl_sci_error(status);
	
		reply += 3; /* skip it */
	}
	/*}}}*/

#ifdef DEBUG
	if (opt_debug) fprintf(stderr,"< %s\n", reply);
#endif

	return reply;
}
/*}}}*/

/*{{{  sci_ftransact*/
/* a printf-style format onto transact */
char *sci_ftransact(int error, char *fmt, ...)
{
	char buffer[SCI_BUFSIZ];
	va_list args;

	va_start(args, fmt);
	vsprintf(buffer+2, fmt, args); /* leave two bytes for id */
	va_end(args);

	return sci_transact(error,buffer);
}
/*}}}*/

/*{{{  sci_close*/
/* sci_close
 * - closes connection to SCI
 * - stores message ID for next invocation
 *
 * - may be called by atexit() mechanism, so avoid calling exit,
 *   including calls to mtl_fatal()  [but mtl_fatal does keep a flag
 *   to prevent re-entrant calls]
 */


void sci_close()
{
	static int already_called = 0;

	if (!already_called)
	{
		already_called=1;
		sci_save(); /* save id */
		pkt_close();
		INFO(MTL__CLOSED);
	}
}
/*}}}*/
