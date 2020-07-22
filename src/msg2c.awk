# msg2c.awk
# part of mtools package
# generates a source file with a table of SCI error messages from
# messages.msg

BEGIN {
	print "/* messages.c - part of mtools";
	print " * generated automatically from messages.msg by msg2c.awk";
	print " */\n";
	print "#include <stdio.h>";
	print "char *sci_messages[] = {";
}

# messages are skipped using the .base directive

/^.base/ || /^.BASE/  {
	while (msg < $2) {
		print "\"\",";
		++msg;
	}
}

/^SCI.*>$/ {
# can probably get the bit between < and > more easily, but I'm no expert
   start=index($0, "<")+1;
   end=index($0, ">");
   message = substr($0, start, end-start);
	print "\"" message "\",";
	++msg;
}

END {
	print "NULL,\n};"
}
