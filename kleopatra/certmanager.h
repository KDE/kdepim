#ifndef _CERTMANAGER_H_
#define _CERTMANAGER_H_

#include <kmainwindow.h>
#include <cryptplugwrapper.h>
class CertBox;
class KProcess;
class CertItem;
class KToolBar;

class CertManager :public KMainWindow
{
Q_OBJECT

public:
    CertManager( QWidget* parent = 0, const char* name = 0);

    const CryptPlugWrapper::CertificateInfoList& certList() const { return _certList; }
    
protected:

protected slots:
    void loadCertificates();
    void newCertificate();
    void quit();
    void revokeCertificate();
    void extendCertificate();
    void importCertFromFile();
    void importCRLFromFile();
    void importCRLFromLDAP();
    void slotGPGSMExited();

private:
  CertItem* fillInOneItem( CertBox* lv, CertItem* parent, 
			   const CryptPlugWrapper::CertificateInfo& info );

    CryptPlugWrapper::CertificateInfoList _certList;
    KProcess* gpgsmProc;
    KToolBar* _toolbar;
    CertBox* _certBox;
};

#endif // _CERTMANAGER_H_
