#ifndef KADDRESSBOOKDETAILEDVIEW_H
#define KADDRESSBOOKDETAILEDVIEW_H


/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mirko Boehm <mirko@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qstring.h>
#include "../kaddressbookview.h"
#include "kab3mainwidget.h"

class KConfig;
/** This class incorporates the KAB MkIII detailed view into
    kaddressbook.
*/

class KAddressBookDetailedView : public KAddressBookView
{
    Q_OBJECT

public:
    KAddressBookDetailedView(KABC::AddressBook *doc,
                              QWidget *parent,
                              const char *name);
    virtual ~KAddressBookDetailedView();

    virtual QStringList selectedUids();
    virtual QString type() const { return "DetailedView"; }

    virtual void incrementalSearch(const QString &value,
                                   const QString &field);
    void init(KConfig*);
protected:
    void resizeEvent(QResizeEvent*);

signals:
    void selected(const QString &uid);
public slots:
    void refresh(QString uid = QString::null);
    void setSelected(QString uid = QString::null, bool selected = true);
    void slotAddresseeSelected(const QString& uid);
    void slotModified();
private:
    Kab3MainWidget *mWidget;
    KABC::AddressBook *mDocument;
};


#endif // KADDRESSBOOKDETAILEDVIEW_H
