/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#ifndef PICKSMSCENTER_H
#define PICKSMSCENTER_H

#include <libkmobiletools/kmobiletools_export.h>

#include <kdialog.h>

/**
	@author Marco Gulino <marco@kmobiletools.org>
*/
class Q3ListViewItem;
class PickSMSCenterPrivate;
class KMOBILETOOLS_EXPORT PickSMSCenter : public KDialog
{
Q_OBJECT
public:
    PickSMSCenter(QWidget *parent = 0);

    ~PickSMSCenter();
    const QString smsCenter();
    static QString smsCenterName(const QString &smsCenter);
    private:
        PickSMSCenterPrivate *const d;

public Q_SLOTS:
    void initList();
    void clicked ( Q3ListViewItem * item );
    void doubleClicked( Q3ListViewItem *, const QPoint &, int );
};

#endif
