/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>
   by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "firstpage.h"

#include <libkmobiletools/deviceloader.h>
#include <libkmobiletools/enginexp.h>
#include <libkmobiletools/ifaces/wizardprovider.h>
#include <libkmobiletools/config.h>

#include <KDE/KMessageBox>
#include <KDE/KServiceTypeTrader>
#include <KDE/KLocale>
#include <KDE/KPluginInfo>
#include <KDE/KComboBox>
#include <KDE/KLineEdit>
#include <KDE/KDebug>

#include <QtGui/QAbstractButton>
#include <QtGui/QLabel>

using namespace KMobileTools;

class FirstPagePrivate {
public:
    QString engine;
    QString deviceName;
    QList<QWizardPage*> wizardPages;
};

FirstPage::FirstPage( QWidget* parent )
    : QWizardPage( parent )
{
    d = new FirstPagePrivate;
    setupUi( this );

    setTitle( i18nc( "First new device wizard page title", "Mobile Phone Information" ) );
    setSubTitle( i18n( "Please enter a name to identify your phone later (e.g. Nokia 3650)\n"
                       "and choose your preferred engine." ) );
}

QString FirstPage::engineName() const {
    return d->engine;
}

void FirstPage::setEngineName( const QString& engine ) {
    d->engine = engine;
}

void FirstPage::engineSelected( int index ) {
    // retrieve information about the engine from the user data of the selected combo box item
    QVariantMap engineInformation = engineSelection->itemData( index ).toMap();

    setEngineName( engineInformation.value( "internalName" ).toString() );
    emit engineNameChanged( engineName() );

    if( index == -1 )
        return;

    // prepare engine description
    QString descriptionLabel;
    descriptionLabel += engineInformation.value( "description" ).toString().replace( '\n', "<br>" );
    descriptionLabel += "<br><br>";
    descriptionLabel += i18n( "Author:" );
    descriptionLabel += ' ';

    QString author = engineInformation.value( "author" ).toString().replace( '\n', "<br>" );
    if( author.isEmpty() )
        descriptionLabel += i18n( "unknown" );
    else
        descriptionLabel += author;

    QString email = engineInformation.value( "email" ).toString();
    if( !email.isEmpty() )
        descriptionLabel += QString( " (<a href=\"mailto:%1\">%1</a>)" ).arg( email );

    engineDescription->setText( descriptionLabel );

    // remove old wizard pages from the engine
    for( int i=0; i<d->wizardPages.size(); i++ )
        delete d->wizardPages.at( i );
}

void FirstPage::initializePage()
{
    // setting up signal-slot connections
    connect( engineSelection, SIGNAL(currentIndexChanged (int)), this, SLOT(engineSelected(int)) );

    // registering fields for QWizard
    registerField( "phoneName*", phoneName );
    registerField( "engine*", this, "engineName", SIGNAL(engineNameChanged(const QString&)) );

    // detecting engines
    KService::List availableEngines = KServiceTypeTrader::self()->query( "KMobileTools/EngineXP" );
    if( availableEngines.count() ) {
        for(int i=0; i<availableEngines.count(); i++ ) {
            KPluginInfo engineInformation( availableEngines.at( i ) );

            QVariantMap additionalInformation;
            additionalInformation.insert( "internalName", availableEngines.at( i ).data()->name() );
            additionalInformation.insert( "description", engineInformation.property( "Description" ) );
            additionalInformation.insert( "author", engineInformation.author() );
            additionalInformation.insert( "email", engineInformation.email() );

            engineSelection->addItem( engineInformation.name(),
                                      additionalInformation );
        }

        engineSelected( engineSelection->currentIndex() );
    }
    else
        KMessageBox::error( this, i18n( "No engine could be found. Please re-install KMobileTools!" ),
                            i18n( "No engines found" ) );
}

void FirstPage::cleanupPage() {
    // unloading any previous loaded device
    if( !d->deviceName.isEmpty() )
        KMobileTools::DeviceLoader::instance()->unloadDevice( d->deviceName );
}

bool FirstPage::isFinalPage() const {
    return false;
}

bool FirstPage::validatePage() {
    cleanupPage();

    QString enteredDeviceName = phoneName->text();
    d->deviceName = enteredDeviceName;

    // look if the entered device name already exists
    QStringList devices = KMobileTools::Config::instance()->deviceList();

    if( devices.contains( enteredDeviceName ) ) {
        KMessageBox::error( this, i18n( "The device \"%1\" already exists.\nPlease choose another name!",
                                        phoneName->text() ),
                            i18n( "Device already exists" ) );
        return false;
    }

    if( engineSelection->currentText().isEmpty() )
        return false;

    // trying to load the engine
    QVariantMap engineInformation = engineSelection->itemData( engineSelection->currentIndex() ).toMap();
    QString engineName = engineInformation.value( "internalName" ).toString();

    // ... but do not load any service yet
    KMobileTools::DeviceLoader::instance()->loadDevice( enteredDeviceName, engineName, false );
    KMobileTools::EngineXP* engine = KMobileTools::DeviceLoader::instance()->engine( enteredDeviceName );

    if( engine ) {
        if( engine->implements( "WizardProvider" ) ) {
            KMobileTools::Ifaces::WizardProvider* wizardIface =
                qobject_cast<KMobileTools::Ifaces::WizardProvider*>( engine );
            if( wizardIface ) {
                QList<QWizardPage*> wizardPages = wizardIface->pageList();
                for( int i=0; i<wizardPages.size(); i++ )
                    wizard()->setPage( 2 + i, wizardPages.at( i ) );
            } else {
                /// @todo add a call to error handler here, since implements() doesn't work then
                kDebug() << "Obviously you forgot to register the wizard interface by "
                            "adding the Q_INTERFACES macro to your engine class...";
            }
        } else {
            // engine does not provide a wizard
            KMessageBox::error( this, i18n( "The selected engine does not provide a wizard "
                                            "to assist you with the device setup.\n"
                                            "You will have setup your device manually." ),
                                i18n( "Engine does not provide a guided device setup" ) );
        }
    } else {
        // engine could not be loaded
        KMessageBox::error( this, i18n( "The selected engine could not be loaded. Please try to select "
                                        "a different engine." ),
                            i18n( "Engine could not be loaded" ) );
        return false;
    }

    return true;
}


#include "firstpage.moc"
