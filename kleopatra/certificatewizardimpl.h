#ifndef CERTIFICATEWIZARDIMPL_H
#define CERTIFICATEWIZARDIMPL_H
#include "certificatewizard.h"

#include <qcstring.h>

class CertificateWizardImpl : public CertificateWizard
{
    Q_OBJECT

public:
    CertificateWizardImpl( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CertificateWizardImpl();

    QByteArray keyData() const { return _keyData; }

protected slots:
    void slotGenerateCertificate();
    void slotEmailAddressChanged( const QString& text );

    void slotHelpClicked();

private:
    QByteArray _keyData;
};

#endif // CERTIFICATEWIZARDIMPL_H
