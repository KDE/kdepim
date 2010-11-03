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

#include "addimagedialog.h"

#ifdef WIN32
#include <QFileDialog>
#else
#include <KFileDialog>
#endif

#include <kmessagebox.h>
#include <kdebug.h>

#include "bilbomedia.h"

AddImageDialog::AddImageDialog(QWidget* parent): AddMediaDialog(parent)
{
    editFrame = new QFrame(this);
    editFrame->setFrameShape(QFrame::StyledPanel);
    editFrame->setFrameShadow(QFrame::Raised);

    editImageWidgetUi.setupUi( editFrame );
    ui.verticalLayout->addWidget( editFrame );

    ui.radiobtnRemoteUrl->setEnabled( true );
    this->setWindowTitle( i18n( "Add Image" ) );

//     QStringList mimeFilter;
//     mimeFilter << "image/gif" << "image/jpeg" << "image/png" ;
//     ui.kurlreqMediaUrl->fileDialog()->setMimeFilter( mimeFilter );
}


AddImageDialog::~AddImageDialog()
{
    kDebug();
}

void AddImageDialog::slotSelectLocalFile()
{
    QString path;
#ifdef WIN32
    path = QFileDialog::getOpenFileName( this, i18n("Choose a file"), QString(), i18n("Images (*.png *.gif *.jpg)" ) );//krazy:exclude=qclasses KFileDialog has problem on WIN32 now
#else
    path = KFileDialog::getImageOpenUrl( KUrl(), this, i18n("Choose a file") ).path();
#endif
    ui.urlReqLineEdit->setText(path);
}

void AddImageDialog::slotButtonClicked(int button)
{
    kDebug();
    _selectedMedia["width"] = editImageWidgetUi.spinboxWidth->value();
    _selectedMedia["height"] = editImageWidgetUi.spinboxHeight->value();
    _selectedMedia["title"] = editImageWidgetUi.txtTitle->text();
    _selectedMedia["link"] = editImageWidgetUi.txtLink->text();
    _selectedMedia["alt"] = editImageWidgetUi.txtAltText->text();
//     kDebug()<<_selectedMedia;
    AddMediaDialog::slotButtonClicked(button);
}

#include "composer/dialogs/addimagedialog.moc"
