/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "sendtoblogdialog.h"
#include "ui_sendtoblogbase.h"
#include <kdebug.h>
#include <KLocalizedString>

class SendToBlogDialog::Private
{
public:
    Private()
        : mIsPrivate(false),
          mIsNew(false)
    {
    }

    Ui::SendToBlogBase ui;
    bool mIsPrivate;
    bool mIsNew;
};
SendToBlogDialog::SendToBlogDialog( bool isNew, bool isPrivate, QWidget *parent )
    : KDialog(parent), d(new Private)
{
    QWidget *dialog = new QWidget( this );
    d->ui.setupUi( dialog );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    this->setMainWidget( dialog );
    setWindowTitle( i18n( "Submitting as..." ) );
    if ( isNew ) {
        d->ui.pubAsModify->setEnabled( false );
        d->ui.pubAsNewPost->setChecked( true );
    } else {
        d->ui.pubAsModify->setChecked( true );
    }
    if ( isPrivate )
        d->ui.saveDraft->setChecked(true);
    d->mIsNew = isNew;
    d->mIsPrivate = isPrivate;
}

SendToBlogDialog::~SendToBlogDialog()
{
    delete d;
}

bool SendToBlogDialog::isPrivate() const
{
    return d->mIsPrivate;
}

bool SendToBlogDialog::isNew() const
{
    return d->mIsNew;
}

void SendToBlogDialog::accept()
{
    d->mIsPrivate = d->ui.saveDraft->isChecked();
    d->mIsNew = !d->ui.pubAsModify->isChecked();
    KDialog::accept();
}

