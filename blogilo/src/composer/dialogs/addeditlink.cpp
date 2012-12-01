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

#include "addeditlink.h"
#include "settings.h"
#include "ui_addeditlinkbase.h"
// KConfigGroup AddEditLink::links( KGlobal::config(), QString::fromLatin1("LinksCache") );

class AddEditLink::Private
{
public:
    Link result;
    KConfigGroup *confGroup;
    Ui::AddEditLinkBase ui;
};
AddEditLink::AddEditLink( QWidget *parent )
        : KDialog( parent ), d(new Private)
{
    initUi();
    d->ui.txtAddress->insertUrl(0, QString());
    d->ui.txtAddress->setCurrentIndex(0);
    this->setWindowTitle( i18nc( "verb, to insert a link into the text", "Add Link" ) );
}

AddEditLink::AddEditLink(const QString& address, const QString& title, const QString& target,
                         QWidget* parent): KDialog(parent), d(new Private)
{
    initUi();
    if ( address.isEmpty() ) {
        d->ui.txtAddress->insertUrl(0, QString());
        d->ui.txtAddress->setCurrentIndex(0);
        this->setWindowTitle( i18nc( "verb, to insert a link into the text", "Add Link" ) );
    } else {
        d->ui.txtAddress->setCurrentItem( address, true );
        this->setWindowTitle( i18nc( "verb, to modify an existing link", "Edit Link" ) );
    }
    if ( !title.isEmpty() ) {
        d->ui.txtTitle->setText( title );
    }
    if ( !target.isEmpty() ) {
        if ( target == "_self" ) {
            d->ui.comboTarget->setCurrentIndex( 1 );
        } else if ( target == "_blank" ) {
            d->ui.comboTarget->setCurrentIndex( 2 );
        }
    }
}

AddEditLink::~AddEditLink()
{
    d->confGroup->writeEntry( "Size", size() );
    d->confGroup->sync();
    delete d->confGroup;
    delete d;
}

void AddEditLink::initUi()
{
    QWidget *dialog = new QWidget( this );
    d->ui.setupUi( dialog );
    d->ui.btnClear->setIcon(KIcon("edit-clear"));
    this->setMainWidget( dialog );

    this->resize( dialog->width(), dialog->height() );

    d->confGroup = new KConfigGroup( KGlobal::config(), QString::fromLatin1("AddEditLinkDialog") );
    QStringList linksList = d->confGroup->readEntry("LinksCache", QStringList());
    linksList.removeDuplicates();
    d->ui.txtAddress->addItems(linksList);
    KCompletion *comp = d->ui.txtAddress->completionObject( true );
    comp->setItems(linksList);
    d->ui.txtAddress->setCompletionMode(KGlobalSettings::CompletionPopupAuto);

    d->ui.txtAddress->setFocus();
    resize( d->confGroup->readEntry("Size", this->size()) );
    connect(d->ui.btnClear, SIGNAL(clicked(bool)), SLOT(slotClearLinkCache()) );
}

Link& AddEditLink::result() const
{
    return d->result;
}

void AddEditLink::slotButtonClicked( int button )
{
    if(button == KDialog::Ok) {
        QString link = d->ui.txtAddress->currentText();
        if ( link.isEmpty() )
            return;
        QString linkTarget;
        if ( d->ui.comboTarget->currentIndex() == 1 ) {
            linkTarget = "_self";
        } else if ( d->ui.comboTarget->currentIndex() == 2 ) {
            linkTarget = "_blank";
        }
        const QString target = linkTarget;
        if( Settings::urlCachingEnabled() ) {
            QStringList linksList = d->confGroup->readEntry("LinksCache", QStringList());
            linksList.append(link);
            d->confGroup->writeEntry("LinksCache", linksList );
            d->confGroup->sync();
        }
        d->result.address = link;
        d->result.target = target;
        d->result.title = d->ui.txtTitle->text();
        accept();
    } else
        KDialog::slotButtonClicked(button);
}

void AddEditLink::slotClearLinkCache()
{
    d->confGroup->writeEntry( "LinksCache", QStringList() );
    QString current = d->ui.txtAddress->currentText();
    d->ui.txtAddress->clear();
    d->ui.txtAddress->addItem( current );
    d->ui.txtAddress->setCurrentIndex( 0 );
    d->ui.txtAddress->completionObject()->clear();
}

#include "composer/dialogs/addeditlink.moc"
