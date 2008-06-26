//
//  kjots
//
//  Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
//  Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
//  Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
//  Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
//  Copyright (C) 2007-2008 Stephen Kelly <steveire@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

//Own Header
#include "kjotsedit.h"

#include <QMimeData>
#include <QTextCursor>
#include <QStackedWidget>
#include <QUrl>
#include <QMenu>
#include <QContextMenuEvent>

#include <kaction.h>
#include <kactioncollection.h>

#include "kjotsentry.h"
#include "kjotslinkdialog.h"
#include "bookshelf.h"

KJotsEdit::KJotsEdit ( QWidget *parent ) : KRichTextWidget(parent)
{
    setAcceptRichText(true);
    setWordWrapMode(QTextOption::WordWrap);
    setRichTextSupport( FullTextFormattingSupport
            | FullListSupport
            | SupportAlignment
            | SupportRuleLine
            | SupportFormatPainting );
}

KJotsEdit::~KJotsEdit()
{
}

void KJotsEdit::contextMenuEvent( QContextMenuEvent *event )
{
    QMenu *popup = createStandardContextMenu();
    connect( popup, SIGNAL( triggered ( QAction* ) ),
             this, SLOT( menuActivated( QAction* ) ) );

    popup->addSeparator();
    popup->addAction(actionCollection->action("copyIntoTitle"));
    popup->addAction(actionCollection->action("insert_checkmark"));

    popup->exec( event->globalPos() );

    delete popup;
}

void KJotsEdit::DelayedInitialization ( KActionCollection *collection, Bookshelf *shelf )
{
    bookshelf = shelf;
    actionCollection = collection;

    connect(actionCollection->action("auto_bullet"), SIGNAL(triggered()), SLOT(onAutoBullet()));
    connect(actionCollection->action("manage_link"), SIGNAL(triggered()), SLOT(onLinkify()));
    connect(actionCollection->action("insert_checkmark"), SIGNAL(triggered()), SLOT(addCheckmark()));

    connect(bookshelf, SIGNAL(itemSelectionChanged()), SLOT(onBookshelfSelection()));
    connect(this, SIGNAL(textChanged()), SLOT(onTextChanged()));


}

void KJotsEdit::disableEditing ( void )
{
    if ( currentPage ) {
        currentPage->setCursor(textCursor());
        setDocument(0);
        currentPage = 0;
    }

    setReadOnly(true);
    setEnabled(false);
}

void KJotsEdit::onBookshelfSelection ( void )
{
    QList<QTreeWidgetItem*> selection = bookshelf->selectedItems();
    int selectionSize = selection.size();

    if (selectionSize !=  1) {
        disableEditing();
    } else {
        KJotsPage *newPage = dynamic_cast<KJotsPage*>(selection[0]);
        if ( !newPage ) {
            disableEditing();
        } else {
            if ( currentPage != newPage ) {
                if ( currentPage ) {
                    currentPage->setCursor(textCursor());
                }
                currentPage = newPage;

                setEnabled(true);
                setReadOnly(false);
                setDocument(currentPage->body());
                if ( !currentPage->getCursor().isNull() ) {
                    setTextCursor(currentPage->getCursor());
                }

                QStackedWidget *stack = static_cast<QStackedWidget*>(parent());
                stack->setCurrentWidget(this);
                setFocus();

                if ( textCursor().atStart() )
                {
                    // Reflect formatting when switching pages and the first word is formatted
                    // Work-around for qrextedit bug. The format does not seem to exist
                    // before the first character. Submitted to qt-bugs, id 192886.
                    moveCursor(QTextCursor::Right);
                    moveCursor(QTextCursor::Left);
                }

            }
        }
    }
}

void KJotsEdit::onAutoBullet ( void )
{
    KTextEdit::AutoFormatting currentFormatting = autoFormatting();

    //TODO: set line spacing properly.

    if ( currentFormatting == KTextEdit::AutoBulletList ) {
        setAutoFormatting(KTextEdit::AutoNone);
        actionCollection->action("auto_bullet")->setChecked( false );
    } else {
        setAutoFormatting(KTextEdit::AutoBulletList);
        actionCollection->action("auto_bullet")->setChecked( true );
    }
}

void KJotsEdit::onLinkify ( void )
{
    selectLinkText();
    KJotsLinkDialog* linkDialog = new KJotsLinkDialog(this, bookshelf);
    linkDialog->setLinkText(currentLinkText());
    linkDialog->setLinkUrl(currentLinkUrl());

    if (linkDialog->exec()) {
        updateLink(linkDialog->linkUrl(), linkDialog->linkText());
    }

    delete linkDialog;
}

void KJotsEdit::addCheckmark( void )
{
    QTextCursor cursor = textCursor();
    static const QChar unicode[] = {0x2713};
    int size = sizeof(unicode) / sizeof(QChar);
    cursor.insertText( QString::fromRawData(unicode, size) );
}

void KJotsEdit::onTextChanged ( void )
{
    if ( currentPage ) {
        currentPage->parentBook()->setDirty(true);
    }
}

bool KJotsEdit::canInsertFromMimeData ( const QMimeData *source ) const
{
    if ( source->formats().contains("kjots/internal_link") ) {
        return true;
    } else if ( source->hasUrls() ) {
        return true;
    } else {
        return KTextEdit::canInsertFromMimeData(source);
    }
}

void KJotsEdit::insertFromMimeData ( const QMimeData *source )
{
    if ( source->formats().contains("kjots/internal_link") ) {
        insertHtml(source->data("kjots/internal_link"));
    } else if ( source->hasUrls() ) {
        foreach ( const QUrl &url, source->urls() ) {
            if ( url.isValid() ) {
                QString html = QString ( "<a href='%1'>%2</a> " )
                    .arg(QString::fromUtf8(url.toEncoded()))
                    .arg(url.toString(QUrl::RemovePassword));
                insertHtml(html);
            }
        }
    } else {
        KTextEdit::insertFromMimeData(source);
    }
}

#include "kjotsedit.moc"
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
