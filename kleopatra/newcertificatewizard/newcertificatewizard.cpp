#include "newcertificatewizard.h"

#include "ui_chooseprotocolpage.h"
#include "ui_enterdetailspage.h"
#include "ui_overviewpage.h"
#include "ui_resultpage.h"

using namespace Kleo;
using namespace Kleo::NewCertificateUi;

namespace {

    class ChooseProtocolPage : public QWizardPage {
        Q_OBJECT
    public:
        explicit ChooseProtocolPage( QWidget * p=0 )
            : QWizardPage( p ), ui()
        {
            ui.setupUi( this );
            registerField( "pgp", ui.pgpCLB );
            registerField( "x509", ui.x509CLB );
        }

    private Q_SLOTS:
        void slotPgpClicked() {
            done( true );
        }

        void slotX509Clicked() {
            done( false );
        }

        void done( bool pgp ) {
            QMetaObject::invokeMethod( wizard(), "next", Qt::QueuedConnection );
        }

    private:
        Ui_ChooseProtocolPage ui;
    };

    class EnterDetailsPage : public QWizardPage {
        Q_OBJECT
    public:
        explicit EnterDetailsPage( QWidget * p=0 )
            : QWizardPage( p ), ui()
        {
            ui.setupUi( this );
        }

    private:
        Ui_EnterDetailsPage ui;
    };

    class OverviewPage : public QWizardPage {
        Q_OBJECT
    public:
        explicit OverviewPage( QWidget * p=0 )
            : QWizardPage( p ), ui()
        {

        }

    private:
        Ui_OverviewPage ui;
    };

    class ResultPage : public QWizardPage {
        Q_OBJECT
    public:
        explicit ResultPage( QWidget * p=0 )
            : QWizardPage( p ), ui()
        {

        }

    private:
        Ui_ResultPage ui;
    };
}

class NewCertificateWizard::Private {
    friend class ::Kleo::NewCertificateWizard;
    NewCertificateWizard * const q;
public:
    explicit Private( NewCertificateWizard * qq )
        : q( qq ),
          ui( q )
    {

    }

private:
    struct Ui {
        ChooseProtocolPage chooseProtocolPage;
        EnterDetailsPage enterDetailsPage;
        OverviewPage overviewPage;
        ResultPage resultPage;

        explicit Ui( NewCertificateWizard * q )
            : chooseProtocolPage( q ),
              enterDetailsPage( q ),
              overviewPage( q ),
              resultPage( q )
        {
            q->setPage( ChooseProtocolPageId, &chooseProtocolPage );
            q->setPage( EnterDetailsPageId,   &enterDetailsPage   );
            q->setPage( OverviewPageId,       &overviewPage       );
            q->setPage( ResultPageId,         &resultPage         );

            q->setStartId( ChooseProtocolPageId );
        }

    } ui;

};

NewCertificateWizard::NewCertificateWizard( QWidget * p )
    : QWizard( p ), d( new Private( this ) )
{

}

NewCertificateWizard::~NewCertificateWizard() {}

#include "moc_newcertificatewizard.cpp"
#include "newcertificatewizard.moc"
