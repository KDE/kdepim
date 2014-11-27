/*
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
#ifndef EVENTVIEWS_PREFS_H
#define EVENTVIEWS_PREFS_H

#include "eventviews_export.h"
#include "eventview.h"

#include <KConfigSkeleton>
#include <KDateTime>

namespace boost {
  template <typename T> class shared_ptr;
}

namespace EventViews {

class EVENTVIEWS_EXPORT Prefs
{
  public:
    /**
      Creates an instance of Prefs with just base config
    */
    Prefs();

    /**
      Creates an instance of Prefs with base config and application override config

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

    void setAgendaHolidaysBackgroundColor( const QColor &color ) const;
    QColor agendaHolidaysBackgroundColor() const;

    void setAgendaViewColors( int colors );
    int agendaViewColors() const;

    void setAgendaViewFont( const QFont &font );
    QFont agendaViewFont() const;

    void setMonthViewFont( const QFont &font );
    QFont monthViewFont() const;

    QColor monthGridBackgroundColor() const;
    void setMonthGridBackgroundColor( const QColor &color );

    QColor monthGridWorkHoursBackgroundColor() const;
    void monthGridWorkHoursBackgroundColor( const QColor &color );

    void setMonthViewColors( int colors ) const;
    int monthViewColors() const;

    bool enableMonthItemIcons() const;
    void setEnableMonthItemIcons( bool enable );

    bool showTimeInMonthView() const;
    void setShowTimeInMonthView( bool show );

    bool showTodosMonthView() const;
    void setShowTodosMonthView( bool show );

    bool showJournalsMonthView() const;
    void setShowJournalsMonthView( bool show );

    bool fullViewMonth() const;
    void setFullViewMonth( bool fullView );

    bool sortCompletedTodosSeparately() const;
    void setSortCompletedTodosSeparately( bool sort );

    void setEnableToolTips( bool enable );
    bool enableToolTips() const;

    void setShowTodosAgendaView( bool show );
    bool showTodosAgendaView() const;

    void setAgendaTimeLabelsFont( const QFont &font );
    QFont agendaTimeLabelsFont() const;

    KConfigSkeleton::ItemFont *fontItem( const QString &name ) const;

    void setResourceColor ( const QString &, const QColor & );
    QColor resourceColor( const QString & );

    void setTimeSpec( const KDateTime::Spec &spec );
    KDateTime::Spec timeSpec() const;

    QStringList timeScaleTimezones() const;
    void setTimeScaleTimezones( const QStringList &list );

    QStringList selectedPlugins() const;
    void setSelectedPlugins( const QStringList &);

    QStringList decorationsAtAgendaViewTop() const;
    void setDecorationsAtAgendaViewTop( const QStringList & );

    QStringList decorationsAtAgendaViewBottom() const;
    void setDecorationsAtAgendaViewBottom( const QStringList & );

    bool colorAgendaBusyDays() const;
    void setColorAgendaBusyDays( bool enable );

    bool colorMonthBusyDays() const;
    void setColorMonthBusyDays( bool enable );

    QColor viewBgBusyColor() const;
    void setViewBgBusyColor( const QColor & );

    QColor holidayColor() const;
    void setHolidayColor( const QColor &color );

    QColor agendaViewBackgroundColor() const;
    void setAgendaViewBackgroundColor( const QColor &color );

    QColor workingHoursColor() const;
    void setWorkingHoursColor( const QColor &color );

    QColor todoDueTodayColor() const;
    void setTodoDueTodayColor( const QColor &color );

    QColor todoOverdueColor() const;
    void setTodoOverdueColor( const QColor &color );

    QSet<EventViews::EventView::ItemIcon> agendaViewIcons() const;
    void setAgendaViewIcons( const QSet<EventViews::EventView::ItemIcon> &icons );

    QSet<EventViews::EventView::ItemIcon> monthViewIcons() const;
    void setMonthViewIcons( const QSet<EventViews::EventView::ItemIcon> &icons );

    void setFlatListTodo( bool );
    bool flatListTodo() const;

    void setFullViewTodo( bool );
    bool fullViewTodo() const;

    bool enableTodoQuickSearch() const;
    void setEnableTodoQuickSearch( bool enable );

    bool enableQuickTodo() const;
    void setEnableQuickTodo( bool enable );

    bool highlightTodos() const;
    void setHighlightTodos( bool );

    KConfig *config() const;

    void createNewColor(QColor &defColor, int seed);
  private:
    class Private;
    Private *const d;
};

typedef boost::shared_ptr<Prefs> PrefsPtr;

}

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
