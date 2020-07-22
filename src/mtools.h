/*
 * file:     mtools.h
 * project:  Mtools
 * version:  1.00
 * date:     29/4/95
 * author :  David Denholm
 *
 * Purpose : master header file, with SCI configuration, message and
 *           status codes, etc
 */

/* compilation options */
 
#define DEBUG  /* if you want it */

#ifdef VMS
#define VMS_MSG /* to use VMS message utility */
#define VMS_CLD /* to use DCL command-line parsing */
#endif

#define OS    "VAX/VMS"
#define SWITCH '/'


#if defined(VMS) && defined(VMS_MSG)
# define SUCCESS ((int) MTL__SUCCESS)
#else
# define SUCCESS EXIT_SUCCESS
#endif

/*{{{  status codes MTL__* and INFO,WARN,FATAL macros - system-dependent*/
#if defined(VMS) && defined(VMS_MSG)

/*{{{  VMS versions - integers access messages from messages.msg*/
typedef int message_type;

/* feature of gnu c is that globalvalueref's are the wrong type. Need to
 * cast. We set the severity (bottom 3 bits) as appropriate.
 * The do..while is the comp.lang.c recommendation for macros
 */

#define FATAL(x) mtl_fatal( (int) (x) )
#define WARN(x)  mtl_putmsg( (((int) (x)) & ~7) )
#define INFO(x)  do { if (opt_verbose) mtl_putmsg( (((int) (x))&~7) | 3 ); } while(0)

/*{{{  gnu_hack.h since it wont be available on vanilla VAX C system*/
/* <gnu_hacks.h>
 *
 * This header file contains hacks that will simulate the effects of certain
 * parts of VAX-C that are not implemented in GNU-C.  This header file is
 * written in such a way that there are macros for both GNU-C and VAX-C.
 * Thus if you use the macros defined here in your program, you should be
 * able to compile your program with either compiler with no source
 * modifications.
 *
 * Note:  The globalvalue implementation is a little bit different than it
 * is on VAX-C.  You can declare a data variable to be a globalvalue, but
 * the data type that the compiler will see will be a pointer to that data
 * type, not that data type.  If you find the warning messages intollerable,
 * you can define a macro to type cast the globalvalue variable back to the
 * expected type.  Since it is impossible to define a macro while expanding
 * another macro, this was not done here.
 *
 * Also, globaldef/ref/value of enum is not implemented.  To simulate this
 * in GNU-C, you can globaldef an integer variable, and then define all of
 * the values that this variable can take to be globalvalue.
 *
 * The arguments to these macros are "fragile" (in the LaTeX sense).  This
 * means that you cannot use type casts or expressions in the name field.
 */
#ifndef _GNU_HACKS_H
#define _GNU_HACKS_H
#define __GNU_HACKS_H_

/* internal macros for traditional vs ANSI */
#ifdef __STDC__
#define __GNU_HACKS_string(foo) #foo
#define __GNU_HACKS_const const
#else
/* => -traditional, or VAXC (just "stringizing", not 'const') */
#define __GNU_HACKS_string(foo) "foo"
#define __GNU_HACKS_const
#endif

#ifdef __GNUC__

/* Store the data in a psect by the same name as the variable name */
#define GLOBALREF(TYPE,NAME) \
 TYPE NAME __asm("_$$PsectAttributes_GLOBALSYMBOL$$" __GNU_HACKS_string(NAME))
#define GLOBALDEF(TYPE,NAME,VALUE) \
 TYPE NAME __asm("_$$PsectAttributes_GLOBALSYMBOL$$" __GNU_HACKS_string(NAME)) = VALUE

#define GLOBALVALUEREF(TYPE,NAME) \
 __GNU_HACKS_const TYPE NAME[1] __asm("_$$PsectAttributes_GLOBALVALUE$$" __GNU_HACKS_string(NAME))
#define GLOBALVALUEDEF(TYPE,NAME,VALUE) \
 __GNU_HACKS_const TYPE NAME[1] __asm("_$$PsectAttributes_GLOBALVALUE$$" __GNU_HACKS_string(NAME)) = {VALUE}

/* Store the data in a psect by a different name than the variable name */
#define GBLREF_ALIAS(TYPE,NAME,ALIAS) \
 TYPE NAME __asm("_$$PsectAttributes_GLOBALSYMBOL$$" __GNU_HACKS_string(ALIAS))
#define GBLDEF_ALIAS(TYPE,NAME,ALIAS,VALUE) \
 TYPE NAME __asm("_$$PsectAttributes_GLOBALSYMBOL$$" __GNU_HACKS_string(ALIAS)) = VALUE

/* Other non-standard qualifiers */
#define _NOSHARE(NAME) NAME __asm("_$$PsectAttributes_NOSHR$$" __GNU_HACKS_string(NAME))
#define _READONLY(NAME) NAME __asm("_$$PsectAttributes_NOWRT$$" __GNU_HACKS_string(NAME))

/* as of gcc 2.0, this does not work, so do not use it... */
#define _ALIGN(NAME,BASE) NAME __attributes((aligned(1<<(BASE))))

#else
/* assume VAXC */
/* These are the equivalent definitions that will work in VAX-C */

#define GLOBALREF(TYPE,NAME) globalref TYPE NAME
#define GLOBALDEF(TYPE,NAME,VALUE) globaldef TYPE NAME = VALUE

#define GLOBALVALUEREF(TYPE,NAME) globalvalue TYPE NAME
#define GLOBALVALUEDEF(TYPE,NAME,VALUE) globalvalue TYPE NAME = VALUE

#define GBLREF_ALIAS(TYPE,NAME,ALIAS) globalref{__GNU_HACKS_string(ALIAS)} TYPE NAME
#define GBLDEF_ALIAS(TYPE,NAME,ALIAS,VALUE) globaldef{__GNU_HACKS_string(ALIAS)} TYPE NAME = VALUE

#define _NOSHARE(NAME) noshare NAME
#define _READONLY(NAME) readonly NAME
#define _ALIGN(NAME,BASE) _align(BASE) NAME

#endif
/*__GNUC__*/

#endif
/*_GNU_HACKS_H*/

/*}}}*/

/* pick up globalrefs of the form MTL__SCI001 automatically
 * generated from messages.msg by msg2h.awk
 */

#include "messages.h"
/*}}}*/

#else

/*{{{  generic versions - char * for MTOOLS messages, char ** for SCI errors*/
typedef char *message_type;
extern char *sci_messages[];

#include "messages.h"
#define INFO(x) do{if(opt_verbose)fprintf(stderr,"%s\n", x);}while(0)
#define WARN(x) fprintf(stderr,"warning: %s\n", x)
#define FATAL(x) mtl_fatal(x)
/*}}}*/

#endif
/*}}}*/

/*{{{  SCI configuration*/
/* maximum reply from getasc is 256 bytes = 512 hex digits.
 * Most efficient to transfer as much as possible at a time.
 * Need to allow for 27 bytes in overhead to user part of reply,
 * plus 3 bytes for MID and alarm, plus 4 bytes for CRC, plus up
 * to 8 bytes for framing. Call it 560
 *
 * in fact, for mgetrep, we can make use of even bigger messages,
 * up to 1024 in fact.
 */
#define SCI_MAXMSG  1024


/* I like to make buffers safely larger - arrays should all be this size */

#define SCI_BUFSIZ  1200


/* error handling approach is to listen after receiving a bad message,
 * since we assume there is more garbage in the buffer. This happens
 * when reply is bigger than MAXMSG, for example, or when a character
 * is corrupted to a terminator.
 * Initial timeout is SCI_TIMEOUT. It is probably safe to wait
 * a shorter period after receiving garbage, since it is
 * probably already in the incoming buffers
 * DO NOT SET EITHER OF THESE TO 0  (unless you do want to wait forever)
 */

/* things configured by RS console */
#define SCI_TERMINATOR '\n'  /* added to sent messages */

/* these can be over-ridden by setting environment variables */
#define SCI_STFR    "("
#define SCI_ENDFR   ")"  /* plus implicit unprintable terminator */
#define SCI_CRC     1    /* yes, by default */


/* TRANSMISSION ERROR HANDLING */

/* after send, we wait this many seconds for a reply */
#define SCI_TIMEOUT     10

/* after receiving garbage, we listen this many seconds more to
 * ensure buffers are flushed, before resending. Eg SCI sent a reply
 * bigger than our input buffer, or a character in middle of reply
 * got converted to 0x0d by line noise.
 */
#define SCI_CLEANUPTIME  4

/* each send costs 4, each listen costs 1 */
#define SCI_RETRIES     20
/*}}}*/

/*{{{  SCI commands*/
#define SCI_CMD_GETCTRL  11
#define SCI_CMD_PUTCTRL  12
#define SCI_CMD_GETRAW   13
#define SCI_CMD_PUTRAW   14
#define SCI_CMD_REPORT   62
#define SCI_CMD_GETASC   63
#define SCI_CMD_PUTASC   64
#define SCI_CMD_TIME     71
#define SCI_CMD_CFS      73
#define SCI_CMD_MSGPAIR  82
#define SCI_CMD_LOOPBACK 91

/*}}}*/

/*{{{  typedefs*/
/* allowed options are passed to mtl_commandline using this
 * structure. We implement check that all required qualifiers
 * are given using a mask word... mtl_commandline takes a bitmask,
 * and as qualifiers are matched, appropriate bits are cleared.
 * If mask is not zero after whole line parsed, there are missing
 * qualifiers. Optional qualifiers have mask field = 0
 * If qualifiers are either/or, they share the same mask.
 */

enum types { boolean, integer, string };

typedef struct {
	char *name;
	enum types type;
	void *var;
	int min, max; /* for integers - for string, max is max length */
	int mask;     /* for required qualifiers */
} option_struct;


/*}}}*/

/*{{{  public function prototypes*/
/**** low-level routines - probably used only by sci_* routines ****/

void pkt_open(void);                 /* open packet exchange channel */
void pkt_send(int size, char *msg); /* send a message */
char *pkt_recv(int timeout);       /* receive a message within given time */
void pkt_close(void);             /* close communications channel */


/**** sci protocol layer ****/

void sci_open(void);  /* establish link to SCI */
void sci_close(void); /* terminate SCI connection */

/* calculate crc on string */
unsigned short sci_crc(unsigned char *msg);

/* perform one transaction with SCI - handles msg ID, CRC,
 * framing, timeouts, etc. error is a boolean, whether sci_transact()
 * should look for, and check, a 3-digit error status in the usual place
 * in the reply. Caller can say no, then check manually if they so desire.
 */
char *sci_transact(int error, char *msg);

/* like transact, but takes a printf-style format and arguments */
char *sci_ftransact(int error, char *fmt, ...);



/**** utilities ****/

/* some RTL's provide a strdup function, some dont. Let's just make
 * our own. Returns a copy of text, in space malloc-ed from the heap
 */
char *mtl_strdup(char *text);

/* converts from hex-pairs to binary data. It is permissible for
 * from==to, so that data can be decoded inline.
 */
void mtl_hex2bin(unsigned char *to, char *from, int count);

/* converts from binary to hex-pairs. to must not be same as from */

void mtl_bin2hex(char *to, unsigned char *from, int count);

/* on VMS, message_type is int, and status is VMS condition code
 * On generic version, message_type is char *, and status is simply the
 * message text
 */

void mtl_fatal(message_type status);
void mtl_sci_error(int number); /* error number from SCI reply */

#if defined(VMS) && defined(VMS_MSG)
void mtl_putmsg(message_type status);
#endif

/* parse environment variables (logical names / symbols) into global
 * variables opt_*.
 */
void mtl_environment(void);

/* parse command line qualifiers into global variables opt_*
 * /help is treated as a special case - the supplied help string
 * can have one %s, which will be replaced with operating system
 * Return pointer to command parameter if given, else NULL if none.
 * (No param anticipated if maxparam is 0)
 * On entry, mask is bitwise 'or' of mask bits in options. As each
 * option is matched, that bit is cleared. If any bits are left,
 * not all required options have been specified and an error is reported.
 * (this is not used by VMS version, since DCL checks for this, but
 *  it could be done here instead if preferred)
 */
char *mtl_commandline(int argc, char *argv[], option_struct *options,
  int mask, char *help, int maxparam);

/* dump a controller to file - getraw and getctrl use the same protocol,
 * but the block address goes at different offsets into the request,
 * so we pass that offset as a parameter. The initial part of the
 * request (file id for getraw) should be set up before entry
 */

void read_mpc(int node, char controller, FILE *outfile, char *request, char *bp);
void read_plc(int node, char controller, FILE *outfile, char *request, char *bp);
/*}}}*/

/*{{{  global option variables*/
/* variables fetched from environment */

extern int   opt_crc;       /* is crc required */
extern char *opt_stfr;     /* start of frame  */
extern char *opt_endfr;   /* end of frame    */

#ifdef DEBUG
extern char *opt_debug;
#endif


/* all tools must provide a global opt_verbose */
extern int opt_verbose;
/*}}}*/
