#include "newcertificatewizard.h"

#include "ui_chooseprotocolpage.h"
#include "ui_enterdetailspage.h"
#include "ui_overviewpage.h"
#include "ui_resultpage.h"

#include <kleo/dn.h>

#include <KConfigGroup>
#include <KGlobal>
#include <KLocale>
#include <KDebug>

#include <QRegExpValidator>
#include <QLineEdit>

using namespace Kleo;
using namespace Kleo::NewCertificateUi;

namespace {

    class ChooseProtocolPage : public QWizardPage {
        Q_OBJECT
    public:
        explicit ChooseProtocolPage( QWidget * p=0 )
            : QWizardPage( p ),
              initialized( false ),
              ui()
        {
            ui.setupUi( this );

            connect( ui.pgpCLB,  SIGNAL(clicked()), wizard(), SLOT(next()), Qt::QueuedConnection );
            connect( ui.x509CLB, SIGNAL(clicked()), wizard(), SLOT(next()), Qt::QueuedConnection );

            registerField( "pgp", ui.pgpCLB );
        }

        /* reimp */ void initializePage() {
            if ( !initialized ) {
                connect( ui.pgpCLB,  SIGNAL(clicked()), wizard(), SLOT(next()), Qt::QueuedConnection );
                connect( ui.x509CLB, SIGNAL(clicked()), wizard(), SLOT(next()), Qt::QueuedConnection );
            }
            initialized = true;
        }

        /* reimp */ bool isComplete() const {
            return ui.pgpCLB->isChecked() || ui.x509CLB->isChecked() ;
        }

    private:
        bool initialized : 1;
        Ui_ChooseProtocolPage ui;
    };

    class EnterDetailsPage : public QWizardPage {
        Q_OBJECT
    public:
        explicit EnterDetailsPage( QWidget * p=0 )
            : QWizardPage( p ), ui()
        {
            ui.setupUi( this );
            updateForm();
        }

        /* reimp */ bool isComplete() const;
        /* reimp */ void initializePage() {
            updateForm();
        }
        /* reimp */ void cleanupPage() {
            savedValues.clear();
            for ( QVector< QPair<QString,QLineEdit*> >::const_iterator it = attributePairList.begin(), end = attributePairList.end() ; it != end ; ++it )
                savedValues[it->first] = it->second->text().trimmed();
        }

    private:
        void updateForm();
        void clearForm();

    private:
        QVector< QPair<QString,QLineEdit*> > attributePairList;
        QList<QWidget*> dynamicWidgets;
        QMap<QString,QString> savedValues;
        Ui_EnterDetailsPage ui;
    };

    class OverviewPage : public QWizardPage {
        Q_OBJECT
    public:
        explicit OverviewPage( QWidget * p=0 )
            : QWizardPage( p ), ui()
        {
            setCommitPage( true );
            setButtonText( QWizard::CommitButton, i18nc("@action", "Create") );
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

static QString pgpLabel( const QString & attr ) {
    if ( attr == "NAME" )
        return i18n("Name");
    if ( attr == "COMMENT" )
        return i18n("Comment");
    if ( attr == "EMAIL" )
        return i18n("EMail");
    return QString();
}

static QString attributeLabel( const QString & attr, bool required, bool pgp ) {
  if ( attr.isEmpty() )
    return QString();
  const QString label = pgp ? pgpLabel( attr ) : Kleo::DNAttributeMapper::instance()->name2label( attr ) ;
  if ( !label.isEmpty() )
      if ( pgp )
          return label + ':';
      else
          return i18nc("Format string for the labels in the \"Your Personal Data\" page",
                       "%1 (%2):", label, attr );
  else
    return attr + ':';
}

static QString attributeFromKey( QString key ) {
  return key.remove( '!' );
}

namespace {
    class LineEdit : public QWidget {
        Q_OBJECT
    public:
        explicit LineEdit( const QString & text, bool required, QWidget * parent )
            : QWidget( parent ),
              lineEdit( text, this ),
              label( required ? i18n("(required)") : i18n("(optional)"), this ),
              layout( this )
        {
            KDAB_SET_OBJECT_NAME( lineEdit );
            KDAB_SET_OBJECT_NAME( label );
            KDAB_SET_OBJECT_NAME( layout );

            layout.addWidget( &lineEdit );
            layout.addWidget( &label );
        }

    public:
        QLineEdit lineEdit;
        QLabel label;
        QHBoxLayout layout;
    };
}

void EnterDetailsPage::updateForm() {

    clearForm();

    const KConfigGroup config( KGlobal::config(), "CertificateCreationWizard" );

    const bool pgp = field("pgp").toBool();

    QStringList attrOrder = config.readEntry( pgp ? "OpenPGPAttributeOrder" : "DNAttributeOrder", QStringList() );
    if ( attrOrder.empty() )
        if ( pgp )
            attrOrder << "NAME!" << "EMAIL!" << "COMMENT";
        else
            attrOrder << "CN!" << "L" << "OU" << "O!" << "C!" << "EMAIL!";

    Q_FOREACH( const QString & rawKey, attrOrder ) {
        const QString key = rawKey.trimmed().toUpper();
        const QString attr = attributeFromKey( key );
        if ( attr.isEmpty() )
            continue;
        const QString preset = savedValues.value( key, config.readEntry( attr, QString() ) );
        const bool required = key.endsWith( QLatin1Char('!') );
        const QString label = config.readEntry( attr + "_label",
                                                attributeLabel( attr, required, pgp ) );

        LineEdit * const le = new LineEdit( preset, required, ui.formLayout->parentWidget() );
        ui.formLayout->addRow( label, le );

        if ( config.isEntryImmutable( attr ) )
            le->lineEdit.setEnabled( false );

        QString regex = config.readEntry( attr + "_regex" );
        if ( regex.isEmpty() )
            regex = "[^\\s].*"; // !empty
        le->lineEdit.setValidator( new QRegExpValidator( QRegExp( regex ), le ) );

        attributePairList.append( qMakePair(key, &le->lineEdit) );

        connect( &le->lineEdit, SIGNAL(textChanged(QString)),
                 SIGNAL(completeChanged()) );

        dynamicWidgets.push_back( ui.formLayout->labelForField( le ) );
        dynamicWidgets.push_back( le );
    }
}

void EnterDetailsPage::clearForm() {
    qDeleteAll( dynamicWidgets );
    dynamicWidgets.clear();
    attributePairList.clear();
}

static bool requirementsAreMet( const QVector< QPair<QString,QLineEdit*> > & list ) {
    for ( QVector< QPair<QString,QLineEdit*> >::const_iterator it = list.begin() ;
          it != list.end() ; ++it ) {
    const QLineEdit * le = (*it).second;
    if ( !le )
      continue;
    const QString key = (*it).first;
    kDebug() << "requirementsAreMet(): checking \"" << key << "\" against \"" << le->text() << "\":";
    if ( key.endsWith('!') && !le->hasAcceptableInput() ) {
      kDebug() << "required field has non-acceptable input!";
      return false;
    }
    kDebug() << "ok" << endl;
  }
  return true;
}

namespace {
    bool EnterDetailsPage::isComplete() const {
        return requirementsAreMet( attributePairList );
    }
}

#include "moc_newcertificatewizard.cpp"
#include "newcertificatewizard.moc"
