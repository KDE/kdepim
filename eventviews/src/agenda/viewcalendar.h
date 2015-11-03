/*
 * Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * As a special exception, permission is given to link this program
 * with any edition of Qt, and distribute the resulting executable,
 * without including the source code for Qt in the source distribution.
 */

#ifndef EVENTVIEWS_VIEWCALENDAR_H
#define EVENTVIEWS_VIEWCALENDAR_H

#include "eventviews_export.h"

#include <AkonadiCore/Item>
#include <Akonadi/Calendar/ETMCalendar>
#include <KCalCore/Incidence>

#include <QColor>
#include <QList>

namespace EventViews
{

class AgendaView;

class EVENTVIEWS_EXPORT ViewCalendar
{

public:
    typedef QSharedPointer<ViewCalendar> Ptr;

    virtual ~ViewCalendar();
    virtual bool isValid(const KCalCore::Incidence::Ptr &incidence) const = 0;
    virtual bool isValid(const QString &incidenceIdentifier) const = 0;
    virtual QString displayName(const KCalCore::Incidence::Ptr &incidence) const = 0;

    virtual QColor resourceColor(const KCalCore::Incidence::Ptr &incidence) const = 0;
    virtual QString iconForIncidence(const KCalCore::Incidence::Ptr &incidence) const = 0;

    virtual KCalCore::Calendar::Ptr getCalendar() const = 0;
};

class AkonadiViewCalendar: public ViewCalendar
{
public:
    typedef QSharedPointer<AkonadiViewCalendar> Ptr;

    virtual ~AkonadiViewCalendar();
    bool isValid(const KCalCore::Incidence::Ptr &incidence) const Q_DECL_OVERRIDE;
    bool isValid(const QString &incidenceIdentifier) const Q_DECL_OVERRIDE;
    QString displayName(const KCalCore::Incidence::Ptr &incidence) const Q_DECL_OVERRIDE;

    QColor resourceColor(const KCalCore::Incidence::Ptr &incidence) const Q_DECL_OVERRIDE;
    QString iconForIncidence(const KCalCore::Incidence::Ptr &incidence) const Q_DECL_OVERRIDE;

    Akonadi::Item item(const KCalCore::Incidence::Ptr &incidence) const;

    KCalCore::Calendar::Ptr getCalendar() const Q_DECL_OVERRIDE;

    KDateTime::Spec timeSpec() const;

    Akonadi::ETMCalendar::Ptr mCalendar;
    AgendaView *mAgendaView;
};

class MultiViewCalendar : public ViewCalendar
{
public:
    typedef QSharedPointer<MultiViewCalendar> Ptr;

    virtual ~MultiViewCalendar();
    ViewCalendar::Ptr findCalendar(const KCalCore::Incidence::Ptr &incidence) const;
    ViewCalendar::Ptr findCalendar(const QString &incidenceIdentifier) const;
    bool isValid(const KCalCore::Incidence::Ptr &incidence) const Q_DECL_OVERRIDE;
    bool isValid(const QString &incidenceIdentifier) const Q_DECL_OVERRIDE;
    QString displayName(const KCalCore::Incidence::Ptr &incidence) const Q_DECL_OVERRIDE;

    QColor resourceColor(const KCalCore::Incidence::Ptr &incidence) const Q_DECL_OVERRIDE;
    QString iconForIncidence(const KCalCore::Incidence::Ptr &incidence) const Q_DECL_OVERRIDE;

    Akonadi::Item item(const KCalCore::Incidence::Ptr &incidence) const;

    void addCalendar(const ViewCalendar::Ptr &calendar);
    void setETMCalendar(const Akonadi::ETMCalendar::Ptr &calendar);
    int calendars() const;
    KCalCore::Calendar::Ptr getCalendar() const Q_DECL_OVERRIDE;
    KCalCore::Incidence::List incidences() const;

    AgendaView *mAgendaView;
    AkonadiViewCalendar::Ptr mETMCalendar;
    QList<ViewCalendar::Ptr> mSubCalendars;
};

}

#endif
