#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <qregexp.h>
#include <qstrlist.h>
#include <qfile.h>
#include <qdatetime.h>
#include <RMM.h>

int process (const char * filename);

main(int argc, char *argv[])
{
	QStrList files;
	QFile filelist("fileList");
	if (!filelist.open(IO_ReadOnly)) {
	   cerr << "Couldn't open file list" << endl;
	   abort();
	}
	char b[1024];
	while (!filelist.atEnd()) {
		filelist.readLine(b, 1024);
		files.append(b);
	}
	filelist.close();
	
	QTime startT = QTime::currentTime();
	int count = 0;
	int tottime = 0;
	QStrListIterator it(files);
	for (; it.current(); ++it, count++)
		tottime += process("/home/rik/Maildir/new/" + QString(it.current()));

	cerr << count << " messages, total time in processing = " << tottime/1000.0 << "s, average time " << tottime / (float)count << "ms" << endl;
	QTime endT = QTime::currentTime();
	cerr << "Total time taken = " << startT.secsTo(endT) << "s" << endl;
	exit(0);
}
	
int process (const char * filename) {
	
	QString fn(filename);
	fn = fn.stripWhiteSpace();
	
	QFile f(fn);
	
	if (!f.exists()) {
		cerr << "File does not exist: \"" << fn << "\"" << endl;
		abort();
	}

	if (!f.open(IO_ReadOnly)) {
		cerr << "Couldn't open \"" << fn << "\"" << endl;
		abort();
	}
	
	cerr << fn << endl;

	/*
	QString msgBuf;

	char buf[100024];

	while (!f.atEnd()) {
		f.readLine(buf, 100024);
		msgBuf += buf;
	}
	f.close();
*/
	
	QTime t = QTime::currentTime();
	char * start;
	size_t length = f.size();
	int prot = PROT_READ;
	int flags = MAP_PRIVATE;
	int fd = f.handle();
	off_t ot = 0;
	start = (char *)mmap(0, length, prot, flags, fd, ot);
	char mapbuf[length];
	strncpy(mapbuf, start, length);
	RMessage m(mapbuf);
	m.parse();
	m.assemble();
	munmap(start, length);
	QTime e = QTime::currentTime();
	int retval =  t.msecsTo(e);
//	QString output = m.asString();
//	cerr << "-------------------xxxxxxxxxxxx----------------------" << endl;
//	cerr << output << endl;
//	cerr << "-------------------xxxxxxxxxxxx----------------------" << endl;
//	toLF(output);
/*
	QString outFileName = fn;
	outFileName += ".out";
	QFile f2(outFileName);
	if (!f2.open(IO_WriteOnly)) abort();
	
	Q_UINT32 l = output.length();

	for (Q_UINT32 c = 0; c < l; c++) {
		f2.putch((int)output.at(c));
	}

	f2.close();
	*/
	return retval;
}

/*	
	QString s = "Hello this is a new body part";
	RBodyPart * p = new RBodyPart(s);
	p->setMimeType(MimeTypeText);
	p->setMimeSubType(MimeSubTypePlain);
	m.addPart(p);
	m.assemble();
	cerr << "---- Message as string ----" << endl << m.asString() << endl <<
			"---------------------------" << endl;
*/

