#ifndef __KIO_MOBILE_H__
#define __KIO_MOBILE_H__

#include <qstring.h>
#include <qvaluelist.h>

#include <kio/slavebase.h>
#include <kmobiledevice.h>
#include <kmobileclient.h>

class KMobileProtocol : public KIO::SlaveBase
{
public:
  KMobileProtocol( const QCString &pool, const QCString &app );
  ~KMobileProtocol();

  void get( const KURL& url );
  void stat( const KURL& url );
  void mimetype( const KURL& url );
  void listDir( const KURL& url );

protected:
  int  getDeviceAndRessource(const QString &_path,
        QString &devName, QString &resource, QString &devPath,
        KMobileDevice::Capabilities &devCaps);

  QString folderMimeType(int cap);
  QString entryMimeType(int cap);

  void listRoot();
  void listTopDeviceDir(const QString &devName);
  void listEntries(const QString &devName,
	const QString &resource, const QString &devPath,
	const KMobileDevice::Capabilities devCaps);

  void listAddressBook(const QString &devName, const QString &resource);
  int  getVCard( const QString &devName, QCString &result, QString &mime, const QString &path );

  void listCalendar(const QString &devName, const QString &resource);
  int  getCalendar( const QString &devName, QCString &result, QString &mime, const QString &path );

  void listNotes(const QString &devName, const QString &resource);
  int  getNote( const QString &devName, QCString &result, QString &mime, const QString &path );

  void listFileStorage(const QString &devName, const QString &resource, const QString &devPath);
  int  getFileStorage( const QString &devName, QCString &result, QString &mime, const QString &path );

private:
  KMobileClient m_dev;
};

#endif
