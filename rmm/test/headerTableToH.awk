#!/bin/awk -f

BEGIN {
i = 0

}

/^xxx/ {
	getline
	while ($1 !~ /xxx/) { s1[idx++] = $0 ; getline }
	getline
}

/^---/ {
	getline
	while ($1 !~ /---/) { s2[idx2++] = $0 ; getline }
	getline
}

/^extra/ {
	a = $2; getline
	while ($1 !~ /end/) { extra[n++] = a ":" $0 ; getline}
}

function f(a) {
	b = tolower(substr(a, 0, 1)) substr(a, 2)
	gsub("-", "", b)
	return b
}

function f2(a) {
	gsub("-", "", a)
	return a
}

function printextracpp(sec, olda, outfile) {
	
	oldOFS=OFS
	OFS=""
	for (x in extra) {

		if (substr(extra[x], 0, length(olda)) == olda) {
			arse = extra[x]
			gsub(olda ":", "", arse);
			count = split(arse, bits, /\//);

			if (bits[1] == sec) {
				
				if (bits[2] == "f") { # function

					functargs = bits[5];
					
					print "\t" bits[3] "\n" olda "::" bits[4] \
						"(" bits[5] ")" bits[6] "\n{\n}\n" >> outfile
				}
			}
		}
	}
	OFS=oldOFS
}

function printextra(sec, olda, outfile) {
	
	oldOFS=OFS
	OFS=""
	for (x in extra) {
		
		if (substr(extra[x], 0, length(olda)) == olda) {
			arse = extra[x]
			gsub(olda ":", "", arse);
			count = split(arse, bits, /\//);
			if (bits[1] == sec) {
				
				if (bits[2] == "f") { # function

					functargs = bits[5];
					
					print "\t\t" bits[3] " " bits[4] \
						"(" bits[5] ")" bits[6] ";" >> outfile

				} else if (bits[2] == "v") {

					print "\t\t" bits[3] " " bits[4] "_;" >> outfile
				}
			}
		}
	}
	OFS=oldOFS
}

function mkh (a) {
	outfile = "include/RMM_" a ".h"
	print "Generating " outfile
	olda = a
	a = "R" a
	underscored = toupper(a) "_H"
	gsub("-", "_", underscored)
	print "// XXX Automatically generated. DO NOT EDIT!" > outfile
	print >> outfile
	print "#ifndef RMM_" underscored >> outfile
	print "#define RMM_" underscored >> outfile
	print >> outfile
	print "#include <qstring.h>" >> outfile
	print >> outfile
	print "class " a " : public RHeaderBody {" >> outfile
	print >> outfile
	print "\tpublic:" >> outfile
	print >> outfile
	print "\t\t" a "();" >> outfile
	print "\t\t" a "(const " a " & " f(a) ");" >> outfile
	print "\t\tconst " a " & operator = (const " a " & " f(a) ");" >> outfile
	print >> outfile
	print "\t\tvirtual ~" a "();" >> outfile
	print >> outfile
	print "\t\tvirtual void parse();" >> outfile
	print "\t\tvirtual void assemble();" >> outfile
	print >> outfile
	print "\t\tbool isValid() const;" >> outfile
	print >> outfile
	printextra("pub", olda, outfile)
	print >> outfile
	print "\tprivate:" >> outfile
	print >> outfile
	printextra("pri", olda, outfile)
	print "\t\tbool isValid_;" >> outfile
	print "};" >> outfile
	print  >> outfile
	print "#endif //" underscored >> outfile
}

function mkcpp (a) {
	olda = a
	a = "R" a
	outfile = a ".cpp"
	print "Generating " outfile
	print "// XXX Automatically generated. Edits will be lost.. maybe\n" \
		> outfile
	print "#include <qstring.h>" >> outfile
	print "#include <RMM_" olda ".h>\n" >> outfile
	print a "::" a "()\n{\n}\n" >> outfile
	print a "::" a "(const " a " & " f(a) ")\n{\n}\n" >> outfile
	print "const " a " &\n" a "::operator = (const " a " & " f(a) ")\n{\n}\n" >> outfile
	print a "::~" a "()\n{\n}\n" >> outfile
	print "\tvoid\n" a "::parse()\n{\n}\n" >> outfile
	print "\tvoid\n" a "::assemble()\n{\n}\n" >> outfile
	print "\tbool\n" a "::isValid() const\n{\n}\n" >> outfile
	printextracpp("pub", olda, outfile)
}

/^$/ {next}
/^#/ {next}

/^def/ {
	defs[$2] = $3
	next
}

{
	headers[nheaders++] = $1
}

function writeToREnvelopeH() {

	o = "include/RMM_Envelope.h"
	
	# Print the top of the page
	for (a = 0; a < idx; a++) {
		print s1[a] > o
	}
	
	print >> o

	# Enums
	OFS=""
	print "enum HeaderType {" >> o
	for (a = 0; a < nheaders; a++) {
		if (a == nheaders - 1) print "\tHeader" f2(headers[a]) >> o
		else print "\tHeader" f2(headers[a]) "," >> o
	}
	print "};" >> o
	print >> o

	# char * headerNames[]
	print "char * headerNames[] = {" >> o
	for (a = 0; a < nheaders; a++) {
		if (a == nheaders - 1) print "\t\"" headers[a] "\"" >> o
		else print "\t\"" headers[a] "\"," >> o
	}
	print "};" >> o
	print >> o
	
	# end of top of page
	for (a = 0; a < idx2; a++) {
		print s2[a] >> o
	}

	print >> o

	print "\t\tvoid has(HeaderType t);" >> o

	print >> o

	print "\t\tRHeader & get(HeaderType t, " a ");" >> o
	
	for (a in defs) {
		print "\t\tvoid set(HeaderType t, const " a " & data);" >> o
	}
	
	print >> o
	print "};" >> o
	print >> o
	print "#endif // RMM_ENVELOPE_H" >> o
}

END {

	for (a in defs) {
		mkh(a)
		mkcpp(a)
	}
	# Do the cpp file
}

