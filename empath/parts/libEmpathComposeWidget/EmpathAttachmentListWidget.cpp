/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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

// KDE includes
#include <kaction.h>
#include <klocale.h>
#include <kconfig.h>

// Local includes
#include "EmpathAttachmentListItem.h"
#include "EmpathAttachmentListWidget.h"
#include "EmpathAttachmentEditDialog.h"

EmpathAttachmentListWidget::EmpathAttachmentListWidget(QWidget * parent)
    :    EmpathListView(parent, "EmpathAttachmentListWidget")
{
    setAllColumnsShowFocus(true);

    addColumn(i18n("Filename"));
    addColumn(i18n("Type"));
    addColumn(i18n("Subtype"));
    addColumn(i18n("Character set"));
    addColumn(i18n("Encoding"));
    addColumn(i18n("Description"));

    _initActions();
}

EmpathAttachmentListWidget::~EmpathAttachmentListWidget()
{
    // Empty.
}

    void
EmpathAttachmentListWidget::use(EmpathAttachmentSpecList l)
{
    clear();

    for (EmpathAttachmentSpecList::Iterator it = l.begin(); it != l.end(); ++it)
        new EmpathAttachmentListItem(this, *it);
}

    void
EmpathAttachmentListWidget::s_attachmentAdd()
{
    EmpathAttachmentEditDialog * e =
        new EmpathAttachmentEditDialog(this, "attachmentEditDialog");
    
    e->browse();
    
    if (e->exec() == QDialog::Accepted)
        new EmpathAttachmentListItem(this, e->spec());
}

    void
EmpathAttachmentListWidget::s_attachmentEdit()
{
    QListViewItem * item(currentItem());
    
    if (item == 0)
        return;
    
    EmpathAttachmentListItem * i = (EmpathAttachmentListItem *)item;
    
    EmpathAttachmentEditDialog * e =
        new EmpathAttachmentEditDialog(this, "attachmentEditDialog");
    
    e->setSpec(i->spec());
    
    if (e->exec() == QDialog::Accepted)
        i->setSpec(e->spec());
}
    
    void
EmpathAttachmentListWidget::s_attachmentRemove()
{
    if (currentItem() != 0)
        QListView::removeItem(currentItem());
}

    EmpathAttachmentSpecList
EmpathAttachmentListWidget::attachments()
{
    EmpathAttachmentSpecList l;

    if (childCount() != 0) {

        QListViewItem * i = firstChild();

        while (i != 0) {

            l.append((static_cast<EmpathAttachmentListItem *>(i))->spec());
            i = i->nextSibling();
        }
    }

    return l;
}

    void
EmpathAttachmentListWidget::_initActions()
{
    actionCollection_ = new QActionCollection(this, "actionCollection");

    ac_attachmentAdd = new KAction(i18n("&Add"), 0, 
                    this, SLOT(s_attachmentAdd()), actionCollection(), "attachmentAdd");
    ac_attachmentEdit = new KAction(i18n("&Edit"), 0, 
                    this, SLOT(s_attachmentEdit()), actionCollection(), "attachmentEdit");
    ac_attachmentRemove = new KAction(i18n("&Remove"), 0, 
                    this, SLOT(s_attachmentRemove()), actionCollection(), "attachmentRemove");
}

// vim:ts=4:sw=4:tw=78
