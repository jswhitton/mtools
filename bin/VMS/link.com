$! relink the mtools
$!
$! recompile the messages file, in case facility has changed
$! make a list file, in case the actual error numbers are required
$ message messages
$
$! and relink all the tools
$
$ link  mgetasc.obj,sci.obj,serial.obj,mtools.obj,messages.obj ,sys$input/opt
sys$library:vaxcrtl/share
$ link  mputasc.obj,sci.obj,serial.obj,mtools.obj,messages.obj ,sys$input/opt
sys$library:vaxcrtl/share
$ link  mgetmsg.obj,sci.obj,serial.obj,mtools.obj,messages.obj ,sys$input/opt
sys$library:vaxcrtl/share
$ link  mgetrep.obj,sci.obj,serial.obj,mtools.obj,messages.obj ,sys$input/opt
sys$library:vaxcrtl/share
$ link  mcfs.obj,sci.obj,serial.obj,mtools.obj,messages.obj ,sys$input/opt
sys$library:vaxcrtl/share
$ link  mtime.obj,sci.obj,serial.obj,mtools.obj,messages.obj ,sys$input/opt
sys$library:vaxcrtl/share
$ link  mconfig.obj,sci.obj,serial.obj,mtools.obj,messages.obj ,sys$input/opt
sys$library:vaxcrtl/share
$ link  mgetraw.obj,sci.obj,serial.obj,mtools.obj,messages.obj ,sys$input/opt
sys$library:vaxcrtl/share
$ link  mputraw.obj,sci.obj,serial.obj,mtools.obj,messages.obj ,sys$input/opt
sys$library:vaxcrtl/share
$ link  mgetctrl.obj,sci.obj,serial.obj,mtools.obj,messages.obj ,sys$input/opt
sys$library:vaxcrtl/share
$ link  mputctrl.obj,sci.obj,serial.obj,mtools.obj,messages.obj ,sys$input/opt
sys$library:vaxcrtl/share
$ exit
