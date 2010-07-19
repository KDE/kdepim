/*
    This file is part of Akonadi Contact.

    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "grantleecontactformatter.h"

#include <grantlee/context.h>
#include <grantlee/engine.h>
#include <grantlee/templateloader.h>

#include <akonadi/item.h>
#include <kabc/addressee.h>
#include <kcolorscheme.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstringhandler.h>

#include <QtCore/QSet>

using namespace Akonadi;

class GrantleeContactFormatter::Private
{
  public:
    Private( const QString &templatePath )
    {
      qDebug("GrantleeContactFormatter::Private called");
      mEngine = new Grantlee::Engine;

      mTemplateLoader = Grantlee::FileSystemTemplateLoader::Ptr( new Grantlee::FileSystemTemplateLoader );
      mTemplateLoader->setTemplateDirs( QStringList() << templatePath );
      mTemplateLoader->setTheme( QLatin1String( "default" ) );

      mEngine->addTemplateLoader( mTemplateLoader );
      mSelfcontainedTemplate = mEngine->loadByName( "contact.html" );
      if ( mSelfcontainedTemplate->error() )
        qDebug() << "error:" << mSelfcontainedTemplate->errorString();

      mEmbeddableTemplate = mEngine->loadByName( "contact_embedded.html" );
      if ( mEmbeddableTemplate->error() )
        qDebug() << "error:" << mEmbeddableTemplate->errorString();
    }

    ~Private()
    {
      delete mEngine;
    }

    QVector<QObject*> mObjects;
    Grantlee::Engine *mEngine;
    Grantlee::FileSystemTemplateLoader::Ptr mTemplateLoader;
    Grantlee::Template mSelfcontainedTemplate;
    Grantlee::Template mEmbeddableTemplate;
};

GrantleeContactFormatter::GrantleeContactFormatter( const QString &templatePath )
  : d( new Private( templatePath ) )
{
}

GrantleeContactFormatter::~GrantleeContactFormatter()
{
  delete d;
}

inline static void setHashField( QVariantHash &hash, const QString &name, const QString &value )
{
  if ( !value.isEmpty() )
    hash.insert( name, value );
}

static QVariantHash phoneNumberHash( const KABC::PhoneNumber &phoneNumber, int counter )
{
  QVariantHash numberObject;

  setHashField( numberObject, QLatin1String( "type" ), phoneNumber.typeLabel() );
  setHashField( numberObject, QLatin1String( "number" ), phoneNumber.number() );

  if ( !phoneNumber.isEmpty() ) {
    const QString url = QString::fromLatin1( "<a href=\"phone:?index=%1\">%2</a>" ).arg( counter ).arg( phoneNumber.number() );
    numberObject.insert( QLatin1String( "numberLink" ), url );
  }

  return numberObject;
}

static QVariantHash addressHash( const KABC::Address &address, int counter )
{
  QVariantHash addressObject;

  setHashField( addressObject, QLatin1String( "type" ), KABC::Address::typeLabel( address.type() ) );
  setHashField( addressObject, QLatin1String( "street" ), address.street() );
  setHashField( addressObject, QLatin1String( "postOfficeBox" ), address.postOfficeBox() );
  setHashField( addressObject, QLatin1String( "locality" ), address.locality() );
  setHashField( addressObject, QLatin1String( "region" ), address.region() );
  setHashField( addressObject, QLatin1String( "postalCode" ), address.postalCode() );
  setHashField( addressObject, QLatin1String( "country" ), address.country() );
  setHashField( addressObject, QLatin1String( "label" ), address.label() );
  setHashField( addressObject, QLatin1String( "formattedAddress" ), address.formattedAddress() );

  QString formattedAddress;

  if ( address.label().isEmpty() ) {
    formattedAddress = address.formattedAddress().trimmed();
  } else {
    formattedAddress = address.label();
  }

  if ( !formattedAddress.isEmpty() ) {
    formattedAddress = formattedAddress.replace( QLatin1Char( '\n' ), QLatin1String( "<br/>" ) );

    const QString url = QString::fromLatin1( "<a href=\"address:?index=%1\">%2</a>" ).arg( counter).arg( formattedAddress );
    addressObject.insert( "formattedAddressLink", url );
  }

  return addressObject;
}

QString GrantleeContactFormatter::toHtml( HtmlForm form ) const
{
  KABC::Addressee rawContact;
  const Akonadi::Item localItem = item();
  if ( localItem.isValid() && localItem.hasPayload<KABC::Addressee>() )
    rawContact = localItem.payload<KABC::Addressee>();
  else
    rawContact = contact();

  if ( rawContact.isEmpty() )
    return QString();

  QVariantHash contactObject;

  // Name parts
  setHashField( contactObject, QLatin1String( "name" ), rawContact.realName() );
  setHashField( contactObject, QLatin1String( "formattedName" ), rawContact.formattedName() );
  setHashField( contactObject, QLatin1String( "prefix" ), rawContact.prefix() );
  setHashField( contactObject, QLatin1String( "givenName" ), rawContact.givenName() );
  setHashField( contactObject, QLatin1String( "additionalName" ), rawContact.additionalName() );
  setHashField( contactObject, QLatin1String( "familyName" ), rawContact.familyName() );
  setHashField( contactObject, QLatin1String( "suffix" ), rawContact.suffix() );
  setHashField( contactObject, QLatin1String( "nickName" ), rawContact.nickName() );

  // Dates
  const QDate birthday = rawContact.birthday().date();
  if ( birthday.isValid() ) {
    contactObject.insert( QLatin1String( "birthday" ), KGlobal::locale()->formatDate( birthday ) );

    const int years = (birthday.daysTo( QDate::currentDate() ) / 365);
    contactObject.insert( QLatin1String( "age" ), QString::number( years ) );
  }

  const QDate anniversary = QDate::fromString( rawContact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Anniversary" ) ), Qt::ISODate );
  if ( anniversary.isValid() ) {
    contactObject.insert( QLatin1String( "anniversary" ), KGlobal::locale()->formatDate( anniversary ) );
  }

  // Emails
  contactObject.insert( QLatin1String( "emails" ), rawContact.emails() );

  // Phone numbers
  QVariantList phoneNumbers;
  int counter = 0;
  foreach ( const KABC::PhoneNumber &phoneNumber, rawContact.phoneNumbers() ) {
    phoneNumbers.append( phoneNumberHash( phoneNumber, counter ) );
    counter++;
  }

  contactObject.insert( QLatin1String( "phoneNumbers" ), phoneNumbers );

  // Homepage
  if ( rawContact.url().isValid() ) {
    QString url = rawContact.url().url();
    if ( !url.startsWith( QLatin1String( "http://" ) ) && !url.startsWith( QLatin1String( "https://" ) ) )
      url = QLatin1String( "http://" ) + url;

    url = KStringHandler::tagUrls( url );
    contactObject.insert( QLatin1String( "website" ), url );
  }

  // Blog Feed
  const QString blog = rawContact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "BlogFeed" ) );
  if ( !blog.isEmpty() ) {
    contactObject.insert( QLatin1String( "blogUrl" ), KStringHandler::tagUrls( blog ) );
  }

  // Address Book
  const QString addressBookName = rawContact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "AddressBook" ) );
  if ( !addressBookName.isEmpty() ) {
    contactObject.insert( QLatin1String( "addressBookName" ), addressBookName );
  }

  // Addresses
  QVariantList addresses;
  counter = 0;
  foreach ( const KABC::Address &address, rawContact.addresses() ) {
    addresses.append( addressHash( address, counter ) );
    counter++;
  }

  contactObject.insert( QLatin1String( "addresses" ), addresses );

  setHashField( contactObject, QLatin1String( "mailer" ), rawContact.mailer() );
  setHashField( contactObject, QLatin1String( "title" ), rawContact.title() );
  setHashField( contactObject, QLatin1String( "role" ), rawContact.role() );
  setHashField( contactObject, QLatin1String( "organization" ), rawContact.organization() );
  setHashField( contactObject, QLatin1String( "department" ), rawContact.department() );
  setHashField( contactObject, QLatin1String( "note" ), rawContact.note() );
  setHashField( contactObject, QLatin1String( "profession" ), rawContact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Profession" ) ) );
  setHashField( contactObject, QLatin1String( "office" ), rawContact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Office" ) ) );
  setHashField( contactObject, QLatin1String( "manager" ), rawContact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-ManagersName" ) ) );
  setHashField( contactObject, QLatin1String( "assistant" ), rawContact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-AssistantsName" ) ) );
  setHashField( contactObject, QLatin1String( "spouse" ), rawContact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-SpousesName" ) ) );
  setHashField( contactObject, QLatin1String( "imAddress" ), rawContact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-IMAddress" ) ) );

  // Custom fields

  QVariantList customFields;

  static QSet<QString> blacklistedKeys;
  if ( blacklistedKeys.isEmpty() ) {
    blacklistedKeys.insert( QLatin1String( "CRYPTOPROTOPREF" ) );
    blacklistedKeys.insert( QLatin1String( "OPENPGPFP" ) );
    blacklistedKeys.insert( QLatin1String( "SMIMEFP" ) );
    blacklistedKeys.insert( QLatin1String( "CRYPTOSIGNPREF" ) );
    blacklistedKeys.insert( QLatin1String( "CRYPTOENCRYPTPREF" ) );
    blacklistedKeys.insert( QLatin1String( "Anniversary" ) );
    blacklistedKeys.insert( QLatin1String( "BlogFeed" ) );
    blacklistedKeys.insert( QLatin1String( "Profession" ) );
    blacklistedKeys.insert( QLatin1String( "Office" ) );
    blacklistedKeys.insert( QLatin1String( "ManagersName" ) );
    blacklistedKeys.insert( QLatin1String( "AssistantsName" ) );
    blacklistedKeys.insert( QLatin1String( "SpousesName" ) );
    blacklistedKeys.insert( QLatin1String( "IMAddress" ) );
    blacklistedKeys.insert( QLatin1String( "AddressBook" ) );
  }

  if ( !rawContact.customs().empty() ) {
    const QStringList customs = rawContact.customs();
    foreach ( QString custom, customs ) { //krazy:exclude=foreach
      if ( custom.startsWith( QLatin1String( "KADDRESSBOOK-" ) ) ) {
        custom.remove( QLatin1String( "KADDRESSBOOK-X-" ) );
        custom.remove( QLatin1String( "KADDRESSBOOK-" ) );

        int pos = custom.indexOf( QLatin1Char( ':' ) );
        QString key = custom.left( pos );
        QString value = custom.mid( pos + 1 );

        if ( blacklistedKeys.contains( key ) )
          continue;

        // check whether it is a custom local field
        foreach ( const QVariantMap &description, customFieldDescriptions() ) {
          if ( description.value( QLatin1String( "key" ) ).toString() == key ) {
            key = description.value( QLatin1String( "title" ) ).toString();
            if ( description.value( QLatin1String( "type" ) ) == QLatin1String( "boolean" ) ) {
              if ( value == QLatin1String( "true" ) )
                value = i18nc( "Boolean value", "yes" );
              else
                value = i18nc( "Boolean value", "no" );
            } else if ( description.value( QLatin1String( "type" ) ) == QLatin1String( "date" ) ) {
              const QDate date = QDate::fromString( value, Qt::ISODate );
              value = KGlobal::locale()->formatDate( date, KLocale::ShortDate );
            } else if ( description.value( QLatin1String( "type" ) ) == QLatin1String( "time" ) ) {
              const QTime time = QTime::fromString( value, Qt::ISODate );
              value = KGlobal::locale()->formatTime( time );
            } else if ( description.value( QLatin1String( "type" ) ) == QLatin1String( "datetime" ) ) {
              const QDateTime dateTime = QDateTime::fromString( value, Qt::ISODate );
              value = KGlobal::locale()->formatDateTime( dateTime, KLocale::ShortDate );
            }
            break;
          }
        }

        QVariantHash customFieldObject;
        customFieldObject.insert( QLatin1String( "title" ), key );
        customFieldObject.insert( QLatin1String( "value" ), value );

        customFields.append( customFieldObject );
      }
    }
  }

  contactObject.insert( QLatin1String( "customFields" ), customFields );

  QVariantHash colorsObject;
  colorsObject.insert( "linkColor", KColorScheme( QPalette::Active, KColorScheme::View ).foreground().color().name() );
  colorsObject.insert( "textColor", KColorScheme( QPalette::Active, KColorScheme::View ).foreground().color().name() );
  colorsObject.insert( "backgroundColor", KColorScheme( QPalette::Active, KColorScheme::View ).background().color().name() );

  QVariantHash mapping;
  mapping.insert( "contact", contactObject );
  mapping.insert( "colors", colorsObject );

  Grantlee::Context context( mapping );

  if ( form == SelfcontainedForm )
    return d->mSelfcontainedTemplate->render( &context );
  else if ( form == EmbeddableForm )
    return d->mEmbeddableTemplate->render( &context );
  else
    return QString();
}
