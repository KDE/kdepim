#!/usr/bin/awk -f

{
	outfile = $1 "_generated.h"
	
	OFS=""
	
	print "// XXX Automatically generated. DO NOT EDIT! XXX //\n" > outfile
	
	if ($2 == "v") { pre = "virtual " } else { pre = "" }

        print "public:" >> outfile
	print $1 "();" >> outfile
	print $1 "(const " $1 " &);" >> outfile
	print $1 "(const QCString &);" >> outfile
	print $1 " & operator = (const " $1 " &);" >> outfile
	print $1 " & operator = (const QCString &);" >> outfile
	print "bool operator == (" $1 " &);" >> outfile
	print "bool operator != (" $1 " & x) { return !(*this == x); }" \
			>> outfile
	print "bool operator == (const QCString & s) { " $1 " a(s); " \
			"return (*this == a); } " >> outfile
	print "bool operator != (const QCString &s) {return !(*this == s);}\n" \
			>> outfile
	print "virtual ~" $1 "();" >> outfile
	print pre "void createDefault();\n" >> outfile
	print pre "const char * className() const { return \"" $1 "\"; }\n" >> outfile
        print "protected:" >> outfile	
	print pre "void _parse();" >> outfile
	print pre "void _assemble();" >> outfile
	
        print "\n// End of automatically generated code           //" >> outfile
}

