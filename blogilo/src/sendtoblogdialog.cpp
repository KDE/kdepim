/***************************************************************************
*   This file is part of the Bilbo Blogger.                               *
*   Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>     *
*   Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#include "sendtoblogdialog.h"
#include <kdebug.h>

SendToBlogDialog::SendToBlogDialog( bool isNew, bool isPrivate, QWidget *parent )
    : KDialog(parent)
{
    kDebug()<<isNew<<isPrivate;
    QWidget *dialog = new QWidget( this );
    ui.setupUi( dialog );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    this->setMainWidget( dialog );
    setWindowTitle( i18n( "Submitting as ..." ) );
    if( isNew ) {
        ui.pubAsModify->setEnabled( false );
        ui.pubAsNewPost->setChecked( true );
    } else {
        ui.pubAsModify->setChecked( true );
    }
    if( isPrivate )
        ui.saveDraft->setChecked(true);
    mIsNew = isNew;
    mIsPrivate = isPrivate;
}

SendToBlogDialog::~SendToBlogDialog()
{}

bool SendToBlogDialog::isPrivate()
{
    return mIsPrivate;
}

bool SendToBlogDialog::isNew()
{
    return mIsNew;
}

void SendToBlogDialog::accept()
{
    if(ui.saveDraft->isChecked()) {
        mIsPrivate = true;
    } else {
        mIsPrivate = false;
    }
    if(ui.pubAsModify->isChecked()) {
        mIsNew = false;
    } else {
        mIsNew = true;
    }
    KDialog::accept();
}

#include "sendtoblogdialog.moc"