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
# pragma implementation "EmpathFolderChooserWidget.h"
#endif

#include <qpixmap.h>

// KDE includes
#include <klocale.h>

// Local includes
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathFolderChooserWidget.h"
#include "EmpathFolderChooserDialog.h"
#include "EmpathMailboxList.h"
#include "EmpathMailbox.h"
#include "EmpathUIUtils.h"

EmpathFolderChooserWidget::EmpathFolderChooserWidget(QWidget * parent)
    :    QHBox(parent, "FolderChooserWidget")
{
    le_folderName_      = new QLineEdit(this, "l_folderName_");
    pb_selectFolder_    = new QPushButton(this, "pb_selectFolder_");

    pb_selectFolder_->setPixmap(empathIcon("misc-browse"));
    pb_selectFolder_->setFixedWidth(pb_selectFolder_->sizeHint().height());
    
    QObject::connect(pb_selectFolder_, SIGNAL(clicked()),
            this, SLOT(s_browse()));
    
    le_folderName_->setText("<" + i18n("no folder selected") + ">");
}

EmpathFolderChooserWidget::~EmpathFolderChooserWidget()
{
    // Empty.
}

    EmpathURL
EmpathFolderChooserWidget::url() const
{
    return url_;
}

    void
EmpathFolderChooserWidget::setURL(const EmpathURL & url)
{
    url_ = url;
    le_folderName_->setText(url_.mailboxName() + "/" + url_.folderPath());
}

    void
EmpathFolderChooserWidget::s_browse()
{
    EmpathFolderChooserDialog fcd(this);
    
    if (!fcd.exec())
        return;

    url_ = fcd.selected();
    le_folderName_->setText(url_.mailboxName() + "/" + url_.folderPath());
}

// vim:ts=4:sw=4:tw=78
