/*
 * file:     serial.c
 * project:  Mtools
 *
 * Purpose : low-level packet read/write to serial port
 *
 */

#include <stdio.h> /* for NULL */
#include "mtools.h"

#if defined(VMS)

/*{{{  VMS version*/
/*{{{  includes*/
#include <descrip.h>
#include <iodef.h>
#include <ttdef.h>
#include <tt2def.h>
#include <dcdef.h>
#include <ssdef.h>
#include <stat.h>
#include <fab.h>
/*}}}*/

/*{{{  externs*/
extern unsigned int sys$assign();
extern unsigned int sys$dassgn();
extern unsigned int sys$qio();
extern unsigned int sys$qiow();
/*}}}*/

/*{{{  static variables*/
/* VMS I/O channel to the serial port */
static short pkt_chan=0;

/*}}}*/

/* useful macro for calling system routines and checking status, aborting
 * on failure. return from QIOW is whether it managed to post the request.
 * iosb is whether it actually went okay.
 */
 
#define CHECK(x)   { status=x; if (!(status&1)) FATAL(status); }
#define IOCHECK(x) { status=x; \
                     if (!(status&1)) FATAL(status); \
                     if (!(iosb[0]&1)) FATAL(iosb[0]); \
                   }

/*{{{  pkt_open()*/
/* pkt_open()
 * - open channel to serial port defined by logical name MTL_PORT
 * - set terminal characteristics
 */
 
void pkt_open()
{
	int status;
	unsigned short iosb[4];
	
	/* system services pass strings by descriptor */
	static $DESCRIPTOR(port, "MTL_PORT");

	CHECK( sys$assign(&port, &pkt_chan, 0, 0) )
}
/*}}}*/

/*{{{  pkt_send()*/
/* send a message - msg must already have terminator appended */

void pkt_send(int size, char *msg)
{
	int status;
	unsigned short iosb[4];
	IOCHECK(sys$qiow(0,pkt_chan,IO$_WRITEVBLK | IO$M_NOFORMAT,iosb,0,0, msg, size,0,0,0,0))
}
/*}}}*/

/*{{{  pkt_recv()*/
/* receive a message, or timeout
 * - return NULL on timeout, or on DATAOVERRUN (not sure what else
 *   we can return in the latter case)
 *
 * last-minute change - have twice got 'DATAOVERUN' fatal error.
 * the simple fix is to ensure that HOSTSYNC is enabled on the
 * terminal driver (allowing vax to send ^S and ^Q to the SCI
 * when the type-ahead buffer fills), and also to use ALTYPEAHD
 * to allow a large buffer. But user may want to run SCI without
 * flow control, so we trap DATAOVERUN and return that as NULL.
 */

char *pkt_recv(int timeout)
{
	/* terminal driver allows us to specify which characters terminate
	 * an incoming message. For simple usage, it is one bit per
	 * character value from 0 to 31.
	 * We allow all as terminators
	 */

	static char pkt_recv_buffer[SCI_BUFSIZ];

	static unsigned int terminator[2] = { 0, 0xffffffff };

	int status;
	unsigned short iosb[4];  /* i/o status block */

	/* not IOCHECK - check that request succeeded, but not final io status */
	CHECK(sys$qiow(0,pkt_chan,
	        IO$_READVBLK | IO$M_NOECHO | IO$M_NOFILTR | (timeout > 0 ? IO$M_TIMED : 0) ,
	        iosb,0,0, pkt_recv_buffer, SCI_MAXMSG, timeout,
	        terminator, 0,0))

	if (iosb[0] == SS$_TIMEOUT)
		return NULL;

	if (iosb[0] == SS$_DATAOVERUN)
	{
		WARN(SS$_DATAOVERUN);
		return NULL;  /* it's worth retrying */
	}

	/* _now_ we can check for other errors */

	if (!(iosb[0]&1)) FATAL(iosb[0]);

	/* length of message (offset to terminator) is in iosb[1]
	 * terminate msg to make it a valid c string
	 */
	pkt_recv_buffer[ iosb[1] ]=0;

	return pkt_recv_buffer;
}
/*}}}*/

/*{{{  pkt_close()*/
/* pkt_close()
 * - restores terminal attributes
 * - closes channel
 *
 * guard against re-entrant calls (since sci_close() is an exit handler
 */

void pkt_close()
{
	if (pkt_chan) /* dont try to close channel if it's not open */
	{
		/* close channel */
		sys$dassgn(pkt_chan); /* not worth a fatal error if we cant close it */
		pkt_chan=0;
	}
}
/*}}}*/
/*}}}*/

#else
#if defined(UNIX)

/*{{{  unix version*/
#error not yet implemented
/*}}}*/

#else
#error Which version ?
#endif
#endif
