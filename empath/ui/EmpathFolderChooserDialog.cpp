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

#ifdef __GNUG__
# pragma implementation "EmpathFolderChooserDialog.h"
#endif

// Qt includes
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>

// KDE includes
#include <klocale.h>
#include <kapp.h>
#include <kbuttonbox.h>

// Local includes
#include "EmpathFolderChooserDialog.h"
#include "EmpathFolderChooserWidget.h"
#include "EmpathSeparatorWidget.h"
#include "EmpathDefines.h"
#include "EmpathFolder.h"
#include "EmpathFolderWidget.h"
#include "EmpathUIUtils.h"

EmpathFolderChooserDialog::EmpathFolderChooserDialog(QWidget * parent)
    :    QDialog(parent, "FolderChooserDialog", true)
{
    setCaption(i18n("Folder Chooser"));

    folderWidget_ = new EmpathFolderWidget(this);
    KButtonBox * buttonBox  = new KButtonBox(this);

    QPushButton * pb_help   = buttonBox->addButton(i18n("&Help"));    
    buttonBox->addStretch();
    QPushButton * pb_OK     = buttonBox->addButton(i18n("&OK"));
    QPushButton * pb_cancel = buttonBox->addButton(i18n("&Cancel"));
    
    buttonBox->layout();

    QObject::connect(pb_OK,     SIGNAL(clicked()), SLOT(s_OK())); 
    QObject::connect(pb_cancel, SIGNAL(clicked()), SLOT(s_cancel()));
    QObject::connect(pb_help,   SIGNAL(clicked()), SLOT(s_help()));

    QVBoxLayout * layout = new QVBoxLayout(this, dialogSpace);

    layout->addWidget(folderWidget_);
    layout->addStretch(10);
    layout->addWidget(new EmpathSeparatorWidget(this));
    layout->addWidget(buttonBox);
}

EmpathFolderChooserDialog::~EmpathFolderChooserDialog()
{
    // Empty.
}

    void
EmpathFolderChooserDialog::s_OK()
{
    accept();
}

    void
EmpathFolderChooserDialog::s_cancel()
{
    reject();
}

    void
EmpathFolderChooserDialog::s_help()
{
    // STUB
}

    EmpathURL
EmpathFolderChooserDialog::selected() const
{
    return folderWidget_->selected();
}

// vim:ts=4:sw=4:tw=78
