/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
*/

#ifdef __GNUG__
# pragma implementation "EmpathMessageStructureWidget.h"
#endif

// Qt includes
#include <qfile.h>
#include <qdatastream.h>

// KDE includes
#include <klocale.h>
#include <kapp.h>
#include <kfiledialog.h>
#include <kmsgbox.h>

// Local includes
#include "EmpathMessageStructureWidget.h"
#include "EmpathMessageStructureItem.h"
#include "EmpathUIUtils.h"

EmpathMessageStructureWidget::EmpathMessageStructureWidget
    (QWidget * parent, const char * name)
    :    QListView(parent, name)
{
    setCaption(i18n("Message Structure - ") + kapp->getCaption());
    
    addColumn(i18n("Type"));
    addColumn(i18n("Subtype"));
    addColumn(i18n("Size"));
    
    setAllColumnsShowFocus(true);
    setRootIsDecorated(true);
    setSorting(-1); // Don't sort this.

    QObject::connect(this, SIGNAL(currentChanged(QListViewItem *)),
            this, SLOT(s_currentChanged(QListViewItem *)));
    
    QObject::connect(
        this,
        SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
        this,
        SLOT(s_rightButtonPressed(QListViewItem *, const QPoint &, int)));
    
    popup_.insertItem(empathIcon("mini-save.png"),
            i18n("Save &As"),
            this, SLOT(s_saveAs()));
    
    popup_.insertItem(
            i18n("Open with..."),
            this, SLOT(s_openWith()));
    
}

    void
EmpathMessageStructureWidget::setMessage(RMM::RBodyPart & m)
{
    clear();
    
    EmpathMessageStructureItem * i = new EmpathMessageStructureItem(this, m);
    CHECK_PTR(i);
    
    QListIterator<RMM::RBodyPart> it(m.body());
    
    for (; it.current(); ++it) {
        
        EmpathMessageStructureItem * j =
            new EmpathMessageStructureItem(i, *(it.current()));
        CHECK_PTR(j);

        _addChildren(it.current(), j);
    }
}

    void
EmpathMessageStructureWidget::_addChildren(RMM::RBodyPart *p, QListViewItem *i)
{
    QListIterator<RMM::RBodyPart> it(p->body());
    
    for (; it.current(); ++it) {

        EmpathMessageStructureItem * j =
            new EmpathMessageStructureItem((EmpathMessageStructureItem *)i, *p);

        CHECK_PTR(j);

        _addChildren(it.current(), j);
    }
}

    void
EmpathMessageStructureWidget::s_currentChanged(QListViewItem * item)
{
    empathDebug("s_currentChanged() called");
    if (item->depth() == 0) return;
    EmpathMessageStructureItem * i = (EmpathMessageStructureItem *)item;
    emit(partChanged(i->part()));
}

    void
EmpathMessageStructureWidget::s_rightButtonPressed(
    QListViewItem * item, const QPoint &, int)
{
    setSelected(item, true);
    popup_.exec(QCursor::pos());
}

    void
EmpathMessageStructureWidget::s_saveAs()
{
    QString saveFilePath =
        KFileDialog::getSaveFileName(
            QString::null, QString::null, this, i18n("Empath: Save Message").ascii());
    empathDebug(saveFilePath);
    
    if (saveFilePath.isEmpty()) {
        empathDebug("No filename given");
        return;
    }
    
    QFile f(saveFilePath);
    if (!f.open(IO_WriteOnly)) {
        // Warn user file cannot be opened.
        empathDebug("Couldn't open file for writing");
        KMsgBox(this, "Empath",i18n("Sorry I can't write to that file. Please try another filename."), KMsgBox::EXCLAMATION, i18n("OK"));
        return;
    }
    empathDebug("Opened " + saveFilePath + " OK");
    
    QDataStream d(&f);
    
    QListViewItem * i(currentItem());
    
    if (!i) return;
    
    QCString s = ((EmpathMessageStructureItem *)i)->part()->asString();
    
    d.writeRawBytes(s, s.length());

    f.close();
    
}

    void
EmpathMessageStructureWidget::s_openWith()
{
}

// vim:ts=4:sw=4:tw=78
