#ifndef KIO_READ_H
#define KIO_READ_H

//This class should be used if someone wants to read the Full Message

#include <qobject.h>

class KKioDrop;
class KornMailId;

class KURL;
namespace KIO { class MetaData; class Job; }
class KIO_Protocol;

class QString;

class KIO_Read : public QObject
{ Q_OBJECT
public:
	KIO_Read( QObject * parent = 0, const char * name = 0 );
	~KIO_Read();

public slots:
	//This is the function which makes the nessesairy slaves for reading a message
	void readMail( const KornMailId *&, KKioDrop* );
	//This function should be called if the user presses canceled.
	void canceled();
private:
	KKioDrop *_kio;
	KIO::Job *_job;
	QString *_message;
	
signals:
	//This signal is emitted when the whole message is read; the message got passed as QString*
	void ready( QString* );
	
private slots:
	void slotResult( KIO::Job* );
	void slotData( KIO::Job*, const QByteArray& );
};

#endif
