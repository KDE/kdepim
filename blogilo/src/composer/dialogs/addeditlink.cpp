/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>

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

#include "addeditlink.h"
#include "settings.h"
// KConfigGroup AddEditLink::links( KGlobal::config(), QString::fromLatin1("LinksCache") );

AddEditLink::AddEditLink( QWidget *parent )
        : KDialog( parent )
{
    setAttribute(Qt::WA_DeleteOnClose);
    QWidget *dialog = new QWidget( this );
    ui.setupUi( dialog );
    ui.btnClear->setIcon(KIcon("edit-clear"));
    this->setMainWidget( dialog );

    this->resize( dialog->width(), dialog->height() );

    confGroup = new KConfigGroup( KGlobal::config(), QString::fromLatin1("AddEditLinkDialog") );
    QStringList linksList = confGroup->readEntry("LinksCache", QStringList());
    ui.txtAddress->addItems(linksList);
    KCompletion *comp = ui.txtAddress->completionObject( true );
    comp->setItems(linksList);
    ui.txtAddress->setCompletionMode(KGlobalSettings::CompletionPopupAuto);

    ui.txtAddress->setFocus();
    resize( confGroup->readEntry("Size", this->size()) );
    connect(ui.btnClear, SIGNAL(clicked(bool)), SLOT(slotClearLinkCache()) );
}

AddEditLink::~AddEditLink()
{
    confGroup->writeEntry( "Size", size() );
    confGroup->sync();
    delete confGroup;
}

void AddEditLink::slotButtonClicked( int button )
{
    if(button == KDialog::Ok) {
        QString link = ui.txtAddress->currentText();
        if ( link.isEmpty() )
            return;
        QString linkTarget;
        if ( ui.comboTarget->currentIndex() == 1 ) {
            linkTarget = "_self";
        } else if ( ui.comboTarget->currentIndex() == 2 ) {
            linkTarget = "_blank";
        }
        const QString target = linkTarget;
        if( Settings::urlCachingEnabled() ) {
            QStringList linksList = confGroup->readEntry("LinksCache", QStringList());
            linksList.append(link);
            confGroup->writeEntry("LinksCache", linksList );
            confGroup->sync();
        }
        emit addLink( link, target, ui.txtTitle->text() );
        accept();
    } else
        KDialog::slotButtonClicked(button);
}

void AddEditLink::show( const QString& address, const QString& title, const QString& target )
{
    KDialog::show();
    if ( address.isEmpty() ) {
        ui.txtAddress->insertUrl(0, QString());
        ui.txtAddress->setCurrentIndex(0);
        this->setWindowTitle( i18nc( "verb, to insert a link into the text", "Add Link" ) );
    } else {
        ui.txtAddress->setCurrentItem( address, true );
        this->setWindowTitle( i18nc( "verb, to modify an existing link", "Edit Link" ) );
    }
    if ( !title.isEmpty() ) {
        ui.txtTitle->setText( title );
    }
    if ( !target.isEmpty() ) {
        if ( target == "_self" ) {
            ui.comboTarget->setCurrentIndex( 1 );
        } else if ( target == "_blank" ) {
            ui.comboTarget->setCurrentIndex( 2 );
        }
    }
}

void AddEditLink::slotClearLinkCache()
{
    confGroup->writeEntry( "LinksCache", QStringList() );
    QString current = ui.txtAddress->currentText();
    ui.txtAddress->clear();
    ui.txtAddress->addItem( current );
    ui.txtAddress->setCurrentIndex( 0 );
    ui.txtAddress->completionObject()->clear();
}

#include "composer/dialogs/addeditlink.moc"
