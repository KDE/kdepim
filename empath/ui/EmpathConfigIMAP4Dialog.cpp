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
# pragma implementation "EmpathConfigIMAP4Dialog.h"
#endif

// Qt includes
#include <qlabel.h>
#include <qlayout.h>

// KDE includes
#include <kbuttonbox.h>
#include <klocale.h>

// Local includes
#include "EmpathUIUtils.h"
#include "EmpathSeparatorWidget.h"
#include "EmpathConfigIMAP4Dialog.h"
#include "EmpathMailboxIMAP4.h"
 
bool EmpathConfigIMAP4Dialog::exists_ = false;

    void
EmpathConfigIMAP4Dialog::create(const EmpathURL & url, QWidget * parent)
{
    if (exists_) return;
    exists_ = true;
    EmpathConfigIMAP4Dialog * d = new EmpathConfigIMAP4Dialog(url, parent);
    CHECK_PTR(d);
    d->show();
    d->loadData();
}
        
EmpathConfigIMAP4Dialog::EmpathConfigIMAP4Dialog
    (const EmpathURL & url, QWidget * parent)
    :   KDialog(parent, "ConfigIMAP4Dialog", true),
        url_(url)
{
    KButtonBox * buttonBox = new KButtonBox(this);

    // Widgets

    // Bottom button group
    pb_OK_      = buttonBox->addButton(i18n("&OK"));
    pb_Cancel_  = buttonBox->addButton(i18n("&Cancel"));
    pb_Help_    = buttonBox->addButton(i18n("&Help"));    

    QVBoxLayout * layout = new QVBoxLayout(this, spacingHint());

    QLabel * l_notImp  =
        new QLabel(i18n("Sorry not implemented yet"), this, "l_notImp");

    layout->addWidget(l_notImp);

    layout->addWidget(new EmpathSeparatorWidget(this));

    layout->addWidget(buttonBox);

    // Layouts
    fillInSavedData();
}

EmpathConfigIMAP4Dialog::~EmpathConfigIMAP4Dialog()
{
    exists_ = false;
}

    void
EmpathConfigIMAP4Dialog::s_OK()
{
    // STUB
    accept();
}

    void
EmpathConfigIMAP4Dialog::fillInSavedData()
{
    // STUB
}

    void
EmpathConfigIMAP4Dialog::s_Cancel()
{
    // STUB
    reject();
}

    void
EmpathConfigIMAP4Dialog::s_Help()
{
    // STUB
}

    void
EmpathConfigIMAP4Dialog::loadData()
{
    // STUB
}

    void
EmpathConfigIMAP4Dialog::saveData()
{
    // STUB
}

// vim:ts=4:sw=4:tw=78
