/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Kevin Krammer, krake@kdab.com

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
#ifndef PREFS_H
#define PREFS_H

#include "eventviews_export.h"

#include <KDateTime>

namespace Akonadi
{
  class Collection;
}

namespace boost {
  template <typename T> class shared_ptr;
}

class KCoreConfigSkeleton;
class QColor;
class QFont;
class QStringList;

namespace EventViews
{

class EVENTVIEWS_EXPORT Prefs
{
  public:

    /** Creates an instance of Prefs with just base config
    */
    Prefs();

    /** Creates an instance of Prefs with base config and application override config

        The passed @p appConfig will be queried for matching items whenever one of the
        accessors is called. If one is found it is used for setting/getting the value
        otherwise the one from the eventviews base config is used.
    */
    explicit Prefs( KCoreConfigSkeleton *appConfig );

    ~Prefs();

    void readConfig();

    void writeConfig();

  public:
    void setMarcusBainsShowSeconds( bool showSeconds );
    bool marcusBainsShowSeconds() const;

    void setAgendaMarcusBainsLineLineColor( const QColor &color );
    QColor agendaMarcusBainsLineLineColor() const;

    void setMarcusBainsEnabled( bool enabled );
    bool marcusBainsEnabled() const;

    void setAgendaMarcusBainsLineFont( const QFont &font );
    QFont agendaMarcusBainsLineFont() const;

    void setHourSize( int size );
    int hourSize() const;

    void setDayBegins( const QDateTime &dateTime );
    QDateTime dayBegins() const;

    void setWorkingHoursStart( const QDateTime &dateTime );
    QDateTime workingHoursStart() const;

    void setWorkingHoursEnd( const QDateTime &dateTime );
    QDateTime workingHoursEnd() const;

    void setSelectionStartsEditor( bool startEditor );
    bool selectionStartsEditor() const;

    void setAgendaGridWorkHoursBackgroundColor( const QColor &color );
    QColor agendaGridWorkHoursBackgroundColor() const;

    void setAgendaGridHighlightColor( const QColor &color );
    QColor agendaGridHighlightColor() const;

    void setAgendaGridBackgroundColor( const QColor &color );
    QColor agendaGridBackgroundColor() const;

    void setEnableAgendaItemIcons( bool enable );
    bool enableAgendaItemIcons() const;

    void setTodosUseCategoryColors( bool useColors );
    bool todosUseCategoryColors() const;

    void setAgendaCalendarItemsToDosOverdueBackgroundColor( const QColor &color );
    QColor agendaCalendarItemsToDosOverdueBackgroundColor() const;

    void setAgendaCalendarItemsToDosDueTodayBackgroundColor( const QColor &color );
    QColor agendaCalendarItemsToDosDueTodayBackgroundColor() const;

    void setUnsetCategoryColor( const QColor &color );
    QColor unsetCategoryColor() const;

    void setAgendaViewColors( int colors );
    int agendaViewColors() const;

    void setAgendaViewFont( const QFont &font );
    QFont agendaViewFont() const;

    void setMonthViewFont( const QFont &font );
    QFont monthViewFont() const;

    void setEnableToolTips( bool enable );
    bool enableToolTips() const;

    void setDefaultDuration( const QDateTime &dateTime );
    QDateTime defaultDuration() const;

    void setShowTodosAgendaView( bool show );
    bool showTodosAgendaView() const;

    void setAgendaTimeLabelsFont( const QFont &font );
    QFont agendaTimeLabelsFont() const;

    void setWorkWeekMask( int mask );
    int workWeekMask() const;

    void setExcludeHolidays( bool exclude );
    bool excludeHolidays() const;

  public:
    // preferences data
    void setFullName( const QString & );
    QString fullName() const;
    void setEmail( const QString & );
    QString email() const;
    /// Returns all email addresses for the user.
    QStringList allEmails() const;
    /// Returns all email addresses together with the full username for the user.
    QStringList fullEmails() const;
    /// Return true if the given email belongs to the user
    bool thatIsMe( const QString &email ) const;

    void setCategoryColor( const QString &cat, const QColor &color );
    QColor categoryColor( const QString &cat ) const;
    bool hasCategoryColor( const QString &cat ) const;

    void setResourceColor ( const QString &, const QColor & );
    QColor resourceColor( const QString & );

    void setTimeSpec( const KDateTime::Spec &spec );
    KDateTime::Spec timeSpec() const;

    void setHtmlExportFile( const QString &fileName );
    QString htmlExportFile() const;

    // Groupware passwords
    void setPublishPassword( const QString &password );
    QString publishPassword() const;

    void setRetrievePassword( const QString &password );
    QString retrievePassword() const;

    QStringList timeScaleTimezones() const;
    void setTimeScaleTimezones( const QStringList &list );

    QString defaultCalendar() const;
    void setDefaultCollection( const Akonadi::Collection& );
    Akonadi::Collection defaultCollection() const;

  private:
    class Private;
    Private *const d;
};

typedef boost::shared_ptr<Prefs> PrefsPtr;

} // namespace EventViews

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
