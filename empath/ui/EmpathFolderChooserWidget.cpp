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

// Qt includes
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qpixmap.h>

// KDE includes
#include <klocale.h>
#include <kiconloader.h>

// Local includes
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathFolderChooserWidget.h"
#include "EmpathFolderChooserDialog.h"
#include "EmpathMailboxList.h"
#include "EmpathMailbox.h"

EmpathFolderChooserWidget::EmpathFolderChooserWidget(QWidget * parent)
    :    QWidget(parent, "FolderChooserWidget")
{
    le_folderName_      = new QLineEdit(this, "l_folderName_");
    pb_selectFolder_    = new QPushButton(this, "pb_selectFolder_");

    pb_selectFolder_->setPixmap(BarIcon("misc-browse"));
    pb_selectFolder_->setFixedWidth(pb_selectFolder_->sizeHint().height());
    
    QObject::connect(pb_selectFolder_, SIGNAL(clicked()),
            this, SLOT(s_browse()));
    
    le_folderName_->setText("<" + i18n("no folder selected") + ">");

    QHBoxLayout * layout = new QHBoxLayout(this);

    layout->addWidget(le_folderName_);
    layout->addWidget(pb_selectFolder_);
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
