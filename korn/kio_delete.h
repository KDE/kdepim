#ifndef KIO_DELETE_H
#define KIO_DELETE_H

/*
 * This class handles removing of selected messages.
 * This class starts working when deleteMails() is called.
 */

#include <qobject.h>
class KKioDrop;
class KIO_Protocol;
class KornMailId;

class KURL;
namespace KIO { class MetaData; class Job; class Slave; }

template<class T> class QPtrList;

class KIO_Delete : public QObject
{ Q_OBJECT
public:
	//constructors
	KIO_Delete( QObject * parent = 0, const char * name = 0 );
	~KIO_Delete( );
	
	//This function should be called if there are messages to be deleted.
	bool deleteMails( QPtrList< const KornMailId > *, KKioDrop* );
	
	//This function should return false then and only then if an error occured.
	bool valid( ) { return _valid; }
	
public slots:
	//If this slot is called, the whole deletion is canceled.
	void canceled( );
private slots:
	void slotResult( KIO::Job* );
	
private:
	void disConnect( );
	bool setupSlave( KURL kurl, KIO::MetaData metadata, KIO_Protocol *& protocol );
	void deleteItem( const KornMailId *item, KURL, KIO::MetaData, KIO_Protocol *&);
	void commitDelete( KURL, KIO::MetaData, KIO_Protocol *& );

	KKioDrop *_kio;
	unsigned int _total;
	QPtrList< KIO::Job > *_jobs;
	KIO::Slave *_slave;
	bool _valid;
};

#endif
