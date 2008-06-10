/* -*- mode: c++; c-basic-offset:4 -*-
    newcertificatewizard/newcertificatewizard.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "newcertificatewizard.h"

#include "ui_chooseprotocolpage.h"
#include "ui_enterdetailspage.h"
#include "ui_overviewpage.h"
#include "ui_resultpage.h"

#include "ui_advancedsettingsdialog.h"

#include <kleo/dn.h>

#include <gpgme++/global.h>

#include <KConfigGroup>
#include <KGlobal>
#include <KLocale>
#include <KDebug>

#include <QRegExpValidator>
#include <QLineEdit>

#include <boost/range.hpp>

#include <algorithm>

using namespace Kleo;
using namespace Kleo::NewCertificateUi;
using namespace GpgME;
using namespace boost;

static const unsigned int key_strengths[] = {
    1024, 1532, 2048, 3072, 4096,
};
static const unsigned int num_key_strengths = sizeof key_strengths / sizeof *key_strengths;

static unsigned int index2strength( unsigned int index ) {
    if ( index < num_key_strengths )
        return key_strengths[index];
    else
        return 0;
}

static int strength2index( unsigned int strength ) {
    const unsigned int * const it = 
        std::lower_bound( begin( key_strengths ), end( key_strengths ), strength );
    if ( it == end( key_strengths ) )
        return key_strengths[num_key_strengths-1];
    else
        return *it;
}

namespace {

    class AdvancedSettingsDialog : public QDialog {
        Q_OBJECT
        Q_ENUMS( PublicKeyAlgorithm )
        Q_PROPERTY( QStringList userIDs READ userIDs WRITE setUserIDs )
        Q_PROPERTY( QStringList emailAddresses READ emailAddresses WRITE setEmailAddresses )
        Q_PROPERTY( QStringList dnsNames READ dnsNames WRITE setDnsNames )
        Q_PROPERTY( QStringList uris READ uris WRITE setUris )
        Q_PROPERTY( uint keyStrength READ keyStrength WRITE setKeyStrength )
        Q_PROPERTY( PublicKeyAlgorithm publicKeyAlgorithm READ publicKeyAlgorithm WRITE setPublicKeyAlgorithm )
        Q_PROPERTY( bool signingAllowed READ signingAllowed WRITE setSigningAllowed )
        Q_PROPERTY( bool encryptionAllowed READ encryptionAllowed WRITE setEncryptionAllowed )
        Q_PROPERTY( bool certificationAllowed READ certificationAllowed WRITE setCertificationAllowed )
        Q_PROPERTY( bool authenticationAllowed READ authenticationAllowed WRITE setAuthenticationAllowed )
        Q_PROPERTY( QDate expiryDate READ expiryDate WRITE setExpiryDate )
    public:
        explicit AdvancedSettingsDialog( QWidget * parent=0 )
            : QDialog( parent ), ui()
        {
            ui.setupUi( this );
            const QDate today = QDate::currentDate();
            ui.expiryDE->setMinimumDate( today );
            ui.expiryDE->setDate( today.addYears( 2 ) );
        }

        void setProtocol( GpgME::Protocol protocol ) {
            ui.uidGB->setVisible( protocol == OpenPGP );
            ui.emailGB->setVisible( protocol == CMS );
            ui.dnsGB->setVisible( protocol == CMS );
            ui.uriGB->setVisible( protocol == CMS );
            if ( protocol == OpenPGP ) {
                ui.signingCB->setChecked( true );
                ui.signingCB->setEnabled( false );
                ui.certificationCB->setChecked( true );
                ui.certificationCB->setEnabled( false );
                ui.authenticationCB->setChecked( false );
                ui.authenticationCB->setEnabled( false );
            } else {
                ui.signingCB->setEnabled( true );
                ui.certificationCB->setEnabled( true );
                ui.authenticationCB->setEnabled( true );
            }
        }
        
        void setUserIDs( const QStringList & items ) { ui.uidLW->setItems( items ); }
        QStringList userIDs() const { return ui.uidLW->items();   }

        void setEmailAddresses( const QStringList & items ) { ui.emailLW->setItems( items ); }
        QStringList emailAddresses() const { return ui.emailLW->items(); }

        void setDnsNames( const QStringList & items ) { ui.dnsLW->setItems( items ); }
        QStringList dnsNames() const { return ui.dnsLW->items();   }

        void setUris( const QStringList & items ) { ui.uriLW->setItems( items ); }
        QStringList uris() const { return ui.uriLW->items();   }


        void setKeyStrength( unsigned int strength ) {
            ui.rsaKeyStrengthCB->setCurrentIndex( strength2index( strength ) );
        }
        unsigned int keyStrength() const {
            return ui.dsaRB->isChecked() ? 1024U : index2strength( ui.rsaKeyStrengthCB->currentIndex() );
        }

        enum PublicKeyAlgorithm {
            RSA, DSA,
            NumPublicKeyAlgorithms
        };
        void setPublicKeyAlgorithm( PublicKeyAlgorithm algo ) {
            ( algo == DSA ? ui.dsaRB : ui.rsaRB )->setChecked( true );
        }
        PublicKeyAlgorithm publicKeyAlgorithm() const { return ui.dsaRB->isChecked() ? DSA : RSA ; }


        void setSigningAllowed( bool on ) { ui.signingCB->setChecked( on ); }
        bool signingAllowed() const { return ui.signingCB->isChecked(); }

        void setEncryptionAllowed( bool on ) { ui.encryptionCB->setChecked( on ); }
        bool encryptionAllowed() const { return ui.encryptionCB->isChecked(); }

        void setCertificationAllowed( bool on ) { ui.certificationCB->setChecked( on ); }
        bool certificationAllowed() const { return ui.certificationCB->isChecked(); }

        void setAuthenticationAllowed( bool on ) { ui.authenticationCB->setChecked( on ); }
        bool authenticationAllowed() const { return ui.authenticationCB->isChecked(); }

        void setExpiryDate( const QDate & date ) { ui.expiryDE->setDate( date ); }
        QDate expiryDate() const { return ui.expiryDE->date(); }

    private:
        Ui_AdvancedSettingsDialog ui;
    };

    class ChooseProtocolPage : public QWizardPage {
        Q_OBJECT
    public:
        explicit ChooseProtocolPage( QWidget * p=0 )
            : QWizardPage( p ),
              initialized( false ),
              ui()
        {
            ui.setupUi( this );
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
            saveValues();
        }

    private:
        void updateForm();
        void clearForm();
        void saveValues();

    private Q_SLOTS:
        void slotAdvancedSettingsClicked();

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

void EnterDetailsPage::saveValues() {
    for ( QVector< QPair<QString,QLineEdit*> >::const_iterator it = attributePairList.begin(), end = attributePairList.end() ; it != end ; ++it )
        savedValues[ attributeFromKey(it->first) ] = it->second->text().trimmed();
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
        const QString preset = savedValues.value( attr, config.readEntry( attr, QString() ) );
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

bool EnterDetailsPage::isComplete() const {
    return requirementsAreMet( attributePairList );
}

void EnterDetailsPage::slotAdvancedSettingsClicked() {
    AdvancedSettingsDialog dialog;
    dialog.setProtocol( field("pgp").toBool() ? OpenPGP : CMS );
    dialog.exec();
}

#include "moc_newcertificatewizard.cpp"
#include "newcertificatewizard.moc"
