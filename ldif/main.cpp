#include <iostream.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdatetime.h>

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
	
	QTime start(QTime::currentTime());
	LdifContent ldc(str);
	ldc.parse();
	while(true) {}
}

