#ifndef _CERTMANAGER_H_
#define _CERTMANAGER_H_

#include <kmainwindow.h>
class CertBox;
class KProcess;
class CryptPlugWrapper;

class CertManager :public KMainWindow
{
Q_OBJECT

public:
    CertManager( QWidget* parent = 0, const char* name = 0);

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
  //CryptPlugWrapper* _wrapper;
    KProcess* gpgsmProc;
    CertBox* _certBox;
};

#endif // _CERTMANAGER_H_
