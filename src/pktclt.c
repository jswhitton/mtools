/* simple test for the packet routines. Can be linked against either
 * packet.obj or mailbox.obj - open driver, send a message, receive a reply,
 * send a message, close driver.
 *
 * for mailbox, can be used with pktserver...
 *  $ spawn/nowait/inp=nl:/out=pktserver.out run pktserver
 *  $ run pktclient
 *
 * (can also do this with serial line, if you have two vaxes, or
 *  connect two serial ports with a serial cable)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mtools.h"

main()
{
	char *reply;
	char farewell[SCI_BUFSIZ];

	pkt_open();
	pkt_send(15,"who are you ? ");

	reply = pkt_recv();
	if (reply)
		printf("[received message '%s']\n", reply);
	else
	{
		printf("[read timed out]\n");
		reply="(anonymous)";
	}

	sprintf(farewell, "Pleased to meet you, %s", reply);
	pkt_send(strlen(farewell),farewell);

	pkt_close();

	return EXIT_SUCCESS;
}
