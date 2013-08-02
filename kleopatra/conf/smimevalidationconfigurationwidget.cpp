/* -*- mode: c++; c-basic-offset:4 -*-
    conf/smimevalidationconfigurationwidget.cpp

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

#include "smimevalidationconfigurationwidget.h"

#include "ui_smimevalidationconfigurationwidget.h"

#include "smimevalidationpreferences.h"

#include <kleo/cryptoconfig.h>
#include <kleo/cryptobackendfactory.h>

#include <kdebug.h>

#include <QDBusConnection>

using namespace Kleo;
using namespace Kleo::Config;

class SMimeValidationConfigurationWidget::Private {
    friend class ::Kleo::Config::SMimeValidationConfigurationWidget;
    SMimeValidationConfigurationWidget * const q;
public:
    explicit Private( SMimeValidationConfigurationWidget * qq )
        : q( qq ),
          customHTTPProxyWritable( false ),
          ui( q )
    {
        QDBusConnection::sessionBus().connect(QString(), QString(), QLatin1String("org.kde.kleo.CryptoConfig"), QLatin1String("changed"), q, SLOT(load()) );
    }

   bool customHTTPProxyWritable;

private:
    void enableDisableActions() {
        ui.customHTTPProxy->setEnabled( ui.useCustomHTTPProxyRB->isChecked() &&
                                        !ui.disableHTTPCB->isChecked() && 
                                        customHTTPProxyWritable );
    }

private:
    struct UI : Ui_SMimeValidationConfigurationWidget {
        explicit UI( SMimeValidationConfigurationWidget * q )
            : Ui_SMimeValidationConfigurationWidget()
        {
            setupUi( q );

            if ( QLayout * l = q->layout() )
                l->setMargin( 0 );

            const struct {
                QObject * object;
                const char * signal;
            } sources[] = {
                { intervalRefreshCB, SIGNAL(toggled(bool)) },
                { intervalRefreshSB, SIGNAL(valueChanged(int)) },
                { CRLRB, SIGNAL(toggled(bool)) },
                { OCSPRB, SIGNAL(toggled(bool)) },
                { OCSPResponderURL, SIGNAL(textChanged(QString)) },
                { OCSPResponderSignature, SIGNAL(selectedCertificatesChanged(QStringList)) },
                { doNotCheckCertPolicyCB, SIGNAL(toggled(bool)) },
                { neverConsultCB, SIGNAL(toggled(bool)) },
                { allowMarkTrustedCB, SIGNAL(toggled(bool)) },
                { fetchMissingCB, SIGNAL(toggled(bool)) },
                { ignoreServiceURLCB, SIGNAL(toggled(bool)) },
                { ignoreHTTPDPCB, SIGNAL(toggled(bool)) },
                { disableHTTPCB, SIGNAL(toggled(bool)) },
                { honorHTTPProxyRB, SIGNAL(toggled(bool)) },
                { useCustomHTTPProxyRB, SIGNAL(toggled(bool)) },
                { customHTTPProxy, SIGNAL(textChanged(QString)) },
                { ignoreLDAPDPCB, SIGNAL(toggled(bool)) },
                { disableLDAPCB, SIGNAL(toggled(bool)) },
                { customLDAPProxy, SIGNAL(textChanged(QString)) },
            };
            for ( unsigned int i = 0 ; i < sizeof sources / sizeof *sources ; ++i )
                connect( sources[i].object, sources[i].signal, q, SIGNAL(changed()) );
            connect( useCustomHTTPProxyRB, SIGNAL(toggled(bool)),
                     q, SLOT(enableDisableActions()) );
            connect( disableHTTPCB, SIGNAL(toggled(bool)),
                     q, SLOT(enableDisableActions()) );

            OCSPResponderSignature->setOnlyX509CertificatesAllowed( true );
            OCSPResponderSignature->setOnlySigningCertificatesAllowed( true );
            OCSPResponderSignature->setMultipleCertificatesAllowed( false );
            //OCSPResponderSignature->setAllowedKeys( KeySelectionDialog::TrustedKeys|KeySelectionDialog::ValidKeys );
        }
    } ui;
};

SMimeValidationConfigurationWidget::SMimeValidationConfigurationWidget( QWidget * p, Qt::WindowFlags f )
    : QWidget( p, f ), d( new Private( this ) )
{

}

SMimeValidationConfigurationWidget::~SMimeValidationConfigurationWidget() {}


static void disableDirmngrWidget( QWidget* w ) {
    w->setEnabled( false );
    w->setWhatsThis( i18n( "This option requires dirmngr >= 0.9.0" ) );
}

static void initializeDirmngrCheckbox( QCheckBox* cb, CryptoConfigEntry* entry ) {
    if ( entry )
        cb->setChecked( entry->boolValue() );
    if ( !entry || entry->isReadOnly() )
        disableDirmngrWidget( cb );
}

struct SMIMECryptoConfigEntries {
    SMIMECryptoConfigEntries( CryptoConfig * config )
        : mConfig( config ),
          // Checkboxes
          mCheckUsingOCSPConfigEntry( configEntry( "gpgsm", "Security", "enable-ocsp", CryptoConfigEntry::ArgType_None, false ) ),
          mEnableOCSPsendingConfigEntry( configEntry( "dirmngr", "OCSP", "allow-ocsp", CryptoConfigEntry::ArgType_None, false ) ),
          mDoNotCheckCertPolicyConfigEntry( configEntry( "gpgsm", "Security", "disable-policy-checks", CryptoConfigEntry::ArgType_None, false ) ),
          mNeverConsultConfigEntry( configEntry( "gpgsm", "Security", "disable-crl-checks", CryptoConfigEntry::ArgType_None, false ) ),
          mAllowMarkTrustedConfigEntry( configEntry( "gpg-agent", "Security", "allow-mark-trusted", CryptoConfigEntry::ArgType_None, false ) ),
          mFetchMissingConfigEntry( configEntry( "gpgsm", "Security", "auto-issuer-key-retrieve", CryptoConfigEntry::ArgType_None, false ) ),
          mNoAllowMarkTrustedConfigEntry( configEntry( "gpg-agent", "Security", "no-allow-mark-trusted", CryptoConfigEntry::ArgType_None, false ) ),
          // dirmngr-0.9.0 options
          mIgnoreServiceURLEntry( configEntry( "dirmngr", "OCSP", "ignore-ocsp-service-url", CryptoConfigEntry::ArgType_None, false ) ),
          mIgnoreHTTPDPEntry( configEntry( "dirmngr", "HTTP", "ignore-http-dp", CryptoConfigEntry::ArgType_None, false ) ),
          mDisableHTTPEntry( configEntry( "dirmngr", "HTTP", "disable-http", CryptoConfigEntry::ArgType_None, false ) ),
          mHonorHTTPProxy( configEntry( "dirmngr", "HTTP", "honor-http-proxy", CryptoConfigEntry::ArgType_None, false ) ),
          mIgnoreLDAPDPEntry( configEntry( "dirmngr", "LDAP", "ignore-ldap-dp", CryptoConfigEntry::ArgType_None, false ) ),
          mDisableLDAPEntry( configEntry( "dirmngr", "LDAP", "disable-ldap", CryptoConfigEntry::ArgType_None, false ) ),
          // Other widgets
          mOCSPResponderURLConfigEntry( configEntry( "dirmngr", "OCSP", "ocsp-responder", CryptoConfigEntry::ArgType_String, false ) ),
          mOCSPResponderSignature( configEntry( "dirmngr", "OCSP", "ocsp-signer", CryptoConfigEntry::ArgType_String, false ) ),
          mCustomHTTPProxy( configEntry( "dirmngr", "HTTP", "http-proxy", CryptoConfigEntry::ArgType_String, false ) ),
          mCustomLDAPProxy( configEntry( "dirmngr", "LDAP", "ldap-proxy", CryptoConfigEntry::ArgType_String, false ) )
    {

    }

    CryptoConfigEntry* configEntry( const char* componentName,
                                    const char* groupName,
                                    const char* entryName,
                                    int argType,
                                    bool isList );

    CryptoConfig * const mConfig;

    // Checkboxes
    CryptoConfigEntry * const mCheckUsingOCSPConfigEntry;
    CryptoConfigEntry * const mEnableOCSPsendingConfigEntry;
    CryptoConfigEntry * const mDoNotCheckCertPolicyConfigEntry;
    CryptoConfigEntry * const mNeverConsultConfigEntry;
    CryptoConfigEntry * const mAllowMarkTrustedConfigEntry;
    CryptoConfigEntry * const mFetchMissingConfigEntry;
    // gnupg 2.0.17+ option that should inhibit allow-mark-trusted display
    CryptoConfigEntry * const mNoAllowMarkTrustedConfigEntry;
    // dirmngr-0.9.0 options
    CryptoConfigEntry * const mIgnoreServiceURLEntry;
    CryptoConfigEntry * const mIgnoreHTTPDPEntry;
    CryptoConfigEntry * const mDisableHTTPEntry;
    CryptoConfigEntry * const mHonorHTTPProxy;
    CryptoConfigEntry * const mIgnoreLDAPDPEntry;
    CryptoConfigEntry * const mDisableLDAPEntry;
    // Other widgets
    CryptoConfigEntry * const mOCSPResponderURLConfigEntry;
    CryptoConfigEntry * const mOCSPResponderSignature;
    CryptoConfigEntry * const mCustomHTTPProxy;
    CryptoConfigEntry * const mCustomLDAPProxy;

};

void SMimeValidationConfigurationWidget::defaults() {
    kDebug() << "not implemented";
}

void SMimeValidationConfigurationWidget::load() {
    const SMimeValidationPreferences preferences;
    const unsigned int refreshInterval = preferences.refreshInterval();
    d->ui.intervalRefreshCB->setChecked( refreshInterval > 0 );
    d->ui.intervalRefreshSB->setValue( refreshInterval );

    CryptoConfig * const config = CryptoBackendFactory::instance()->config();
    if ( !config ) {
        setEnabled( false );
        return;
    }

#if 0
    // crashes other pages' save() by nuking the CryptoConfigEntries under their feet.
    // This was probably not a problem in KMail, where this code comes
    // from. But here, it's fatal.

    // Force re-parsing gpgconf data, in case e.g. kleopatra or "configure backend" was used
    // (which ends up calling us via D-Bus)
    config->clear();
#endif

    // Create config entries
    // Don't keep them around, they'll get deleted by clear(), which could be
    // done by the "configure backend" button even before we save().
    const SMIMECryptoConfigEntries e( config );

    // Initialize GUI items from the config entries

    if ( e.mCheckUsingOCSPConfigEntry ) {
        const bool b = e.mCheckUsingOCSPConfigEntry->boolValue();
        d->ui.OCSPRB->setChecked( b );
        d->ui.CRLRB->setChecked( !b );
        d->ui.OCSPGroupBox->setEnabled( b );
    } else {
        d->ui.OCSPGroupBox->setEnabled( false );
    }


    if ( e.mDoNotCheckCertPolicyConfigEntry )
        d->ui.doNotCheckCertPolicyCB->setChecked( e.mDoNotCheckCertPolicyConfigEntry->boolValue() );
    if ( e.mNeverConsultConfigEntry )
        d->ui.neverConsultCB->setChecked( e.mNeverConsultConfigEntry->boolValue() );
    if ( e.mNoAllowMarkTrustedConfigEntry )
        d->ui.allowMarkTrustedCB->hide(); // this option was only here to _enable_ allow-mark-trusted, and makes no sense if it's already default on
    if ( e.mAllowMarkTrustedConfigEntry )
        d->ui.allowMarkTrustedCB->setChecked( e.mAllowMarkTrustedConfigEntry->boolValue() );
    if ( e.mFetchMissingConfigEntry )
        d->ui.fetchMissingCB->setChecked( e.mFetchMissingConfigEntry->boolValue() );

    if ( e.mOCSPResponderURLConfigEntry )
        d->ui.OCSPResponderURL->setText( e.mOCSPResponderURLConfigEntry->stringValue() );
    if ( e.mOCSPResponderSignature )
        d->ui.OCSPResponderSignature->setSelectedCertificate( e.mOCSPResponderSignature->stringValue() );

    // dirmngr-0.9.0 options
    initializeDirmngrCheckbox( d->ui.ignoreServiceURLCB, e.mIgnoreServiceURLEntry );
    initializeDirmngrCheckbox( d->ui.ignoreHTTPDPCB, e.mIgnoreHTTPDPEntry );
    initializeDirmngrCheckbox( d->ui.disableHTTPCB, e.mDisableHTTPEntry );
    initializeDirmngrCheckbox( d->ui.ignoreLDAPDPCB, e.mIgnoreLDAPDPEntry );
    initializeDirmngrCheckbox( d->ui.disableLDAPCB, e.mDisableLDAPEntry );
    if ( e.mCustomHTTPProxy ) {
        QString systemProxy = QString::fromLocal8Bit( qgetenv( "http_proxy" ) );
        if ( systemProxy.isEmpty() )
            systemProxy = i18n( "no proxy" );
        d->ui.systemHTTPProxy->setText( i18n( "(Current system setting: %1)", systemProxy ) );
        const bool honor = e.mHonorHTTPProxy && e.mHonorHTTPProxy->boolValue();
        d->ui.honorHTTPProxyRB->setChecked( honor );
        d->ui.useCustomHTTPProxyRB->setChecked( !honor );
        d->ui.customHTTPProxy->setText( e.mCustomHTTPProxy->stringValue() );
    } 
    d->customHTTPProxyWritable = e.mCustomHTTPProxy && !e.mCustomHTTPProxy->isReadOnly();
    if ( !d->customHTTPProxyWritable ) {
        disableDirmngrWidget( d->ui.honorHTTPProxyRB );
        disableDirmngrWidget( d->ui.useCustomHTTPProxyRB );
        disableDirmngrWidget( d->ui.systemHTTPProxy );
        disableDirmngrWidget( d->ui.customHTTPProxy );
    }
    if ( e.mCustomLDAPProxy )
        d->ui.customLDAPProxy->setText( e.mCustomLDAPProxy->stringValue() );
    if ( !e.mCustomLDAPProxy || e.mCustomLDAPProxy->isReadOnly() ) {
        disableDirmngrWidget( d->ui.customLDAPProxy );
        disableDirmngrWidget( d->ui.customLDAPLabel );
    }
    d->enableDisableActions();
}

static void saveCheckBoxToKleoEntry( QCheckBox * cb, CryptoConfigEntry * entry ) {
    const bool b = cb->isChecked();
    if ( entry && entry->boolValue() != b )
        entry->setBoolValue( b );
}

void SMimeValidationConfigurationWidget::save() const {
    CryptoConfig * const config = CryptoBackendFactory::instance()->config();
    if ( !config )
        return;

    {
        SMimeValidationPreferences preferences;
        preferences.setRefreshInterval( d->ui.intervalRefreshCB->isChecked() ? d->ui.intervalRefreshSB->value() : 0 );
        preferences.writeConfig();
    }

    // Create config entries
    // Don't keep them around, they'll get deleted by clear(), which could be done by the
    // "configure backend" button.
    const SMIMECryptoConfigEntries e( config );

    const bool b = d->ui.OCSPRB->isChecked();
    if ( e.mCheckUsingOCSPConfigEntry && e.mCheckUsingOCSPConfigEntry->boolValue() != b )
        e.mCheckUsingOCSPConfigEntry->setBoolValue( b );
    // Set allow-ocsp together with enable-ocsp
    if ( e.mEnableOCSPsendingConfigEntry && e.mEnableOCSPsendingConfigEntry->boolValue() != b )
        e.mEnableOCSPsendingConfigEntry->setBoolValue( b );

    saveCheckBoxToKleoEntry( d->ui.doNotCheckCertPolicyCB, e.mDoNotCheckCertPolicyConfigEntry );
    saveCheckBoxToKleoEntry( d->ui.neverConsultCB, e.mNeverConsultConfigEntry );
    saveCheckBoxToKleoEntry( d->ui.allowMarkTrustedCB, e.mAllowMarkTrustedConfigEntry );
    saveCheckBoxToKleoEntry( d->ui.fetchMissingCB, e.mFetchMissingConfigEntry );

    QString txt = d->ui.OCSPResponderURL->text();
    if ( e.mOCSPResponderURLConfigEntry && e.mOCSPResponderURLConfigEntry->stringValue() != txt )
        e.mOCSPResponderURLConfigEntry->setStringValue( txt );

    txt = d->ui.OCSPResponderSignature->selectedCertificate();
    if ( e.mOCSPResponderSignature && e.mOCSPResponderSignature->stringValue() != txt )
        e.mOCSPResponderSignature->setStringValue( txt );

    //dirmngr-0.9.0 options
    saveCheckBoxToKleoEntry( d->ui.ignoreServiceURLCB, e.mIgnoreServiceURLEntry );
    saveCheckBoxToKleoEntry( d->ui.ignoreHTTPDPCB, e.mIgnoreHTTPDPEntry );
    saveCheckBoxToKleoEntry( d->ui.disableHTTPCB, e.mDisableHTTPEntry );
    saveCheckBoxToKleoEntry( d->ui.ignoreLDAPDPCB, e.mIgnoreLDAPDPEntry );
    saveCheckBoxToKleoEntry( d->ui.disableLDAPCB, e.mDisableLDAPEntry );
    if ( e.mCustomHTTPProxy ) {
        const bool honor = d->ui.honorHTTPProxyRB->isChecked();
        if ( e.mHonorHTTPProxy && e.mHonorHTTPProxy->boolValue() != honor )
            e.mHonorHTTPProxy->setBoolValue( honor );

        const QString chosenProxy = d->ui.customHTTPProxy->text();
        if ( chosenProxy != e.mCustomHTTPProxy->stringValue() )
            e.mCustomHTTPProxy->setStringValue( chosenProxy );
    }
    txt = d->ui.customLDAPProxy->text();
    if ( e.mCustomLDAPProxy && e.mCustomLDAPProxy->stringValue() != txt )
        e.mCustomLDAPProxy->setStringValue( d->ui.customLDAPProxy->text() );

    config->sync( true );
}

CryptoConfigEntry * SMIMECryptoConfigEntries::configEntry( const char * componentName,
                                                           const char * groupName,
                                                           const char * entryName,
                                                           int /*CryptoConfigEntry::ArgType*/ argType,
                                                           bool isList )
{
    CryptoConfigEntry * const entry = mConfig->entry( QLatin1String(componentName), QLatin1String(groupName), QLatin1String(entryName) );
    if ( !entry ) {
        kWarning(5151) << QString::fromLatin1("Backend error: gpgconf doesn't seem to know the entry for %1/%2/%3" ).arg( QLatin1String(componentName), QLatin1String(groupName), QLatin1String(entryName) );
        return 0;
    }
    if( entry->argType() != argType || entry->isList() != isList ) {
        kWarning(5151) << QString::fromLatin1("Backend error: gpgconf has wrong type for %1/%2/%3: %4 %5" ).arg( QLatin1String(componentName), QLatin1String(groupName), QLatin1String(entryName) ).arg( entry->argType() ).arg( entry->isList() );
        return 0;
    }
    return entry;
}


#include "moc_smimevalidationconfigurationwidget.cpp"
