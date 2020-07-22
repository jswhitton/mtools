/*
 * file:     mailbox.c
 * project:  Mtools
 *
 * Purpose : use mailbox to emulate the serial port, for testing
 * - provides routines with same interface as serial.c
 * - because we only use one mailbox, sleep for a few seconds after
 *  a write, so that partner gets the message, rather than us getting
 *  it back when we subsequently read the mailbox
 */

/*{{{  includes*/
#include "mtools.h"

#include <descrip.h>
#include <iodef.h>
#include <ttdef.h>
#include <tt2def.h>
#include <dcdef.h>
#include <ssdef.h>
#include <stat.h>
#include <fab.h>

#include <unixio.h>
#include <fcntl.h>

/*}}}*/

/*{{{  prototypes*/
int sys$assign();
int sys$dassgn();
int sys$crembx();
/*}}}*/

/*{{{  static variables*/
/* file handle onto mailbox */
static int pkt_chan=0;

/*}}}*/

/* useful macro for calling system routines and checking status, returning
 * the status to the caller on failure
 */
 
#define CHECK(x) { int status=x; if (!(status&1)) FATAL(status); }

/*{{{  pkt_open()*/
/* pkt_open()
 * - create mailbox, and open for C i/o read/write
 *  (manual recommends language i/o, since a write will wait for
 * partner to read; we are using one mailbox for bi-directional comms
 */
 
void pkt_open()
{
	short chan;
	
	/*{{{  create mailbox*/
	{
		/* system services pass strings by descriptor */
		static $DESCRIPTOR(port, "MTL_MAILBOX");
	
		CHECK (sys$crembx(0, &chan, 0,0,0,0, &port) )
	}	
	/*}}}*/
	
	/*{{{  get a c handle onto it*/
	if ( (pkt_chan = open("MTL_MAILBOX", O_RDWR, 0, "rfm=var")) == -1)
	{
		perror("perror says");
		FATAL(4); /* fatal error */
	}
	/*}}}*/

	/*{{{  give up the VMS channel*/
	sys$dassgn(chan);
	/*}}}*/
}

/*}}}*/

/*{{{  pkt_send()*/
/* send a message - msg must already have terminator appended */

void pkt_send(int size, char *msg)
{
	if (write(pkt_chan, msg, size) < size)
	{
		perror("perror says");
		exit(4);
	}
	sleep(2); /* prevent a read from being posted too soon */
}

/*}}}*/

/*{{{  pkt_recv()*/
/* receive a message, or timeout
 * - replace terminator with 0 to make it valid c string
 */

char *pkt_recv(int timeout)
{
	static char recv_buffer[SCI_BUFSIZ];
	
	int size = read(pkt_chan, recv_buffer, SCI_MAXMSG);
	char *end = recv_buffer+size;

	if (size < 0)
		abort(); /* we can do this in debugging code !!! */

	while (end > recv_buffer && end[-1] <32)
		--end;

	*end = 0;
	return recv_buffer;
}
/*}}}*/

/*{{{  pkt_close()*/
/* pkt_close()
 * - restores terminal attributes
 * - closes channel
 */

void pkt_close()
{
	if (pkt_chan) /* guard against repeated closure */
	{
		close(pkt_chan);
		pkt_chan=0;
	}
}
/*}}}*/


