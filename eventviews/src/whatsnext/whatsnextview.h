/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef CALENDARVIEWS_WHATSNEXTVIEW_H
#define CALENDARVIEWS_WHATSNEXTVIEW_H

#include "eventview.h"

#include <Akonadi/Calendar/IncidenceChanger>
#include <Akonadi/Calendar/ETMCalendar>
#include <QTextBrowser>
#include <kiconloader.h>

namespace EventViews
{

class WhatsNextTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    explicit WhatsNextTextBrowser(QWidget *parent) : QTextBrowser(parent) {}
    /** Reimplemented from QTextBrowser to handle links. */
    void setSource(const QUrl &name) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void showIncidence(const QString &uid);
};

/**
  This class provides a view of the next events and todos
*/
class EVENTVIEWS_EXPORT WhatsNextView : public EventViews::EventView
{
    Q_OBJECT
public:
    explicit WhatsNextView(QWidget *parent = Q_NULLPTR);
    ~WhatsNextView();

    int currentDateCount() const Q_DECL_OVERRIDE;
    Akonadi::Item::List selectedIncidences() const Q_DECL_OVERRIDE
    {
        return Akonadi::Item::List();
    }
    KCalCore::DateList selectedIncidenceDates() const Q_DECL_OVERRIDE
    {
        return KCalCore::DateList();
    }

    bool supportsDateNavigation() const
    {
        return true;
    }

public Q_SLOTS:
    void updateView() Q_DECL_OVERRIDE;
    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth) Q_DECL_OVERRIDE;
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) Q_DECL_OVERRIDE;

    void changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType);

protected:
    void appendEvent(const KCalCore::Incidence::Ptr &,
                     const QDateTime &start = QDateTime(),
                     const QDateTime &end = QDateTime());
    void appendTodo(const KCalCore::Incidence::Ptr &);

private Q_SLOTS:
    void showIncidence(const QString &);

private:
    void createTaskRow(KIconLoader *kil);
    WhatsNextTextBrowser *mView;
    QString mText;
    QDate mStartDate;
    QDate mEndDate;

    Akonadi::Item::List mTodos;
};

}

#endif
