#!/usr/bin/awk -f

{
	outfile = $1 "_generated.h"

  N = $1
	
	OFS=""
	
	print "// XXX Automatically generated. DO NOT EDIT! XXX //\n" > outfile
	 
  pre = "virtual "

	if ($2 != "") {
  
    print "public:" >> outfile
    print N "() {}" >> outfile
    print N "(const " N " & other) : " $2 "(other) {}" >> outfile
    print N "(const QCString & s) : " $2 "(s) {}" >> outfile
    print "virtual ~" N "() {}" >> outfile
    print "virtual const char * className() const { return \"" N "\"; }\n" >> outfile
  } else {

    print "public:" >> outfile
    print N "();" >> outfile
    print N "(const " N " &);" >> outfile
    print N "(const QCString &);" >> outfile
    print N " & operator = (const " N " &);" >> outfile
    print N " & operator = (const QCString &);" >> outfile
    print "friend QDataStream & operator >> (QDataStream & s, " N " &);" \
    >> outfile
    print "friend QDataStream & operator << (QDataStream & s, " N " &);" \
    >> outfile
    print "bool operator == (" N " &);" >> outfile
    print "bool operator != (" N " & x) { return !(*this == x); }" \
    >> outfile
    print "bool operator == (const QCString & s) { " N " a(s); " \
    "return (*this == a); } " >> outfile
    print "bool operator != (const QCString &s) {return !(*this == s);}\n" \
    >> outfile
    print pre "~" N "();" >> outfile
    print pre "bool isNull() { parse(); return strRep_.isEmpty(); }" >> outfile
    print pre "bool operator ! () { return isNull(); }" >> outfile
    print pre "void createDefault();\n" >> outfile
    print pre "const char * className() const { return \"" N "\"; }\n" >> outfile
    print "protected:" >> outfile	
    print pre "void _parse();" >> outfile
    print pre "void _assemble();" >> outfile
  }
  
  print "\n// End of automatically generated code           //" >> outfile
}

