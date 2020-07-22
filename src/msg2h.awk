# msg2h.awk
# part of mtools package
# generates appropriate header file from master messages.msg file
# for message-utility version, outputs GLOBALVALUEREF's
# for generic version, outputs #defines for strings
#
# messages are in lines of the form
# IDENT <message>
# with some
# .BASE number
# directives so that we can control which numbers are associcated with
# particular numbers (for the SCI errors)
# used from the c program with  MTL__IDENT

BEGIN {
	print "/* messages.h - part of mtools";
	print " * generated automatically from messages.msg by msg2h.awk";
	print " */\n";
}

# bit for generic version - message defn that doesn't start SCI

/<.*>$/ && !/^SCI/ {
# can probably get the bit between < and > more easily, but I'm no expert
   start=index($0, "<")+1;
   end=index($0, ">");
   message = substr($0, start, end-start);
	generic = generic "#define MTL__" $1 " \"" message "\"\n";
}

# bit for VMS message version

/<.*>$/ {
	vms = vms "GLOBALVALUEREF(int, MTL__" $1 ");\n";
}

END {
	print "#if defined(VMS) && defined(VMS_MSG)";
	print vms;
	print "#else";
	print generic;
	print "#endif"
}
