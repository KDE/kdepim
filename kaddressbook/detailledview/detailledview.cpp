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


#include "detailledview.h"

KAddressBookDetailedView::KAddressBookDetailedView(KABC::AddressBook *doc,
                                                     QWidget *parent,
                                                     const char *name)
    : KAddressBookView(doc, parent, name)
{
    mWidget=new Kab3MainWidget(doc, this);
    if(mWidget)
    {
        connect(mWidget, SIGNAL(databaseModified()), SLOT(slotModified()));
        connect(mWidget, SIGNAL(selected(const QString&)),
                SLOT(slotAddresseeSelected(const QString&)));
    }
    mDocument=doc;
}

KAddressBookDetailedView::~KAddressBookDetailedView()
{
}

void KAddressBookDetailedView::resizeEvent(QResizeEvent*)
{
    mWidget->setGeometry(0, 0, width(), height());
}

QStringList KAddressBookDetailedView::selectedUids()
{
    return mWidget->selectedUids();
}

void KAddressBookDetailedView::incrementalSearch(const QString &/* value */,
                                                  const QString &/* field */)
{
}

void KAddressBookDetailedView::refresh(QString)
{
    mWidget->slotAddressBookChanged(mDocument);
}


void KAddressBookDetailedView::setSelected(QString uid, bool selected)
{
    mWidget->setSelected(uid, selected);
}

void KAddressBookDetailedView::slotAddresseeSelected(const QString& uid)
{
    emit(selected(uid));
}

void KAddressBookDetailedView::slotModified()
{
    emit(modified());
}

void KAddressBookDetailedView::init(KConfig* config)
{
    mWidget->init(config);
}

#include "detailledview.moc"
