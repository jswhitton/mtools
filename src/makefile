#CFLAGS = /warn/nocase

# invokes cc on remote node by decnet
CC = cc
CFLAGS = /warn

# set PACKET to serial or mailbox on the commandline, during testing
# hardwire it it SERIAL now that we're talking to a real SCI
PACKET = serial

SUPPORT   = $(PACKET).obj,mtools.obj,messages.obj
SCISUPPORT= sci.obj,$(SUPPORT)


all : mgetasc.exe mputasc.exe mgetmsg.exe mgetrep.exe mcfs.exe\
      mtime.exe   mconfig.exe mgetraw.exe mputraw.exe \
      mgetctrl.exe mputctrl.exe

####### the mtools #########

mtime.exe : mtime.obj,$(SCISUPPORT)
	$(LD) $(LDFLAGS) mtime,$(SCISUPPORT) $(CRTL_SHARE)

mgetasc.exe : mgetasc.obj,$(SCISUPPORT)
	$(LD) $(LDFLAGS) mgetasc,$(SCISUPPORT) $(CRTL_SHARE)

mputasc.exe : mputasc.obj,$(SCISUPPORT)
	$(LD) $(LDFLAGS) mputasc,$(SCISUPPORT) $(CRTL_SHARE)

mgetmsg.exe : mgetmsg.obj,$(SCISUPPORT)
	$(LD) $(LDFLAGS) mgetmsg,$(SCISUPPORT) $(CRTL_SHARE)

mgetrep.exe : mgetrep.obj,$(SCISUPPORT)
	$(LD) $(LDFLAGS) mgetrep,$(SCISUPPORT) $(CRTL_SHARE)

mcfs.exe : mcfs.obj,$(SCISUPPORT)
	$(LD) $(LDFLAGS) mcfs,$(SCISUPPORT) $(CRTL_SHARE)

mconfig.exe : mconfig.obj,$(SCISUPPORT)
	$(LD) $(LDFLAGS) mconfig,$(SCISUPPORT) $(CRTL_SHARE)

mgetraw.exe : mgetraw.obj,$(SCISUPPORT)
	$(LD) $(LDFLAGS) mgetraw,$(SCISUPPORT) $(CRTL_SHARE)

mputraw.exe : mputraw.obj,$(SCISUPPORT)
	$(LD) $(LDFLAGS) mputraw,$(SCISUPPORT) $(CRTL_SHARE)

mgetctrl.exe : mgetctrl.obj,$(SCISUPPORT)
	$(LD) $(LDFLAGS) mgetctrl,$(SCISUPPORT) $(CRTL_SHARE)

mputctrl.exe : mputctrl.obj,$(SCISUPPORT)
	$(LD) $(LDFLAGS) mputctrl,$(SCISUPPORT) $(CRTL_SHARE)


###### the test programs ########

pktclt.exe : pktclt.obj $(SUPPORT)
	$(LD) $(LDFLAGS) pktclt,$(SUPPORT) $(CRTL_SHARE)

pktsrv.exe : pktsrv.obj $(SUPPORT)
	$(LD) $(LDFLAGS) pktsrv,$(SUPPORT) $(CRTL_SHARE)

sciclt.exe : sciclt.obj $(SCISUPPORT)
	$(LD) $(LDFLAGS) sciclt,$(SCISUPPORT) $(CRTL_SHARE)

emulate.exe : emulate.obj $(SCISUPPORT)
	$(LD) $(LDFLAGS) emulate,$(SCISUPPORT) $(CRTL_SHARE)



####### dependencies #########

mtools.obj sci.obj serial.obj mailbox.obj : mtools.h messages.h

mgetasc.obj mputasc.obj mgetmsg.obj mgetrep.obj mcfs.obj mtime.obj : mtools.h messages.h

internal.obj : emulate.c mtools.h messages.h
	$(CC)/OBJ=$@ $(CFLAGS) $(CDFLAGS) /define=internal emulate.c

# all error messages are defined by messages.msg
# Thus they can easily be translated to other languages.
# For ease of maintainability, other files are generated automatically

messages.h : messages.msg
	gawk -f msg2h.awk messages.msg > messages.h

messages.obj : messages.msg
	message messages

#
# for generic version, use
#
#messages.c : messages.msg
#	gawk -f msg2c.awk messages.msg > messages.c
#messages.obj : messages.c
