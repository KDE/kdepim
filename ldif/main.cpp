#include <iostream.h>
#include <qfile.h>
#include <qtextstream.h>

#include "ldif.h"

main(int argc, char * argv[])
{
	if (argc != 2) {
		cerr << "Usage: " << argv[0] << " <filename>" << endl;
		exit(1);
	}
	
	QFile f(argv[1]);
	
	QCString str;
	
	if (!f.open(IO_ReadOnly)) {
		cerr << "Couldn't open file \"" << argv[1] << endl;
		exit(1);
	}
	
	QTextStream t(&f);
	
	while (!t.eof())
		str += t.readLine() + '\n';
	
	using namespace LDIF; 
	
	LdifContent ldc(str);
	
	// Force a parse
	ldc.parse();
	
	cerr << ldc.asString();
}

