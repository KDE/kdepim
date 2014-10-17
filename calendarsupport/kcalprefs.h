/*
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef CALENDARSUPPORT_KCALPREFS_H
#define CALENDARSUPPORT_KCALPREFS_H

#include "calendarsupport_export.h"
#include "kcalprefs_base.h"

#include <Akonadi/Entity>
#include <Akonadi/Tag>

#include <KDateTime>

namespace Akonadi {
    class Monitor;
};
class KJob;

namespace CalendarSupport {

class CALENDARSUPPORT_EXPORT KCalPrefs : public KCalPrefsBase
{
  public:
    /** Constructor disabled for public. Use instance() to create a KCalPrefs
    object. */
    KCalPrefs();
    virtual ~KCalPrefs();

    /** Get instance of KCalPrefs. It is made sure that there is only one
    instance. */
    static KCalPrefs *instance();

    /** Set preferences to default values */
    void usrSetDefaults();

    /** Read preferences from config file */
    void usrReadConfig();

    /** Write preferences to config file */
    void usrWriteConfig();

  protected:
    void setTimeZoneDefault();

    /** Fill empty mail fields with default values. */
    void fillMailDefaults();

  public:
    // preferences data
    void setFullName( const QString & );
    QString fullName();
    void setEmail( const QString & );
    QString email();
    /// Returns all email addresses for the user.
    QStringList allEmails();
    /// Returns all email addresses together with the full username for the user.
    QStringList fullEmails();
    /// Return true if the given email belongs to the user
    bool thatIsMe( const QString &email );

    void setTimeSpec( const KDateTime::Spec &spec );
    KDateTime::Spec timeSpec();

    QString mailTransport() const;

    Akonadi::Entity::Id defaultCalendarId() const;
    void setDefaultCalendarId( const Akonadi::Entity::Id );

    void setCategoryColor( const QString &cat, const QColor &color );
    QColor categoryColor( const QString &cat ) const;
    bool hasCategoryColor( const QString &cat ) const;

    void setDayBegins( const QDateTime &dateTime );
    QDateTime dayBegins() const;

  private:
    class Private;
    Private *const d;
};

/**
 * A tag cache
 */
class TagCache : public QObject
{
  Q_OBJECT
public:
    TagCache();
    Akonadi::Tag getTagByGid(const QByteArray &gid);

private Q_SLOTS:
    void onTagAdded(const Akonadi::Tag &);
    void onTagChanged(const Akonadi::Tag &);
    void onTagRemoved(const Akonadi::Tag &);
    void onTagsFetched(KJob*);

private:
    void retrieveTags();

    QHash<Akonadi::Tag::Id, Akonadi::Tag> mCache;
    QHash<QByteArray, Akonadi::Tag::Id> mGidCache;
    Akonadi::Monitor *mMonitor;
};

}

#endif
