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
# pragma implementation "EmpathAttachmentListWidget.h"
#endif

// KDE includes
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>
#include <kfiledialog.h>

// Local includes
#include "EmpathAttachmentListWidget.h"
#include "EmpathAttachmentEditDialog.h"
#include "EmpathConfig.h"
#include "EmpathUIUtils.h"
#include "Empath.h"

EmpathAttachmentListWidget::EmpathAttachmentListWidget(
        QWidget * parent,
        const char * name)
    :    QListView(parent, name)
{
    setAllColumnsShowFocus(true);

    addColumn(i18n("Filename"));
    addColumn(i18n("Type"));
    addColumn(i18n("Subtype"));
    addColumn(i18n("Character set"));
    addColumn(i18n("Encoding"));
    addColumn(i18n("Description"));
}

EmpathAttachmentListWidget::~EmpathAttachmentListWidget()
{
    empathDebug("dtor");
}

    void
EmpathAttachmentListWidget::use(const RMM::RMessage &)
{
}

    void
EmpathAttachmentListWidget::addAttachment()
{
    empathDebug("addAttachment() called");

    EmpathAttachmentEditDialog * e =
        new EmpathAttachmentEditDialog(this, "attachmentEditDialog");
    
    e->browse();
    
    if (e->exec() != QDialog::Accepted)
        return;
    
    new EmpathAttachmentListItem(this, e->spec());
}

    void
EmpathAttachmentListWidget::editAttachment()
{
    empathDebug("editAttachment() called");
    
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
EmpathAttachmentListWidget::removeAttachment()
{
    empathDebug("removeAttachment() called");
    
    if (currentItem() != 0)
        QListView::removeItem(currentItem());
}

// vim:ts=4:sw=4:tw=78
