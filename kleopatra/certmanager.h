#ifndef _CERTMANAGER_H_
#define _CERTMANAGER_H_

#include <kmainwindow.h>
class CertBox;
class KProcess;

class CertManager :public KMainWindow
{
Q_OBJECT

public:
  CertManager( QWidget* parent, const char* name = 0);

protected:
  void loadCertificates();

protected slots:
    void newCertificate();
    void quit();
    void revokeCertificate();
    void extendCertificate();
    void importCertFromFile();
    void importCRLFromFile();
    void importCRLFromLDAP();
    void slotGPGSMExited();

private:
    KProcess* gpgsmProc;
    CertBox* _certBox;
};

#endif // _CERTMANAGER_H_
