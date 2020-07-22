/* file emulate.c
 * a simple SCI emulator, for use with mtools suite.
 * can be used in one of two ways
 * - standalone (external mode), communicating with main image by pkt layer
 *   (serial port or mailbox)
 * - inline (internal mode) - linked into mtools image instead of serial.c,
 *   presenting a pkt interface to the client image.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>  /* for processing ... */

#include "mtools.h"


static int stlen, endlen; /* lengths of framing strings */	

static FILE *fp=NULL;

static char buffer[SCI_BUFSIZ];

/* takes an sprintf-like format, and does the framing, crc, etc */
/*{{{  static int send(char *fmt, ...)*/
static void send(char *fmt, ...)
{
	va_list args;
	int len;

	strcpy(buffer, opt_stfr);
	
	va_start(args, fmt);
	len = vsprintf(buffer+stlen, fmt, args);
	va_end(args);

	if (opt_crc)
		len += sprintf(buffer+stlen+len, "%04X%s%c",
		   sci_crc(buffer+stlen),
		   opt_endfr,
		   SCI_TERMINATOR
		   );
	else
		len += sprintf(buffer+stlen+len, "%s%c", opt_endfr, SCI_TERMINATOR);
}
/*}}}*/

/*{{{  static void emulate(char *request)*/
static void emulate(char *request)
{
	int id, message;

	if (opt_crc)
	{
		int len=strlen(request);
		/* truncate at crc (dont bother checking it !) */
		request[len-4]=0;
	}
	
	if (sscanf(request, "%2X%2d", &id, &message) != 2)
		exit(EXIT_FAILURE);
	
	switch(message)
	{
		/*{{{  case SCI_CMD_LOOPBACK:*/
		case SCI_CMD_LOOPBACK:
			/* we need to insert an alarm byte */
			send("%02X%02d0%s", id, message, request);
			break;
		/*}}}*/

		/*{{{  case SCI_CMD_MSGPAIR:*/
		case SCI_CMD_MSGPAIR:
			{
				int min,max;
				char string[SCI_BUFSIZ], *p=string;
		
				if (sscanf(request+4, "%3d%3d", &min, &max) != 2)
				{
					send("%02X0%02d003",id,message); /* generic */
					break;
				}
		
				if ( (max-min)*17 + 20 > SCI_MAXMSG)
				{	send("%02X0%02d009",id,message); /* reply too long */
					break;
				}
		
				*p=0; /* in case min > max */
		
				while (min <= max)
				{
					p += sprintf(p, "ON%03d   OFF%03d  ,", min, min);
					++min;
				}
		
				send("%02X0%02d000%s", id,message, string);
				break;
			}
		/*}}}*/

		/*{{{  case SCI_CMD_CFS:*/
		case SCI_CMD_CFS:
		{
			send(
		/* header part */
		"%02X0%02d0000.10310.912.344.3219.87611.234"
		/* slot A */
		"031.1112.23.3440000.121.234"
		/* slot B */
		"072.2223.34.4551110.123.456"
		/* params for header part */
		,id,message);
			break;
		}
		/*}}}*/

		/*{{{  case SCI_CMD_REPORT:*/
		case SCI_CMD_REPORT:
			{
				/* this implementation returns an empty message at eof, but
				 * that's probably good enough for our purposes
				 */
				 
				int seq;
				static FILE *fp=NULL;
				if (sscanf(request+4, "%4d", &seq)!=1)
				{
					send("%02X0%02d003",id,message);
					break;
				}
		
				if (seq==1)
				/*{{{  open file*/
				{
					char filename[10];
					if (sscanf(request+22,"%9s", filename) != 1)
					{
						send("%02X0%02d003",id,message);
						break;
					}
					if (!(fp=fopen(filename, "r")))
					{
						send("%02X0%02d015",id,message);
						break;
					}
				}
				/*}}}*/
		
				/*{{{  return next line*/
				{
					char line[255];
					int len;
					
					if (!fgets(line, 255, fp))
					{
							/* sequence 0, no entries in this message */
							send("%02X0%02d0000000000", id,message);
							break;
					}
				
					len = strlen(line);
					/* strip the \n */
					line[--len]=0;
				
					send("%02X0%2d000%04d00100000%03d%s", id,message,++seq,len,line);
				}
				/*}}}*/
		
				break;
			}					
		/*}}}*/

		/*{{{  case SCI_CMD_GETASC:*/
		case SCI_CMD_GETASC:
		{
			static FILE *fp = NULL;
			long offset;
			char filename[10];
			char data[64], reply[SCI_BUFSIZ];
			
			if (sscanf(request+4, "%*10s%*3d%*1d%*2d%*2d%9s%*1d%8lx", filename, &offset) < 2)
				/*{{{  error*/
				{
					send("%02X0%02d003",id,message);
					break;
				}
				/*}}}*/
		
			if (offset)
				/*{{{  seek*/
				{
					fseek(fp, offset, 0);
				}
				/*}}}*/
			else
				/*{{{  open file*/
				{
					fprintf(stderr,"Reading file %s\n", filename);
				
					if (!(fp=fopen(filename,"r")))
						/*{{{  error*/
						{
							send("%02X0%02d015",id,message);
							break;
						}
						/*}}}*/
				}
				/*}}}*/
		
			if (!fgets(data, 64, fp))
				/*{{{  report end of file*/
				{
					send("%02X0%02d0000000000000000000000000", id, message);
					fclose(fp);
					break;
				}
				/*}}}*/
		
			/*{{{  encode data*/
			{
				char *p=data;
				char *q=reply;
			
				while (*p)
					q += sprintf(q, "%02X", *p++);
			
				*q=0;
			}
			/*}}}*/
		
			send("%02X0%02d000000000000000%08X%02X%08X%04X%s", id,message,ftell(fp), strlen(data), offset, strlen(data),reply);
			break;
		}
		/*}}}*/

		/*{{{  default:*/
		default:
			send("%02X0%02d%03d", id, message, 1); /* unknown message */
			break;
		/*}}}*/
	}
}
/*}}}*/

/*{{{  static void init()*/
static void init()
{
	mtl_environment();
	stlen = strlen(opt_stfr);
	endlen = strlen(opt_endfr);
}	
/*}}}*/

#ifdef INTERNAL

/*{{{  pkt_ interface*/
void pkt_open()
{
	init();
}

/* _receive_ a request from sci layer */

void pkt_send(int len, char *message)
{
	message[len-1]=0; /* remove the terminator */
	emulate(message);
}


/* _send_ a reply to the sci layer */

char *pkt_recv(int timeout)
{
	int len = strlen(buffer);
	buffer[len-1]=0; /* strip terminator */
	return buffer;
}

void pkt_close()
{
}
/*}}}*/

#else

/*{{{  main*/
int main()
{
	char *request;

	init();
	
	pkt_open();

	while (1)
	{
		int id,message;

		request=pkt_recv(0); /* no timeout */

		fprintf(stderr, "< %s\n", request);

		emulate(request);
		pkt_send(strlen(buffer), buffer);
	}

	pkt_close();

	return EXIT_SUCCESS;
}
/*}}}*/

#endif
