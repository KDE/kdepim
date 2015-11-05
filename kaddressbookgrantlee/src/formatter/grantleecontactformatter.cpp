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

#include <KContacts/Addressee>

#include <KColorScheme>

#include <KIconLoader>
#include <KStringHandler>
#include <KConfigGroup>
#include <KLocalizedString>

#include <akonadi/contact/improtocols.h>

#include <QSet>
#include <QRegExp>
#include <QTextDocument>
#include <QLocale>

using namespace KAddressBookGrantlee;

class Q_DECL_HIDDEN GrantleeContactFormatter::Private
{
public:
    Private()
        : forceDisableQRCode(false)
    {
        mEngine = new Grantlee::Engine;

        mTemplateLoader = QSharedPointer<Grantlee::FileSystemTemplateLoader>(new Grantlee::FileSystemTemplateLoader());
    }

    ~Private()
    {
        delete mEngine;
    }

    void changeGrantleePath(const QString &path)
    {
        mTemplateLoader->setTemplateDirs(QStringList() << path);
        mEngine->addTemplateLoader(mTemplateLoader);

        mSelfcontainedTemplate = mEngine->loadByName(QStringLiteral("contact.html"));
        if (mSelfcontainedTemplate->error()) {
            mErrorMessage += mSelfcontainedTemplate->errorString() + QStringLiteral("<br>");
        }

        mEmbeddableTemplate = mEngine->loadByName(QStringLiteral("contact_embedded.html"));
        if (mEmbeddableTemplate->error()) {
            mErrorMessage += mEmbeddableTemplate->errorString() + QStringLiteral("<br>");
        }
    }

    QVector<QObject *> mObjects;
    Grantlee::Engine *mEngine;
    QSharedPointer<Grantlee::FileSystemTemplateLoader> mTemplateLoader;
    Grantlee::Template mSelfcontainedTemplate;
    Grantlee::Template mEmbeddableTemplate;
    QString mErrorMessage;
    bool forceDisableQRCode;
};

GrantleeContactFormatter::GrantleeContactFormatter()
    : d(new Private)
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

inline static void setHashField(QVariantHash &hash, const QString &name, const QString &value)
{
    if (!value.isEmpty()) {
        hash.insert(name, value);
    }
}

static QVariantHash phoneNumberHash(const KContacts::PhoneNumber &phoneNumber, int counter)
{
    QVariantHash numberObject;

    setHashField(numberObject, QStringLiteral("type"), phoneNumber.typeLabel());
    setHashField(numberObject, QStringLiteral("number"), phoneNumber.number());

    if (!phoneNumber.isEmpty()) {
        const QString url =
            QStringLiteral("<a href=\"phone:?index=%1\">%2</a>").
            arg(counter).
            arg(phoneNumber.number());
        numberObject.insert(QStringLiteral("numberLink"), url);

        if (phoneNumber.type() & KContacts::PhoneNumber::Cell) {
            const QString url =
                QStringLiteral("<a href=\"sms:?index=%1\"><img src=\"sms_icon\" align=\"top\"/></a>").arg(counter);
            numberObject.insert(QStringLiteral("smsLink"), url);
        }
    }

    return numberObject;
}

static QVariantHash imAddressHash(const QString &typeKey, const QString &imAddress)
{
    QVariantHash addressObject;

    const QString dispLabel = i18nc("@title:row label for an Instant Messaging address, %1 is I18Ned protocol name",
                                    "IM (%1)", IMProtocols::self()->name(typeKey));

    setHashField(addressObject, QStringLiteral("type"), dispLabel);
    setHashField(addressObject, QStringLiteral("imAddress"), imAddress);

    const QString iconUrl = QUrl::fromLocalFile(KIconLoader::global()->iconPath(IMProtocols::self()->icon(typeKey),
                            -KIconLoader::SizeSmall)).url();
    const QString url = QStringLiteral("<img src=\"%1\" align=\"top\"/>").arg(iconUrl);
    addressObject.insert(QStringLiteral("imIcon"), url);

    return addressObject;
}

static QVariantHash addressHash(const KContacts::Address &address, int counter)
{
    QVariantHash addressObject;

    setHashField(addressObject, QStringLiteral("type"),
                 KContacts::Address::typeLabel(address.type()));

    setHashField(addressObject, QStringLiteral("street"), address.street());

    setHashField(addressObject, QStringLiteral("postOfficeBox"), address.postOfficeBox());

    setHashField(addressObject, QStringLiteral("locality"), address.locality());

    setHashField(addressObject, QStringLiteral("region"), address.region());

    setHashField(addressObject, QStringLiteral("postalCode"), address.postalCode());

    setHashField(addressObject, QStringLiteral("country"), address.country());

    setHashField(addressObject, QStringLiteral("label"), address.label());

    setHashField(addressObject, QStringLiteral("formattedAddress"), address.formattedAddress());

    QString formattedAddress;

    if (address.label().isEmpty()) {
        formattedAddress = address.formattedAddress().trimmed();
    } else {
        formattedAddress = address.label();
    }

    if (!formattedAddress.isEmpty()) {
        formattedAddress = formattedAddress.replace(QRegExp(QLatin1String("\n+")), QStringLiteral("<br/>"));

        const QString link = QStringLiteral("<a href=\"address:?index=%1\">%2</a>").
                             arg(counter);
        QString url = link.arg(formattedAddress);
        addressObject.insert(QStringLiteral("formattedAddressLink"), url);

        url = link.arg(QStringLiteral("<img src=\"map_icon\" align=\"top\"/>"));
        addressObject.insert(QStringLiteral("formattedAddressIcon"), url);
    }

    return addressObject;
}

static int contactAge(const QDate &date)
{
    QDate now = QDate::currentDate();
    int age = now.year() - date.year();
    if (date > now.addYears(-age)) {
        age--;
    }
    return age;
}

QString GrantleeContactFormatter::toHtml(HtmlForm form) const
{
    if (!d->mErrorMessage.isEmpty()) {
        return d->mErrorMessage;
    }

    KContacts::Addressee rawContact;
    const Akonadi::Item localItem = item();
    if (localItem.isValid() && localItem.hasPayload<KContacts::Addressee>()) {
        rawContact = localItem.payload<KContacts::Addressee>();
    } else {
        rawContact = contact();
    }

    if (rawContact.isEmpty()) {
        return QString();
    }

    QVariantHash contactObject;
    GrantleeContactUtils grantleeContactUtil;
    // Name parts
    setHashField(contactObject, QStringLiteral("name"), rawContact.realName());
    setHashField(contactObject, QStringLiteral("formattedName"), rawContact.formattedName());
    setHashField(contactObject, QStringLiteral("prefix"), rawContact.prefix());
    setHashField(contactObject, QStringLiteral("givenName"), rawContact.givenName());
    setHashField(contactObject, QStringLiteral("additionalName"), rawContact.additionalName());
    setHashField(contactObject, QStringLiteral("familyName"), rawContact.familyName());
    setHashField(contactObject, QStringLiteral("suffix"), rawContact.suffix());
    setHashField(contactObject, QStringLiteral("nickName"), rawContact.nickName());

    // Dates
    const QDate birthday = rawContact.birthday().date();
    if (birthday.isValid()) {
        grantleeContactUtil.insertVariableToQVariantHash(contactObject, QStringLiteral("birthdayi18n"));

        const QString formattedBirthday = QLocale().toString(birthday);
        contactObject.insert(QStringLiteral("birthday"), formattedBirthday);

        const int years = contactAge(birthday);
        contactObject.insert(QStringLiteral("age"), QString::number(years));
        contactObject.insert(QStringLiteral("birthdayage"), QString(formattedBirthday +
                             QStringLiteral("&nbsp;&nbsp;") +
                             i18np("(One year old)", "(%1 years old)", years)));
    }

    const QDate anniversary =
        QDate::fromString(rawContact.custom(QStringLiteral("KADDRESSBOOK"),
                                            QStringLiteral("X-Anniversary")), Qt::ISODate);
    if (anniversary.isValid()) {
        contactObject.insert(QStringLiteral("anniversary"),
                             QLocale().toString(anniversary));
        grantleeContactUtil.insertVariableToQVariantHash(contactObject, QStringLiteral("anniversaryi18n"));
    }

    // Emails
    QStringList emails;
    foreach (const QString &email, rawContact.emails()) {
        const QString fullEmail = QString::fromLatin1(QUrl::toPercentEncoding(rawContact.fullEmail(email)));

        const QString url = QStringLiteral("<a href=\"mailto:%1\">%2</a>")
                            .arg(fullEmail, email);
        emails << url;
    }
    grantleeContactUtil.insertVariableToQVariantHash(contactObject, QStringLiteral("emailsi18n"));
    contactObject.insert(QStringLiteral("emails"), emails);

    // Phone numbers
    QVariantList phoneNumbers;
    int counter = 0;
    foreach (const KContacts::PhoneNumber &phoneNumber, rawContact.phoneNumbers()) {
        phoneNumbers.append(phoneNumberHash(phoneNumber, counter));
        counter++;
    }

    contactObject.insert(QStringLiteral("phoneNumbers"), phoneNumbers);

    // IM
    QVariantList imAddresses;
    const QStringList customs = rawContact.customs();
    if (!customs.empty()) {
        foreach (const QString &custom, customs) {
            if (custom.startsWith(QStringLiteral("messaging/"))) {
                int pos = custom.indexOf(QLatin1Char(':'));
                QString key = custom.left(pos);
                key.remove(QStringLiteral("-All"));
                const QString value = custom.mid(pos + 1);

                imAddresses.append(imAddressHash(key, value));
            }
        }
    }

    contactObject.insert(QStringLiteral("imAddresses"), imAddresses);

    // Homepage
    if (rawContact.url().url().isValid()) {
        QString url = rawContact.url().url().url();
        if (!url.startsWith(QStringLiteral("http://")) &&
                !url.startsWith(QStringLiteral("https://"))) {
            url = QStringLiteral("http://") + url;
        }

        url = KStringHandler::tagUrls(url);
        contactObject.insert(QStringLiteral("website"), url);
        grantleeContactUtil.insertVariableToQVariantHash(contactObject, QStringLiteral("websitei18n"));
    }

    // Blog Feed
    const QString blog =
        rawContact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("BlogFeed"));
    if (!blog.isEmpty()) {
        contactObject.insert(QStringLiteral("blogUrl"), KStringHandler::tagUrls(blog));
        grantleeContactUtil.insertVariableToQVariantHash(contactObject, QStringLiteral("blogUrli18n"));
    }

    // Address Book
    const QString addressBookName =
        rawContact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("AddressBook"));
    if (!addressBookName.isEmpty()) {
        contactObject.insert(QStringLiteral("addressBookName"), addressBookName);
        grantleeContactUtil.insertVariableToQVariantHash(contactObject, QStringLiteral("addressBookNamei18n"));
    }

    // Addresses
    QVariantList addresses;
    counter = 0;
    foreach (const KContacts::Address &address, rawContact.addresses()) {
        addresses.append(addressHash(address, counter));
        counter++;
    }
    // Note
    if (!rawContact.note().isEmpty()) {
        const QString notes = QStringLiteral("<a>%1</a>").arg(rawContact.note().replace(QLatin1Char('\n'), QStringLiteral("<br>")));
        contactObject.insert(QStringLiteral("note"), notes);
        grantleeContactUtil.insertVariableToQVariantHash(contactObject, QStringLiteral("notei18n"));
    }

    contactObject.insert(QStringLiteral("addresses"), addresses);

    setHashField(contactObject, QStringLiteral("mailer"), rawContact.mailer());

    setHashField(contactObject, QStringLiteral("title"), rawContact.title());

    setHashField(contactObject, QStringLiteral("role"), rawContact.role());

    QString job = rawContact.title();
    if (job.isEmpty()) {
        job = rawContact.role();
    }
    if (job.isEmpty()) {
        job = rawContact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Profession"));
    }
    setHashField(contactObject, QStringLiteral("job"), job);

    setHashField(contactObject, QStringLiteral("organization"), rawContact.organization());

    setHashField(contactObject, QStringLiteral("department"), rawContact.department());
    grantleeContactUtil.insertVariableToQVariantHash(contactObject, QStringLiteral("departmenti18n"));

    //setHashField( contactObject, QStringLiteral( "note" ), rawContact.note() );

    setHashField(contactObject, QStringLiteral("profession"),
                 rawContact.custom(QStringLiteral("KADDRESSBOOK"),
                                   QStringLiteral("X-Profession")));
    grantleeContactUtil.insertVariableToQVariantHash(contactObject, QStringLiteral("Professioni18n"));
    setHashField(contactObject, QStringLiteral("office"),
                 rawContact.custom(QStringLiteral("KADDRESSBOOK"),
                                   QStringLiteral("X-Office")));
    grantleeContactUtil.insertVariableToQVariantHash(contactObject, QStringLiteral("officei18n"));

    setHashField(contactObject, QStringLiteral("manager"),
                 rawContact.custom(QStringLiteral("KADDRESSBOOK"),
                                   QStringLiteral("X-ManagersName")));
    grantleeContactUtil.insertVariableToQVariantHash(contactObject, QStringLiteral("manageri18n"));

    setHashField(contactObject, QStringLiteral("assistant"),
                 rawContact.custom(QStringLiteral("KADDRESSBOOK"),
                                   QStringLiteral("X-AssistantsName")));
    grantleeContactUtil.insertVariableToQVariantHash(contactObject, QStringLiteral("assistanti18n"));

    setHashField(contactObject, QStringLiteral("spouse"),
                 rawContact.custom(QStringLiteral("KADDRESSBOOK"),
                                   QStringLiteral("X-SpousesName")));
    grantleeContactUtil.insertVariableToQVariantHash(contactObject, QStringLiteral("spousei18n"));

    setHashField(contactObject, QStringLiteral("imAddress"),
                 rawContact.custom(QStringLiteral("KADDRESSBOOK"),
                                   QStringLiteral("X-IMAddress")));
    grantleeContactUtil.insertVariableToQVariantHash(contactObject, QStringLiteral("imAddressi18n"));

    // Custom fields

    QVariantList customFields;
    QVariantList customFieldsUrl;
    static QSet<QString> blacklistedKeys;
    if (blacklistedKeys.isEmpty()) {
        blacklistedKeys.insert(QStringLiteral("CRYPTOPROTOPREF"));
        blacklistedKeys.insert(QStringLiteral("OPENPGPFP"));
        blacklistedKeys.insert(QStringLiteral("SMIMEFP"));
        blacklistedKeys.insert(QStringLiteral("CRYPTOSIGNPREF"));
        blacklistedKeys.insert(QStringLiteral("CRYPTOENCRYPTPREF"));
        blacklistedKeys.insert(QStringLiteral("Anniversary"));
        blacklistedKeys.insert(QStringLiteral("BlogFeed"));
        blacklistedKeys.insert(QStringLiteral("Profession"));
        blacklistedKeys.insert(QStringLiteral("Office"));
        blacklistedKeys.insert(QStringLiteral("ManagersName"));
        blacklistedKeys.insert(QStringLiteral("AssistantsName"));
        blacklistedKeys.insert(QStringLiteral("SpousesName"));
        blacklistedKeys.insert(QStringLiteral("IMAddress"));
        blacklistedKeys.insert(QStringLiteral("AddressBook"));
        blacklistedKeys.insert(QStringLiteral("MailPreferedFormatting"));
        blacklistedKeys.insert(QStringLiteral("MailAllowToRemoteContent"));
    }

    if (!customs.empty()) {
        foreach (QString custom, customs) {   //krazy:exclude=foreach
            if (custom.startsWith(QStringLiteral("KADDRESSBOOK-"))) {
                custom.remove(QStringLiteral("KADDRESSBOOK-X-"));
                custom.remove(QStringLiteral("KADDRESSBOOK-"));

                int pos = custom.indexOf(QLatin1Char(':'));
                QString key = custom.left(pos);
                QString value = custom.mid(pos + 1);

                if (blacklistedKeys.contains(key)) {
                    continue;
                }

                bool addUrl = false;
                // check whether it is a custom local field
                foreach (const QVariantMap &description, customFieldDescriptions()) {
                    if (description.value(QStringLiteral("key")).toString() == key) {
                        key = description.value(QStringLiteral("title")).toString();
                        const QString descriptionType = description.value(QStringLiteral("type")).toString();
                        if (descriptionType == QLatin1String("boolean")) {
                            if (value == QLatin1String("true")) {
                                value = i18nc("Boolean value", "yes");
                            } else {
                                value = i18nc("Boolean value", "no");
                            }

                        } else if (descriptionType  == QLatin1String("date")) {
                            const QDate date = QDate::fromString(value, Qt::ISODate);
                            value = QLocale().toString(date, QLocale::ShortFormat);

                        } else if (descriptionType == QLatin1String("time")) {
                            const QTime time = QTime::fromString(value, Qt::ISODate);
                            value = QLocale::system().toString(time, QLocale::ShortFormat);

                        } else if (descriptionType == QLatin1String("datetime")) {
                            const QDateTime dateTime = QDateTime::fromString(value, Qt::ISODate);
                            value = QLocale().toString(dateTime, QLocale::ShortFormat);
                        } else if (descriptionType == QLatin1String("url")) {
                            value = KStringHandler::tagUrls(value.toHtmlEscaped());
                            addUrl = true;
                        }
                        break;
                    }
                }
                QVariantHash customFieldObject;
                customFieldObject.insert(QStringLiteral("title"), key);
                customFieldObject.insert(QStringLiteral("value"), value);

                if (addUrl) {
                    customFieldsUrl.append(customFieldObject);
                } else {
                    customFields.append(customFieldObject);
                }
            }
        }
    }

    contactObject.insert(QStringLiteral("customFields"), customFields);
    contactObject.insert(QStringLiteral("customFieldsUrl"), customFieldsUrl);

#if defined(HAVE_PRISON)
    if (!d->forceDisableQRCode) {
        KConfig config(QStringLiteral("akonadi_contactrc"));
        KConfigGroup group(&config, QStringLiteral("View"));
        if (group.readEntry("QRCodes", true)) {
            contactObject.insert(QStringLiteral("hasqrcode"), QStringLiteral("true"));
        }
    }
#endif

    QVariantHash colorsObject;

    colorsObject.insert(
        QStringLiteral("linkColor"),
        KColorScheme(QPalette::Active, KColorScheme::View).foreground().color().name());

    colorsObject.insert(
        QStringLiteral("textColor"),
        KColorScheme(QPalette::Active, KColorScheme::View).foreground().color().name());

    colorsObject.insert(
        QStringLiteral("backgroundColor"),
        KColorScheme(QPalette::Active, KColorScheme::View).background().color().name());

    QVariantHash mapping;
    mapping.insert(QStringLiteral("contact"), contactObject);
    mapping.insert(QStringLiteral("colors"), colorsObject);

    Grantlee::Context context(mapping);

    if (form == SelfcontainedForm) {
        return d->mSelfcontainedTemplate->render(&context);
    } else if (form == EmbeddableForm) {
        return d->mEmbeddableTemplate->render(&context);
    } else {
        return QString();
    }
}
