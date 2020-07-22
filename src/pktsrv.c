/* a simple server for pktclient.c test program.
 * receive one message, send a message, receive a message
 * can be linked against either packet.obj or mailbox.obj
 */

#include "mtools.h"

#include <stdio.h>
#include <stdlib.h>

main()
{
	char *msg;
	
	pkt_open();

	msg=pkt_recv();
	if (msg) printf("[got message '%s']\n", msg); else printf("[Read timed out]\n");

	pkt_send(9,"pktserver");
	
	msg = pkt_recv();
	if (msg) printf("[got message '%s']\n", msg); else printf("[Read timed out]\n");

	pkt_close();

	return EXIT_SUCCESS;
}
