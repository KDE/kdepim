//
//  kjots
//
//  Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
//  Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
//  Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
//  Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
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
#include "kjotsbrowser.h"

#include <QTextBrowser>
#include <QTextDocument>
#include <QStackedWidget>

#include <krun.h>
#include <kdebug.h>

#include "kjotsentry.h"
#include "bookshelf.h"

KJotsBrowser::KJotsBrowser ( QWidget *parent ) : QTextBrowser(parent)
{
    setWordWrapMode(QTextOption::WordWrap);
}

KJotsBrowser::~KJotsBrowser()
{
}

void KJotsBrowser::DelayedInitialization ( Bookshelf *shelf )
{
    bookshelf = shelf;
    connect(bookshelf, SIGNAL(itemSelectionChanged()), SLOT(onSelectionChange()));
    connect(this, SIGNAL(anchorClicked(const QUrl&)), SLOT(linkClicked(const QUrl&)));
}

void KJotsBrowser::onSelectionChange ( void )
{
    QList<KJotsEntry*> selection = bookshelf->selected();
    int selectionSize = selection.size();

    QStackedWidget *stack = static_cast<QStackedWidget*>(parent());
    
    if ( selectionSize == 0 ) {
        clear();
        setEnabled(false);
        stack->setCurrentWidget(this);
        setFocus();
    } else if (selectionSize ==  1 && selection[0]->isPage() ) {
        setEnabled(false);
    } else {
        setEnabled(true);
        QTextDocument document;
        QTextCursor bookCursor ( &document );

        foreach ( KJotsEntry *entry, selection ) {
            entry->generateHtml(entry, false, &bookCursor);
        }

        setHtml(document.toHtml());

        stack->setCurrentWidget(this);
        setFocus();
    }
}

/*!
    \brief Handle link clicks.
*/
void KJotsBrowser::linkClicked(const QUrl& link)
{
    kDebug() << "Link clicked: " << link;

    //Stop QTextBrowser from being stupid by giving it an invalid url.
    QUrl url;
    setSource(url);

    QString anchor = link.fragment();
    if ( anchor.size() ) {
        scrollToAnchor(anchor);
        return;
    }

    if ( link.scheme() == "kjots" ) {
        quint64 target = link.path().mid(1).toULongLong();
        bookshelf->jumpToId(target);
    } else {
        new KRun ( link, this );
    }

    return;
}

#include "kjotsbrowser.moc"
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
