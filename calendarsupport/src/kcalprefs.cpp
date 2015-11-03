/*
  Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kcalprefs.h"
#include "identitymanager.h"
#include "tagcache.h"
#include "calendarsupport_debug.h"

#include <AkonadiCore/TagAttribute>
#include <AkonadiCore/TagModifyJob>

#include <KMime/HeaderParsing>

#include <KIdentityManagement/Identity>
#include <KIdentityManagement/IdentityManager>

#include <KCodecs/KEmailAddress>

#include <KEMailSettings>
#include <KSystemTimeZone>

using namespace CalendarSupport;

Q_GLOBAL_STATIC(KCalPrefs, globalPrefs)

class Q_DECL_HIDDEN KCalPrefs::Private
{
public:
    Private() : mDefaultCalendarId(-1)
        , mDefaultCategoryColor(QColor(151, 235, 121))
    {
    }

    ~Private()
    {
    }

    KDateTime::Spec mTimeSpec;
    Akonadi::Collection::Id mDefaultCalendarId;

    TagCache mTagCache;
    QColor mDefaultCategoryColor;
    QDateTime mDayBegins;
};

KCalPrefs::KCalPrefs() : KCalPrefsBase(), d(new Private())
{
}

KCalPrefs::~KCalPrefs()
{
    delete d;
}

KCalPrefs *KCalPrefs::instance()
{
    static bool firstCall = true;

    if (firstCall) {
        firstCall = false;
        globalPrefs->load();
    }

    return globalPrefs;
}

void KCalPrefs::usrSetDefaults()
{
    // Default should be set a bit smarter, respecting username and locale
    // settings for example.

    KEMailSettings settings;
    QString tmp = settings.getSetting(KEMailSettings::RealName);
    if (!tmp.isEmpty()) {
        setUserName(tmp);
    }
    tmp = settings.getSetting(KEMailSettings::EmailAddress);
    if (!tmp.isEmpty()) {
        setUserEmail(tmp);
    }
    fillMailDefaults();

    setTimeZoneDefault();

    KConfigSkeleton::usrSetDefaults();
}

KDateTime::Spec KCalPrefs::timeSpec()
{
    return KSystemTimeZones::local();
}

void KCalPrefs::setTimeSpec(const KDateTime::Spec &spec)
{
    d->mTimeSpec = spec;
}

Akonadi::Collection::Id KCalPrefs::defaultCalendarId() const
{
    return d->mDefaultCalendarId;
}

void KCalPrefs::setDefaultCalendarId(const Akonadi::Collection::Id id)
{
    d->mDefaultCalendarId = id;
}

void KCalPrefs::setTimeZoneDefault()
{
    KTimeZone zone = KSystemTimeZones::local();
    if (!zone.isValid()) {
        qCCritical(CALENDARSUPPORT_LOG) << "KSystemTimeZones::local() return 0";
        return;
    }

    qCDebug(CALENDARSUPPORT_LOG) << "----- time zone:" << zone.name();

    d->mTimeSpec = zone;
}

void KCalPrefs::fillMailDefaults()
{
    userEmailItem()->swapDefault();
    QString defEmail = userEmailItem()->value();
    userEmailItem()->swapDefault();

    if (userEmail() == defEmail) {
        // No korg settings - but maybe there's a kcontrol[/kmail] setting available
        KEMailSettings settings;
        if (!settings.getSetting(KEMailSettings::EmailAddress).isEmpty()) {
            mEmailControlCenter = true;
        }
    }
}

void KCalPrefs::usrRead()
{
    KConfigGroup generalConfig(config(), "General");

    if (!d->mTimeSpec.isValid()) {
        setTimeZoneDefault();
    }

    KConfigGroup defaultCalendarConfig(config(), "Calendar");
    d->mDefaultCalendarId = defaultCalendarConfig.readEntry("Default Calendar", -1);

#if 0
    config()->setGroup("FreeBusy");
    if (mRememberRetrievePw) {
        d->mRetrievePassword =
            KStringHandler::obscure(config()->readEntry("Retrieve Server Password"));
    }
#endif

    KConfigSkeleton::usrRead();
    fillMailDefaults();
}

bool KCalPrefs::usrSave()
{
    KConfigGroup generalConfig(config(), "General");

#if 0
    if (mRememberRetrievePw) {
        config()->writeEntry("Retrieve Server Password",
                             KStringHandler::obscure(d->mRetrievePassword));
    } else {
        config()->deleteEntry("Retrieve Server Password");
    }
#endif

    KConfigGroup defaultCalendarConfig(config(), "Calendar");
    defaultCalendarConfig.writeEntry("Default Calendar", defaultCalendarId());

    return KConfigSkeleton::usrSave();
}

QString KCalPrefs::fullName()
{
    QString tusername;
    if (mEmailControlCenter) {
        KEMailSettings settings;
        tusername = settings.getSetting(KEMailSettings::RealName);
    } else {
        tusername = userName();
    }

    // Quote the username as it might contain commas and other quotable chars.
    tusername = KEmailAddress::quoteNameIfNecessary(tusername);

    QString tname, temail;
    // ignore the return value from extractEmailAddressAndName() because
    // it will always be false since tusername does not contain "@domain".
    KEmailAddress::extractEmailAddressAndName(tusername, temail, tname);
    return tname;
}

QString KCalPrefs::email()
{
    if (mEmailControlCenter) {
        KEMailSettings settings;
        return settings.getSetting(KEMailSettings::EmailAddress);
    } else {
        return userEmail();
    }
}

QStringList KCalPrefs::allEmails()
{
    // Grab emails from the email identities
    QStringList lst = CalendarSupport::identityManager()->allEmails();
    // Add emails configured in korganizer
    lst += mAdditionalMails;
    // Add the email entered as the userEmail here
    lst += email();

    // Warning, this list could contain duplicates.
    return lst;
}

QStringList KCalPrefs::fullEmails()
{
    QStringList fullEmails;

    // Grab emails from the email identities
    KIdentityManagement::IdentityManager *idmanager = CalendarSupport::identityManager();
    QStringList lst = idmanager->identities();

    fullEmails.reserve(1 + mAdditionalMails.count() + lst.count());
    // The user name and email from the config dialog:
    fullEmails << QStringLiteral("%1 <%2>").arg(fullName()).arg(email());

    QStringList::Iterator it;
    KIdentityManagement::IdentityManager::ConstIterator it1;
    for (it1 = idmanager->begin(); it1 != idmanager->end(); ++it1) {
        fullEmails << (*it1).fullEmailAddr();
    }
    // Add emails configured in korganizer
    lst = mAdditionalMails;
    for (it = lst.begin(); it != lst.end(); ++it) {
        fullEmails << QStringLiteral("%1 <%2>").arg(fullName()).arg(*it);
    }

    // Warning, this list could contain duplicates.
    return fullEmails;
}

bool KCalPrefs::thatIsMe(const QString &_email)
{
    // NOTE: this method is called for every created agenda view item,
    // so we need to keep performance in mind

    /* identityManager()->thatIsMe() is quite expensive since it does parsing of
       _email in a way which is unnecessarily complex for what we can have here,
       so we do that ourselves. This makes sense since this

    if ( Akonadi::identityManager()->thatIsMe( _email ) ) {
      return true;
    }
    */

    // in case email contains a full name, strip it out.
    // the below is the simpler but slower version of the following code:
    // const QString email = KPIM::getEmailAddress( _email );
    const QByteArray tmp = _email.toUtf8();
    const char *cursor = tmp.constData();
    const char *end = tmp.data() + tmp.length();
    KMime::Types::Mailbox mbox;
    KMime::HeaderParsing::parseMailbox(cursor, end, mbox);
    const QString email = mbox.addrSpec().asString();

    if (this->email() == email) {
        return true;
    }

    CalendarSupport::IdentityManager::ConstIterator it;
    for (it = CalendarSupport::identityManager()->begin();
            it != CalendarSupport::identityManager()->end(); ++it) {
        if ((*it).matchesEmailAddress(email)) {
            return true;
        }
    }

    if (mAdditionalMails.contains(email)) {
        return true;
    }

    return false;
}

void KCalPrefs::setCategoryColor(const QString &cat, const QColor &color)
{
    Akonadi::Tag tag = d->mTagCache.getTagByGid(cat.toUtf8());
    Akonadi::TagAttribute *attr = tag.attribute<Akonadi::TagAttribute>(Akonadi::Tag::AddIfMissing);
    attr->setBackgroundColor(color);
    new Akonadi::TagModifyJob(tag);
}

QColor KCalPrefs::categoryColor(const QString &cat) const
{
    QColor color;

    if (!cat.isEmpty()) {
        const Akonadi::Tag &tag = d->mTagCache.getTagByGid(cat.toUtf8());
        if (Akonadi::TagAttribute *attr = tag.attribute<Akonadi::TagAttribute>()) {
            color = attr->backgroundColor();
        }
    }

    return color.isValid() ? color : d->mDefaultCategoryColor;
}

bool KCalPrefs::hasCategoryColor(const QString &cat) const
{
    return (categoryColor(cat) != d->mDefaultCategoryColor);
}

void KCalPrefs::setDayBegins(const QDateTime &dateTime)
{
    d->mDayBegins = dateTime;
}

QDateTime KCalPrefs::dayBegins() const
{
    return d->mDayBegins;
}

