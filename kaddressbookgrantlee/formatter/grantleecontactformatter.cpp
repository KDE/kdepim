/*
  This file is part of KAddressBook.

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
#include "grantleetheme/grantleetheme.h"
#include "grantleecontactutils.h"

#include <grantlee/context.h>
#include <grantlee/engine.h>
#include <grantlee/templateloader.h>

#include <AkonadiCore/Item>

#include <KABC/Addressee>

#include <KColorScheme>
#include <KGlobal>
#include <KIconLoader>
#include <KLocale>
#include <KStringHandler>
#include <KConfigGroup>

#include "Akonadi/Contact/improtocols.h"

#include <QSet>
#include <QRegExp>
#include <QTextDocument>

using namespace KAddressBookGrantlee;

class GrantleeContactFormatter::Private
{
public:
    Private()
        : forceDisableQRCode(false)
    {
        mEngine = new Grantlee::Engine;

        mTemplateLoader = Grantlee::FileSystemTemplateLoader::Ptr( new Grantlee::FileSystemTemplateLoader );
    }

    ~Private()
    {
        delete mEngine;
    }

    void changeGrantleePath(const QString &path)
    {
        mTemplateLoader->setTemplateDirs( QStringList() << path );
        mEngine->addTemplateLoader( mTemplateLoader );

        mSelfcontainedTemplate = mEngine->loadByName( QLatin1String("contact.html") );
        if ( mSelfcontainedTemplate->error() ) {
            mErrorMessage += mSelfcontainedTemplate->errorString() + QLatin1String("<br>");
        }

        mEmbeddableTemplate = mEngine->loadByName( QLatin1String("contact_embedded.html") );
        if ( mEmbeddableTemplate->error() ) {
            mErrorMessage += mEmbeddableTemplate->errorString() + QLatin1String("<br>");
        }
    }


    QVector<QObject*> mObjects;
    Grantlee::Engine *mEngine;
    Grantlee::FileSystemTemplateLoader::Ptr mTemplateLoader;
    Grantlee::Template mSelfcontainedTemplate;
    Grantlee::Template mEmbeddableTemplate;
    QString mErrorMessage;
    bool forceDisableQRCode;
};

GrantleeContactFormatter::GrantleeContactFormatter()
    : d( new Private )
{
}

GrantleeContactFormatter::~GrantleeContactFormatter()
{
    delete d;
}

void GrantleeContactFormatter::setAbsoluteThemePath(const QString &path)
{
    d->changeGrantleePath(path);
}

void GrantleeContactFormatter::setGrantleeTheme(const GrantleeTheme::Theme &theme)
{
    d->changeGrantleePath(theme.absolutePath());
}

void GrantleeContactFormatter::setForceDisableQRCode(bool b)
{
    d->forceDisableQRCode = b;
}

bool GrantleeContactFormatter::forceDisableQRCode() const
{
    return d->forceDisableQRCode;
}

inline static void setHashField( QVariantHash &hash, const QString &name, const QString &value )
{
    if ( !value.isEmpty() ) {
        hash.insert( name, value );
    }
}

static QVariantHash phoneNumberHash( const KABC::PhoneNumber &phoneNumber, int counter )
{
    QVariantHash numberObject;

    setHashField( numberObject, QLatin1String( "type" ), phoneNumber.typeLabel() );
    setHashField( numberObject, QLatin1String( "number" ), phoneNumber.number() );

    if ( !phoneNumber.isEmpty() ) {
        const QString url =
                QString::fromLatin1( "<a href=\"phone:?index=%1\">%2</a>" ).
                arg( counter ).
                arg( phoneNumber.number() );
        numberObject.insert( QLatin1String( "numberLink" ), url );

        if ( phoneNumber.type() & KABC::PhoneNumber::Cell ) {
            const QString url =
                    QString::fromLatin1( "<a href=\"sms:?index=%1\"><img src=\"sms_icon\" align=\"top\"/></a>" ).arg( counter );
            numberObject.insert( QLatin1String( "smsLink" ), url );
        }
    }

    return numberObject;
}

static QVariantHash imAddressHash( const QString &typeKey, const QString &imAddress )
{
    QVariantHash addressObject;

    const QString dispLabel = i18nc( "@title:row label for an Instant Messaging address, %1 is I18Ned protocol name",
                                     "IM (%1)", IMProtocols::self()->name( typeKey ) );

    setHashField( addressObject, QLatin1String( "type" ), dispLabel );
    setHashField( addressObject, QLatin1String( "imAddress" ), imAddress );

    const QString iconUrl = KUrl::fromPath( KIconLoader::global()->iconPath( IMProtocols::self()->icon( typeKey ),
                                                                             -KIconLoader::SizeSmall) ).url();
    const QString url = QString::fromLatin1( "<img src=\"%1\" align=\"top\"/>" ).arg( iconUrl );
    addressObject.insert( QLatin1String( "imIcon" ), url );

    return addressObject;
}

static QVariantHash addressHash( const KABC::Address &address, int counter )
{
    QVariantHash addressObject;

    setHashField( addressObject, QLatin1String( "type" ),
                  KABC::Address::typeLabel( address.type() ) );

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
        formattedAddress = formattedAddress.replace( QRegExp( QLatin1String("\n+") ), QLatin1String( "<br/>" ) );

        const QString link = QString::fromLatin1( "<a href=\"address:?index=%1\">%2</a>" ).
                arg( counter );
        QString url = link.arg( formattedAddress );
        addressObject.insert( QLatin1String("formattedAddressLink"), url );

        url = link.arg( QString::fromLatin1( "<img src=\"map_icon\" align=\"top\"/>" ) );
        addressObject.insert( QLatin1String("formattedAddressIcon"), url );
    }

    return addressObject;
}

static int contactAge( const QDate &date )
{
    QDate now = QDate::currentDate();
    int age = now.year() - date.year();
    if ( date > now.addYears( -age ) ) {
        age--;
    }
    return age;
}

QString GrantleeContactFormatter::toHtml( HtmlForm form ) const
{
    if ( !d->mErrorMessage.isEmpty() ) {
        return d->mErrorMessage;
    }

    KABC::Addressee rawContact;
    const Akonadi::Item localItem = item();
    if ( localItem.isValid() && localItem.hasPayload<KABC::Addressee>() ) {
        rawContact = localItem.payload<KABC::Addressee>();
    } else {
        rawContact = contact();
    }

    if ( rawContact.isEmpty() ) {
        return QString();
    }

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
        GrantleeContactUtils::insertVariableToQVariantHash(contactObject, QLatin1String( "birthdayi18n" ));

        const QString formattedBirthday = KGlobal::locale()->formatDate( birthday );
        contactObject.insert( QLatin1String( "birthday" ), formattedBirthday );

        const int years = contactAge( birthday );
        contactObject.insert( QLatin1String( "age" ), QString::number( years ) );
        contactObject.insert( QLatin1String( "birthdayage" ), QString( formattedBirthday +
                                                              QLatin1String( "&nbsp;&nbsp;" ) +
                                                              i18np( "(One year old)", "(%1 years old)", years ) ) );
    }

    const QDate anniversary =
            QDate::fromString( rawContact.custom( QLatin1String( "KADDRESSBOOK" ),
                                                  QLatin1String( "X-Anniversary" ) ), Qt::ISODate );
    if ( anniversary.isValid() ) {
        contactObject.insert( QLatin1String( "anniversary" ),
                              KGlobal::locale()->formatDate( anniversary ) );
        GrantleeContactUtils::insertVariableToQVariantHash(contactObject, QLatin1String("anniversaryi18n") );
    }

    // Emails
    QStringList emails;
    foreach ( const QString &email, rawContact.emails() ) {
        const QString fullEmail = QString::fromLatin1( KUrl::toPercentEncoding( rawContact.fullEmail( email ) ) );

        const QString url = QString::fromLatin1( "<a href=\"mailto:%1\">%2</a>" )
                .arg( fullEmail, email );
        emails << url;
    }
    GrantleeContactUtils::insertVariableToQVariantHash(contactObject, QLatin1String( "emailsi18n" ) );
    contactObject.insert( QLatin1String( "emails" ), emails );

    // Phone numbers
    QVariantList phoneNumbers;
    int counter = 0;
    foreach ( const KABC::PhoneNumber &phoneNumber, rawContact.phoneNumbers() ) {
        phoneNumbers.append( phoneNumberHash( phoneNumber, counter ) );
        counter++;
    }

    contactObject.insert( QLatin1String( "phoneNumbers" ), phoneNumbers );

    // IM
    QVariantList imAddresses;
   const QStringList customs = rawContact.customs();
    if ( !customs.empty() ) {
        foreach ( const QString &custom, customs ) {
            if ( custom.startsWith( QLatin1String( "messaging/" ) ) ) {
                int pos = custom.indexOf( QLatin1Char( ':' ) );
                QString key = custom.left( pos );
                key.remove( QLatin1String( "-All" ) );
                const QString value = custom.mid( pos + 1 );

                imAddresses.append( imAddressHash( key, value ) );
            }
        }
    }

    contactObject.insert( QLatin1String( "imAddresses" ), imAddresses );

    // Homepage
    if ( rawContact.url().isValid() ) {
        QString url = rawContact.url().url();
        if ( !url.startsWith( QLatin1String( "http://" ) ) &&
             !url.startsWith( QLatin1String( "https://" ) ) ) {
            url = QLatin1String( "http://" ) + url;
        }

        url = KStringHandler::tagUrls( url );
        contactObject.insert( QLatin1String( "website" ), url );
        GrantleeContactUtils::insertVariableToQVariantHash(contactObject, QLatin1String( "websitei18n" ) );
    }

    // Blog Feed
    const QString blog =
            rawContact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "BlogFeed" ) );
    if ( !blog.isEmpty() ) {
        contactObject.insert( QLatin1String( "blogUrl" ), KStringHandler::tagUrls( blog ) );
        GrantleeContactUtils::insertVariableToQVariantHash(contactObject, QLatin1String( "blogUrli18n" ) );
    }

    // Address Book
    const QString addressBookName =
            rawContact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "AddressBook" ) );
    if ( !addressBookName.isEmpty() ) {
        contactObject.insert( QLatin1String( "addressBookName" ), addressBookName );
        GrantleeContactUtils::insertVariableToQVariantHash(contactObject, QLatin1String( "addressBookNamei18n" ) );
    }

    // Addresses
    QVariantList addresses;
    counter = 0;
    foreach ( const KABC::Address &address, rawContact.addresses() ) {
        addresses.append( addressHash( address, counter ) );
        counter++;
    }
    // Note
    if ( !rawContact.note().isEmpty() ) {
        const QString notes = QString::fromLatin1( "<a>%1</a>" ).arg(rawContact.note().replace( QLatin1Char( '\n' ), QLatin1String( "<br>" )));
        contactObject.insert( QLatin1String( "note" ), notes );
        GrantleeContactUtils::insertVariableToQVariantHash(contactObject, QLatin1String( "notei18n" ) );
    }

    contactObject.insert( QLatin1String( "addresses" ), addresses );

    setHashField( contactObject, QLatin1String( "mailer" ), rawContact.mailer() );

    setHashField( contactObject, QLatin1String( "title" ), rawContact.title() );

    setHashField( contactObject, QLatin1String( "role" ), rawContact.role() );

    QString job = rawContact.title();
    if ( job.isEmpty() ) {
        job = rawContact.role();
    }
    if ( job.isEmpty() ) {
        job = rawContact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Profession" ) );
    }
    setHashField( contactObject, QLatin1String( "job" ), job );

    setHashField( contactObject, QLatin1String( "organization" ), rawContact.organization() );

    setHashField( contactObject, QLatin1String( "department" ), rawContact.department() );
    GrantleeContactUtils::insertVariableToQVariantHash(contactObject, QLatin1String( "departmenti18n" ) );

    //setHashField( contactObject, QLatin1String( "note" ), rawContact.note() );

    setHashField( contactObject, QLatin1String( "profession" ),
                  rawContact.custom( QLatin1String( "KADDRESSBOOK" ),
                                     QLatin1String( "X-Profession" ) ) );
    GrantleeContactUtils::insertVariableToQVariantHash(contactObject, QLatin1String( "Professioni18n" ) );
    setHashField( contactObject, QLatin1String( "office" ),
                  rawContact.custom( QLatin1String( "KADDRESSBOOK" ),
                                     QLatin1String( "X-Office" ) ) );
    GrantleeContactUtils::insertVariableToQVariantHash(contactObject, QLatin1String( "officei18n" ) );

    setHashField( contactObject, QLatin1String( "manager" ),
                  rawContact.custom( QLatin1String( "KADDRESSBOOK" ),
                                     QLatin1String( "X-ManagersName" ) ) );
    GrantleeContactUtils::insertVariableToQVariantHash(contactObject, QLatin1String( "manageri18n" ) );

    setHashField( contactObject, QLatin1String( "assistant" ),
                  rawContact.custom( QLatin1String( "KADDRESSBOOK" ),
                                     QLatin1String( "X-AssistantsName" ) ) );
    GrantleeContactUtils::insertVariableToQVariantHash(contactObject, QLatin1String( "assistanti18n" ) );

    setHashField( contactObject, QLatin1String( "spouse" ),
                  rawContact.custom( QLatin1String( "KADDRESSBOOK" ),
                                     QLatin1String( "X-SpousesName" ) ) );
    GrantleeContactUtils::insertVariableToQVariantHash(contactObject, QLatin1String( "spousei18n" ) );

    setHashField( contactObject, QLatin1String( "imAddress" ),
                  rawContact.custom( QLatin1String( "KADDRESSBOOK" ),
                                     QLatin1String( "X-IMAddress" ) ) );
    GrantleeContactUtils::insertVariableToQVariantHash(contactObject, QLatin1String( "imAddressi18n" ));

    // Custom fields

    QVariantList customFields;
    QVariantList customFieldsUrl;
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
        blacklistedKeys.insert( QLatin1String( "MailPreferedFormatting" ) );
        blacklistedKeys.insert( QLatin1String( "MailAllowToRemoteContent" ) );
    }

    if ( !customs.empty() ) {
        foreach ( QString custom, customs ) { //krazy:exclude=foreach
            if ( custom.startsWith( QLatin1String( "KADDRESSBOOK-" ) ) ) {
                custom.remove( QLatin1String( "KADDRESSBOOK-X-" ) );
                custom.remove( QLatin1String( "KADDRESSBOOK-" ) );

                int pos = custom.indexOf( QLatin1Char( ':' ) );
                QString key = custom.left( pos );
                QString value = custom.mid( pos + 1 );

                if ( blacklistedKeys.contains( key ) ) {
                    continue;
                }

                bool addUrl = false;
                // check whether it is a custom local field
                foreach ( const QVariantMap &description, customFieldDescriptions() ) {
                    if ( description.value( QLatin1String( "key" ) ).toString() == key ) {
                        key = description.value( QLatin1String( "title" ) ).toString();
                        const QString descriptionType = description.value( QLatin1String( "type" ) ).toString();
                        if ( descriptionType == QLatin1String( "boolean" ) ) {
                            if ( value == QLatin1String( "true" ) ) {
                                value = i18nc( "Boolean value", "yes" );
                            } else {
                                value = i18nc( "Boolean value", "no" );
                            }

                        } else if ( descriptionType  == QLatin1String( "date" ) ) {
                            const QDate date = QDate::fromString( value, Qt::ISODate );
                            value = KGlobal::locale()->formatDate( date, KLocale::ShortDate );

                        } else if ( descriptionType == QLatin1String( "time" ) ) {
                            const QTime time = QTime::fromString( value, Qt::ISODate );
                            value = KGlobal::locale()->formatTime( time );

                        } else if ( descriptionType == QLatin1String( "datetime" ) ) {
                            const QDateTime dateTime = QDateTime::fromString( value, Qt::ISODate );
                            value = KGlobal::locale()->formatDateTime( dateTime, KLocale::ShortDate );
                        } else if ( descriptionType == QLatin1String("url") ) {
                            value = KStringHandler::tagUrls( Qt::escape(value) );
                            addUrl = true;
                        }
                        break;
                    }
                }
                QVariantHash customFieldObject;
                customFieldObject.insert( QLatin1String( "title" ), key );
                customFieldObject.insert( QLatin1String( "value" ), value );

                if (addUrl) {
                    customFieldsUrl.append( customFieldObject );
                } else {
                    customFields.append( customFieldObject );
                }
            }
        }
    }

    contactObject.insert( QLatin1String( "customFields" ), customFields );
    contactObject.insert( QLatin1String( "customFieldsUrl" ), customFieldsUrl );

#if defined(HAVE_PRISON)
    if (!d->forceDisableQRCode) {
        KConfig config( QLatin1String( "akonadi_contactrc" ) );
        KConfigGroup group( &config, QLatin1String( "View" ) );
        if (group.readEntry( "QRCodes", true )) {
            contactObject.insert( QLatin1String( "hasqrcode" ), QLatin1String("true") );
        }
    }
#endif

    QVariantHash colorsObject;

    colorsObject.insert(
                QLatin1String("linkColor"),
                KColorScheme( QPalette::Active, KColorScheme::View ).foreground().color().name() );

    colorsObject.insert(
                QLatin1String("textColor"),
                KColorScheme( QPalette::Active, KColorScheme::View ).foreground().color().name() );

    colorsObject.insert(
                QLatin1String("backgroundColor"),
                KColorScheme( QPalette::Active, KColorScheme::View ).background().color().name() );

    QVariantHash mapping;
    mapping.insert( QLatin1String("contact"), contactObject );
    mapping.insert( QLatin1String("colors"), colorsObject );

    Grantlee::Context context( mapping );

    if ( form == SelfcontainedForm ) {
        return d->mSelfcontainedTemplate->render( &context );
    } else if ( form == EmbeddableForm ) {
        return d->mEmbeddableTemplate->render( &context );
    } else {
        return QString();
    }
}
