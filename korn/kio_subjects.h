#ifndef MK_KIO_SUBJECTS_H
#define MK_KIO_SUBJECTS_H

//This class calls other class to read all the subjects

#include <qobject.h>
class KKioDrop;
class KIO_Single_Subject;
class KIO_Protocol;
class KornMailSubject;

namespace KIO { class MetaData; class Slave; }
class KURL;

template<class T> class QPtrList;
class QString;
template<class T> class QValueList;

class KIO_Subjects : public QObject
{ Q_OBJECT
public:
	KIO_Subjects( QObject * parent, const char * name );
	~KIO_Subjects( );
	
	//This function let it start fetching headers.
	void doReadSubjects( KKioDrop* );
	
	//This function should return true then and only then of no error occured.
	bool valid( ) { return _valid; }
	
private:
	KKioDrop *_kio;
	KURL *_kurl;
	KIO::MetaData *_metadata;
	KIO_Protocol *_protocol;
	QPtrList<KIO_Single_Subject> *_jobs;
	KIO::Slave *_slave;
	bool _valid;
	
	//Opens a connection.
	void getConnection( );
	//Start a job; the job itself is executed in KIO_Single_Subject
	void startJob( const QString&, const long );
	//Disconnect the connection
	void disConnect( bool );
	
public slots:
	//This function called the fetching of headers.
	void cancelled( );
	
private slots:
	void slotReadSubject( KornMailSubject* );
	void slotFinished( KIO_Single_Subject* );
};

#endif
