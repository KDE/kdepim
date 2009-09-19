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

#include "medialistwidget.h"

#include <QContextMenuEvent>
#include <QtGui/QClipboard>

#include <kmenu.h>
#include <kaction.h>
#include <klocalizedstring.h>
#include <kdialog.h>
#include <kdebug.h>

MediaListWidget::MediaListWidget( QWidget *parent ): KListWidget( parent )
{
    actEdit = new KAction( i18n( "Edit properties" ), this );
    connect( actEdit, SIGNAL( triggered( bool ) ), this, SLOT( sltEditProperties() ) );
    actCopyUrl = new KAction( i18n( "Copy Url" ), this );
    connect( actCopyUrl, SIGNAL( triggered( bool ) ), this, SLOT( sltCopyUrl() ) );
    actRemove = new KAction( i18n( "Remove media" ), this );
    connect( actRemove, SIGNAL( triggered( bool ) ), this, SLOT( sltRemoveMedia() ) );
}

MediaListWidget::~MediaListWidget()
{
}

void MediaListWidget::contextMenuEvent( QContextMenuEvent *event )
{
    if ( this->itemAt( event->pos() ) ) {
        KMenu menu( this );
        if ( this->itemAt( event->pos() )->type() == ImageType ) {
            menu.addAction( actEdit );
        }
        menu.addAction( actCopyUrl );
        menu.addAction( actRemove );
        menu.exec( event->globalPos() );
    }
}

void MediaListWidget::sltEditProperties()
{
//     QWidget *temp = new QWidget( this );
    KDialog *dialog = new KDialog( this );
    QWidget *temp = new QWidget( dialog );
    //ui.setupUi(dialog);
    ui.setupUi( temp );
    dialog->setMainWidget( temp );
    dialog->setWindowTitle( temp->windowTitle() );
    dialog->resize( temp->width(), temp->height() );
    dialog->setWindowModality( Qt::WindowModal );
//     dialog->setAttribute( Qt::WA_DeleteOnClose );
    connect( dialog, SIGNAL( okClicked() ), this, SLOT( sltSetProperties() ) );
    ui.spinboxWidth->setFocus();
    
    dialog->exec();
    dialog->deleteLater();
}
void MediaListWidget::sltSetProperties()
{
    Q_EMIT( sigSetProperties( this->currentRow(), ui.spinboxWidth->value(),
                              ui.spinboxHeight->value(), ui.txtTitle->text(), 
                              ui.txtLink->text(), ui.txtAltText->text() ) );
}

void MediaListWidget::sltCopyUrl()
{
    QApplication::clipboard()->setText( this->currentItem()->toolTip() );
}

void MediaListWidget::sltRemoveMedia()
{
    kDebug() << this->currentRow();
    Q_EMIT( sigRemoveMedia( this->currentRow() ) );
}

#include "composer/medialistwidget.moc"
