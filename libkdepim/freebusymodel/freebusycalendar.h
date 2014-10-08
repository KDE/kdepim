/*
 * Copyright 2014  Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef FBMODEL_FBCALENDAR_H
#define FBMODEL_FBCALENDAR_H

#include "libkdepim/kdepim_export.h"

#include "freebusyitemmodel.h"

#include <KCalCore/Calendar>
#include <KCalCore/Event>

class KDEPIM_EXPORT FreeBusyCalendar : QObject {
    Q_OBJECT
public:
    explicit FreeBusyCalendar(QObject *parent = 0);

    virtual ~FreeBusyCalendar();
    
    void setModel(FreeBusyItemModel *model);
    FreeBusyItemModel *model() const;
    KCalCore::Calendar::Ptr calendar() const;

private slots:
    void onRowsChanged(const QModelIndex &, const QModelIndex &);
    void onRowsInserted(const QModelIndex &,int,int);
    void onRowsRemoved(const QModelIndex &,int,int);
    void onLayoutChanged();

private:
    FreeBusyItemModel *mModel;
    KCalCore::Calendar::Ptr mCalendar;
    QMap<QModelIndex,KCalCore::Event::Ptr> mFbEvent;
};

#endif // FBMODEL_FBCALENDAR_H
