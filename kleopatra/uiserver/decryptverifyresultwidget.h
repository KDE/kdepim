#ifndef __KLEOPATRA_UISERVER_DECRYPTVERIFYRESULTWIDGET_H__
#define __KLEOPATRA_UISERVER_DECRYPTVERIFYRESULTWIDGET_H__

#include <uiserver/resultdisplaywidget.h>

#include <vector>

namespace GpgME {
    class DecryptionResult;
    class VerificationResult;
    class Key;
}

class QVBoxLayout;

namespace Kleo {

    class DecryptVerifyResultWidget : public ResultDisplayWidget {
        Q_OBJECT
    public:
        explicit DecryptVerifyResultWidget( QWidget * parent );
        ~DecryptVerifyResultWidget();

        QString formatDecryptionResult( const GpgME::DecryptionResult & res, const std::vector<GpgME::Key> & recipients );
        QString formatVerificationResult( const GpgME::VerificationResult & res ) const;

        void setResult( const GpgME::DecryptionResult & decryptionResult, const GpgME::VerificationResult & verificationResult );
    private:
        QVBoxLayout * m_box;
    };

}

#endif /* __KLEOPATRA_UISERVER_DECRYPTVERIFYRESULTWIDGET_H__ */
