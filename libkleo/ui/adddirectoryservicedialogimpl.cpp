/*
    adddirectoryservicedialogimpl.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/


#include "adddirectoryservicedialogimpl.h"

#include <QValidator>
#include <klineedit.h>

/*
 *  Constructs a AddDirectoryServiceDialogImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AddDirectoryServiceDialogImpl::AddDirectoryServiceDialogImpl( QWidget* parent,  const char* name, bool modal, Qt::WindowFlags fl )
    : QDialog(parent, fl), AddDirectoryServiceDialog()
{
    setObjectName( QLatin1String(name) );
    setModal( modal );
    setupUi(this);
    portED->setValidator( new QIntValidator( 0, 65535, portED ) );
    connect(PushButton3, SIGNAL(clicked()), SLOT(slotAccept()));
    connect(PushButton4, SIGNAL(clicked()), SLOT(slotCancel()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
AddDirectoryServiceDialogImpl::~AddDirectoryServiceDialogImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

void AddDirectoryServiceDialogImpl::slotAccept()
{
        accept();
}

void AddDirectoryServiceDialogImpl::slotCancel()
{
        reject();
}


