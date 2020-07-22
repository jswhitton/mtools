$! set up process environment for Mtools.
$! things would be done rather differently for a system-wide installation:
$! 1) verb is put into sys$library:dcltables.exe (once)
$! 2) help info put into a shared help file
$! 3) logicals defined in sys$manager:sylogicals.com
$!
$! Find the directory spec. for the install procedure
$!
$ Procedure:='f$environment("PROCEDURE")'
$ Device:='f$parse(Procedure,,,"DEVICE","NO_CONCEAL")'
$ Directory:='f$parse(Procedure,,,"DIRECTORY","NO_CONCEAL")'
$ define mtl_cmds 'Device''Directory'
$!
$ define mtl_mid   mtl_cmds:mid.sys
$ define mtl_port  tta3:
$ define mtl_chksm "y"
$
$ set command mtl_cmds:mtools
$
$ if f$trnlnm("HLP$LIBRARY") .eqs. ""
$ then
$   define hlp$library mtl_cmds:mtools.hlb
$   goto got_help
$ endif
$
$ i=1
$help_loop:
$ if f$trnlnm("HLP$LIBRARY_''i'") .eqs. ""
$ then
$   define hlp$library_'i' mtl_cmds:mtools.hlb
$   goto got_help
$ endif
$ i=i+1
$ if i .le. 8 then goto help_loop
$
$got_help:
$!
$! aliases
$!
$ mgetasc :== mgasc
$ mputasc :== mpasc
$ mgetrep :== mgrep
$ mgetmsg :== mgmsg
$ mgetctrl:== mgctrl
$ mputraw :== mpraw
$
$ exit
