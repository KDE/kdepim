#ifndef _CERTMANAGER_H_
#define _CERTMANAGER_H_

#include <kmainwindow.h>
#include <cryptplugwrapper.h>
class CertBox;
class KProcess;
class CertItem;
class KToolBar;
class KAction;

class LineEditAction;
class ComboAction;

class CertManager :public KMainWindow
{
  Q_OBJECT

public:
    CertManager( bool remote = false, const QString& query = QString::null, 
		 QWidget* parent = 0, const char* name = 0);

    const CryptPlugWrapper::CertificateInfoList& certList() const { return _certList; }

    bool isRemote() const { return _remote; }

    int importCertificateWithFingerprint( const QString& fingerprint, QString* info );
    int importCertificateFromFile( const QString& filename, QString* info );

    bool haveCertificate( const QString &fingerprint );

protected slots:
    void loadCertificates();
    void newCertificate();
    void quit();
    void revokeCertificate();
    void extendCertificate();

    void importCertFromFile();
    void importCRLFromFile();
    void importCRLFromLDAP();

    void slotDirmngrExited();
    void slotStderr( KProcess*, char*, int );

    void slotToggleRemote(int idx);


private:
    bool checkExec( const QStringList& args );

    CertItem* fillInOneItem( CertBox* lv, CertItem* parent, 
			     const CryptPlugWrapper::CertificateInfo& info );

    CryptPlugWrapper::CertificateInfoList _certList;

  //KProcess* gpgsmProc;
    KProcess* dirmngrProc;
    QString errorbuffer;


    KToolBar* _toolbar;
    LineEditAction* _leAction;
    ComboAction* _comboAction;
    KAction* _findAction;

    CertBox* _certBox;
    bool     _remote;
};

#endif // _CERTMANAGER_H_
