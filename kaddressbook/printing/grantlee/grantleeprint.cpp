/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "grantleeprint.h"
#include "contactgrantleeprintobject.h"

#include "kaddressbookgrantlee/formatter/grantleecontactutils.h"

#include <grantlee/context.h>
#include <grantlee/engine.h>
#include <grantlee/templateloader.h>


using namespace KABPrinting;

GrantleePrint::GrantleePrint(const QString &themePath, QObject *parent)
    : QObject(parent)
{
    mEngine = new Grantlee::Engine;
    mTemplateLoader = Grantlee::FileSystemTemplateLoader::Ptr( new Grantlee::FileSystemTemplateLoader );

    mTemplateLoader->setTemplateDirs( QStringList() << themePath );
    mEngine->addTemplateLoader( mTemplateLoader );

    mSelfcontainedTemplate = mEngine->loadByName( QLatin1String("theme.html") );
    if ( mSelfcontainedTemplate->error() ) {
        mErrorMessage = mSelfcontainedTemplate->errorString() + QLatin1String("<br>");
    }
}

GrantleePrint::~GrantleePrint()
{
    delete mEngine;
}

QString GrantleePrint::contactsToHtml( const KABC::Addressee::List &contacts )
{
    if (!mErrorMessage.isEmpty())
        return mErrorMessage;

    if (contacts.isEmpty()) {
        return QString();
    }
    QVariantList contactsList;
    QList<ContactGrantleePrintObject*> lst;
    Q_FOREACH (const KABC::Addressee &address, contacts) {
        ContactGrantleePrintObject *contactPrintObject = new ContactGrantleePrintObject(address);
        lst.append(contactPrintObject);
        contactsList << QVariant::fromValue(static_cast<QObject*>(contactPrintObject));
    }
    QVariantHash mapping;
    QVariantHash contactI18n;
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "birthdayi18n" ) );
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String("anniversaryi18n") );
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "emailsi18n" ) );
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "websitei18n" ) );
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "blogUrli18n" ) );
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "addressBookNamei18n" ) );
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "notei18n" ) );
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "departmenti18n" ) );
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "Professioni18n" ) );
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "officei18n" ) );
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "manageri18n" ) );
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "assistanti18n" ) );
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "spousei18n" ) );
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "imAddressi18n" ) );
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "latitudei18n" ));
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "longitudei18n" ));
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "organizationi18n" ));
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "titlei18n" ));
    GrantleeContactUtils::insertVariableToQVariantHash( contactI18n, QLatin1String( "nextcontacti18n" ));
    mapping.insert( QLatin1String("contacti18n"), contactI18n );

    Grantlee::Context context( mapping );
    context.insert(QLatin1String("contacts"), contactsList);
    const QString content = mSelfcontainedTemplate->render( &context );
    qDeleteAll(lst);
    return content;
}
