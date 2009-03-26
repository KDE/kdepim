//
//  Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
//  Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
//  Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
//  Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
//  Copyright (C) 2007 Stephen Kelly <steveire@gmail.com>
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
#include "bookshelf.h"

#include <QMimeData>
#include <QDir>
#include <QApplication>
#include <QClipboard>
#include <QDragMoveEvent>
#include <QMenu>
#include <QContextMenuEvent>
#include <QHeaderView>

#include <kaction.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kactioncollection.h>
#include <kmenu.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <kcolordialog.h>

#include "kjotsentry.h"
#include "KJotsSettings.h"

QString mimeType = "kjots/dropdata";

Bookshelf::Bookshelf ( QWidget *parent ) : QTreeWidget(parent)
{
    sortOrder = Qt::DescendingOrder;

    setObjectName( "bookshelf" );
    setColumnCount(2);
    setColumnHidden(1, true);
    setRootIsDecorated(true);
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding));
    setMinimumWidth(fontMetrics().maxWidth() * 5 + 5);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    headerItem()->setText(0, i18n("Bookshelf"));
    header()->setClickable(true);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), SLOT(entryRenamed(QTreeWidgetItem*, int)));

}

void Bookshelf::DelayedInitialization ( KActionCollection *actionCollection ) {
    loadBooks();

    //These need to be connected after books are loaded.
    connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)), SLOT(itemWasExpanded(QTreeWidgetItem*)));
    connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem*)), SLOT(itemWasCollapsed(QTreeWidgetItem*)));

    connect(actionCollection->action("copy_link_address"), SIGNAL(triggered()),
        this, SLOT(copyLinkAddress()));
    connect(actionCollection->action("change_color"), SIGNAL(triggered()),
        this, SLOT(changeColor()));
    // This doesn't work and causes crashes eg when moving books after sorting
    // Disabling for KDE4.2.
//     connect(header(), SIGNAL(sectionClicked(int)), this, SLOT(onHeaderClick(int)));

    m_actionCollection = actionCollection;

    return;
}

/*!
    \brief Loads all the books it can find in the appdata directory.
    \warning Needless to say: this should only ever be called once.
    For reasons surpassing understanding, QTreeWidgetItems do not behave well
    when you construct a tree without first attaching them to a QTreeWidget.
    Hopefully, this will be fixed in a future version of Qt. Until then, we
    attach the books and create them first, then remove them, then reattach
    them in the correct order.
*/
void Bookshelf::loadBooks ( void )
{
    QDir dir(KStandardDirs::locateLocal("data","kjots"));
    QList<KJotsBook*> books;

    QStringList filter;
    filter << "*.book";

    setUpdatesEnabled(false);

    //Read in books from disk
    QStringList files = dir.entryList(filter, QDir::Files|QDir::Readable);
    foreach ( const QString &file, files ) {
        QString filepath = dir.absoluteFilePath(file);
        KJotsBook* book = new KJotsBook();
        book->openBook(filepath);
        books << book;
    }

    //Sort books and add them in order. KConfig doesn't have a built-in
    //ULongLongList so we do our own thing for now.
    QString orderString = KJotsSettings::bookshelfOrder();
    QStringList idStrings = orderString.split(',');
    foreach ( const QString &idString, idStrings ) {
        quint64 id = idString.toLongLong();
        foreach ( KJotsBook *book, books ) {
            if ( book->id() == id ) {
                addTopLevelItem(book);
                books.removeAll(book);
                break;
            }
        }
    }

    //If we missed anything someone must by playing around with the data
    //files. Just add it to the end.
    foreach ( KJotsBook *book, books ) {
        addTopLevelItem(book);
    }

    // Open any books that need opening.
    if ( topLevelItemCount() ) {
        QTreeWidgetItemIterator it ( this, QTreeWidgetItemIterator::HasChildren );
        for ( ; *it ; it++ ) {
            KJotsBook *book = static_cast<KJotsBook*>(*it);

            if ( book->shouldBeOpened() ) {
                setItemExpanded(book, true);
            }
        }
    }

    setUpdatesEnabled(true);

    return;
}

void Bookshelf::contextMenuEvent( QContextMenuEvent* event)
{
    QMenu *popup = new QMenu(this);

    // Set up the context menu on the bookshelf.
    // TODO: Move this to the xml file.
    popup->addAction(m_actionCollection->action("new_book"));
    popup->addAction(m_actionCollection->action("new_page"));
    popup->addAction(m_actionCollection->action("rename_entry"));
    popup->addAction(m_actionCollection->action("save_to"));
    popup->addAction(m_actionCollection->action("copy_link_address"));
    popup->addAction(m_actionCollection->action("change_color"));
    popup->addSeparator();

    QList<QTreeWidgetItem*> selection = selectedItems();
    if ( selection.size() == 1 ) {
        KJotsEntry* entry = static_cast<KJotsEntry*>(selection.at(0));
        if ( entry->isBook() ) {
            popup->addAction(m_actionCollection->action("del_folder"));
        } else {
            popup->addAction(m_actionCollection->action("del_page"));
        }
    } else if ( selection.size() < 1 ) {
        // No meaningful selection.
    } else {
        // Multiple items selected.
        popup->addAction(m_actionCollection->action("del_mult"));
    }

    popup->exec( event->globalPos() );

    delete popup;
}

/*!
*    Runs just before KJots is closed. Save your stuff!
*/
void Bookshelf::prepareForExit ( void )
{
    QList<QTreeWidgetItem*> selection = selectedItems();
    if ( selection.size() == 1 ) {
        KJotsEntry *entry = static_cast<KJotsBook*>(selection[0]);
        KJotsSettings::setCurrentSelection(entry->id());
    } else {
        KJotsSettings::setCurrentSelection(0);
    }

    QStringList idList;
    int tops = topLevelItemCount();
    for ( int i=0; i<tops; i++ ) {
        KJotsBook *book = static_cast<KJotsBook*>(topLevelItem(i));
        idList << QString::number(book->id());
    }
    KJotsSettings::setBookshelfOrder( idList.join(",") );

}

/*!
*    Returns a pointer to the KJotsEntry with the given ID or
*     0 if none matches.
*    \param id The ID to look for.
*    \todo Should we optimize this by using a container?
*/
KJotsEntry* Bookshelf::entryFromId(quint64 id)
{
    KJotsEntry *entry = 0;
    if ( !id ) return entry; //why waste time?

    for ( QTreeWidgetItemIterator it( this ); *it; it++ ) {
        entry = dynamic_cast<KJotsEntry*>(*it);
        if ( entry && entry->id() == id ) {
            break;
        }
    }

    return entry;
}

/*!
*    Makes the given ID the new selection in the bookshelf.
*    \param id The ID to select.
*/
void Bookshelf::jumpToId(quint64 id)
{
    KJotsEntry *entry = entryFromId(id);
    jumpToEntry(entry);

    return;
}

/*!
*    Makes the given entry the new selection in the bookshelf.
*    \param item The entry to select.
*/
void Bookshelf::jumpToEntry(QTreeWidgetItem *item)
{
    if ( item ) {
        clearSelection();
        scrollToItem(item);
        setItemSelected(item, true);
    }

    return;
}

/*!
*  Remove the given entry from the bookshelf and delete it if necessary.
*/
void Bookshelf::remove(QTreeWidgetItem *item)
{
    KJotsEntry *entry = dynamic_cast<KJotsEntry*>(item);
    Q_ASSERT(entry);


    if ( entry->isBook() ) {
        // Backup the book before it gets removed from the bookshelf.
        KJotsBook *book = dynamic_cast<KJotsBook*>(entry);
        book->saveAndBackupBook();
    }

    if ( entry->parentBook() ) {
        entry->parentBook()->takeChild(entry->parentBook()->indexOfChild(entry));
    } else {
        takeTopLevelItem(indexOfTopLevelItem(entry));
    }

    if ( entry->isBook() ) {
        // Fix for bug 164480.
        // Not certain what the problem is here, but if a page is selected and
        // its parent book is deleted, we get a crash. The workaround is to
        // select the book before removing it.
        // Stephen Kelly - 20th June 2008
        jumpToEntry( entry );

        KJotsBook *book = dynamic_cast<KJotsBook*>(entry);
        if (book)
        {
//             book->
            book->deleteBook();
        }
    }

    delete entry;
}

/*!
*    Indicate the drop types we handle.
*/
Qt::DropActions Bookshelf::supportedDropActions() const
{
    return Qt::CopyAction;
}

/*!
*    Indicate the mime types we handle.
*/
QStringList Bookshelf::mimeTypes() const
{
    QStringList types;
    types << mimeType;
    return types;
}

void Bookshelf::dragEnterEvent ( QDragEnterEvent *event )
{
    QTreeWidget::dragEnterEvent(event);

    //Don't accept multi-drags in the bookshelf
    QByteArray incomingData = event->mimeData()->data(mimeType);
    if ( incomingData.count('|') ) {
        event->setAccepted(false);
    }
}

void Bookshelf::dragMoveEvent ( QDragMoveEvent *event )
{
    QTreeWidget::dragMoveEvent(event);

    QByteArray incomingData = event->mimeData()->data(mimeType);
    quint64 id = incomingData.toULongLong();
    KJotsEntry *entry = entryFromId(id);

    //Don't allow a book to be dragged into itself
    if ( entry->isBook() ) {
        KJotsBook *book = static_cast<KJotsBook*>(entry);
        QTreeWidgetItem *item = itemAt(event->pos());
        KJotsEntry *target = static_cast<KJotsEntry*>(item);
        if ( book->contents().contains(target) ) {
            event->setAccepted(false);
        }
    }
}

/*!
*    Exports dropped data.
*/
QMimeData* Bookshelf::mimeData(const QList<QTreeWidgetItem*> items) const
{
    QMimeData *mimeData = new QMimeData();
    QStringList ids;
    QString textData, htmlData;

    foreach ( QTreeWidgetItem *item, items ) {
        KJotsEntry *entry = dynamic_cast<KJotsEntry*>(item);

        if ( entry ) {
            ids << QString::number(entry->id());

            if ( entry->isPage() ) {
                textData += static_cast<KJotsPage*>(entry)->body()->toPlainText();
                htmlData += static_cast<KJotsPage*>(entry)->body()->toHtml();
            }
        }
    }

    mimeData->setData(mimeType, ids.join(QString('|')).toAscii());
    mimeData->setText(textData);
    mimeData->setHtml(htmlData);
    return mimeData;
}

/*!
*    Imports dropped data.
*/
bool Bookshelf::dropMimeData ( QTreeWidgetItem *parent, int index,
    const QMimeData *data, Qt::DropAction action )
{
    QList<KJotsEntry*> movedItems;
    KJotsBook *newParent = dynamic_cast<KJotsBook*>(parent);
    bool success = false;

    if (action == Qt::IgnoreAction) return true; //Sanity Check

    QList<QByteArray> ids = data->data(mimeType).split('|');
    foreach ( const QByteArray &id, ids ) {
        movedItems << entryFromId(id.toULongLong());
    }

    foreach ( KJotsEntry *item, movedItems ) {
        bool wasOpened = isItemExpanded(item);

        // We don't accept pages in the root. Create a new book?
        if ( !newParent && item->isPage() ) {
            //Disable ourselves to prevent user from doing anything.
            QWidget::setEnabled(false);

            if (KMessageBox::questionYesNo(this,
                i18n("All pages must be inside a book. "
                "Would you like to create a new book to put the page in, "
                "or would you prefer to not move the page at all?"),
                QString(), KGuiItem(i18n("Create New Book")), KGuiItem(i18n("Do Not Move Page"))) ==
                KMessageBox::Yes )
            {
                //TODO: create the book at the place where the drop occurred instead of
                //appending it to the end.
                KJotsBook* book = new KJotsBook();
                addTopLevelItem(book);
                //TODO: Ask for Book name.
                book->setTitle(i18n("New Book"));
                item->parentBook()->setDirty(true);
                item->parentBook()->takeChild(index);
                book->setDirty(true);
                book->addChild(item);
            }

            QWidget::setEnabled(true);
        } else {
            int oldIndex = (item->parentBook()) ?
                item->parentBook()->indexOfChild(item) : indexOfTopLevelItem(item);

            //Proceed with caution when keeping the same parent...
            if ( newParent == item->parentBook() ) {

                //Make sure we are actually moving somewhere.
                if ( index == (oldIndex + 1) || index == oldIndex ) {
                    return false;
                }

                if ( index > oldIndex ) {
                    --index; //Don't count ourselves.
                }

                if ( newParent ) {
                    newParent->setDirty(true);
                    newParent->takeChild(oldIndex);
                    parent->insertChild(index, item);
                } else {
                    item->topLevelBook()->setDirty(true);
                    takeTopLevelItem(oldIndex);
                    insertTopLevelItem(index, item);
                }
            } else {
                if ( !item->parentBook() ) {
                    takeTopLevelItem(oldIndex);
                } else {
                    item->parentBook()->setDirty(true);
                    item->parentBook()->takeChild(oldIndex);
                }

                if ( !parent ) {
                    insertTopLevelItem(index, item);
                    // Nested book moved to the root.
                    // Open the book so it can be marked dirty and saved.
                    KJotsBook *book = static_cast<KJotsBook*>(item);
                    book->openBook(QString());
                } else {
                    parent->insertChild(index, item);
                }

                item->topLevelBook()->setDirty(true);
                success = true;
            }

            if ( wasOpened ) {
                setItemExpanded(item, true);
            }
        }
    }

    return success;
}

void Bookshelf::entryRenamed(QTreeWidgetItem* item, int  /*col*/)
{
    KJotsEntry* entry = dynamic_cast<KJotsEntry*>(item);

    if (entry) {
        entry->topLevelBook()->setDirty(true);
        jumpToEntry(entry);
    }
}

/*!
    Returns a pointer to the currently selected page.
*/
KJotsEntry* Bookshelf::currentEntry ( void )
{
    KJotsEntry *entry = 0;
    QList<QTreeWidgetItem*> selection = selectedItems();

    if ( selection.size() == 1 ) {
        entry = dynamic_cast<KJotsEntry*>(selection.at(0));
    }

    return entry;
}
/*!
    Returns a pointer to the currently selected page.
*/
KJotsPage* Bookshelf::currentPage ( void )
{
    KJotsPage *page = 0;
    QList<QTreeWidgetItem*> selection = selectedItems();

    if ( selection.size() == 1 ) {
        page = dynamic_cast<KJotsPage*>(selection.at(0));
    }

    return page;
}

/*!
    Returns a pointer to the currently selected book, or the book that owns
    the currently selected page.
*/
KJotsBook* Bookshelf::currentBook ( void )
{
    KJotsBook *book = 0;
    QList<QTreeWidgetItem*> selection = selectedItems();

    if ( selection.size() == 1 ) {
        KJotsEntry *entry = dynamic_cast<KJotsEntry*>(selection.at(0));
        Q_ASSERT(entry);

        if ( entry->isPage() ) {
            book = entry->parentBook();
        } else {
            book = dynamic_cast<KJotsBook*>(entry);
        }
    }

    return book;
}

/*!
    Returns a pointer to the toplevel book that owns the currently selected
    book or page.
*/
KJotsBook* Bookshelf::currentTopLevelBook ( void )
{
    KJotsBook *book = currentBook();

    while ( book && book->parentBook() ) {
        book = book->parentBook();
    }

    return book;
}

/*!
    \brief Called when a book is opened/expanded/whatever.
*/
void Bookshelf::itemWasExpanded(QTreeWidgetItem *item)
{
    KJotsEntry *entry = dynamic_cast<KJotsEntry*>(item);
    if ( entry ) {
        entry->topLevelBook()->setDirty(true);
    }
}

/*!
    \brief Called when a book is closed/collapsed/whatever.
*/
void Bookshelf::itemWasCollapsed(QTreeWidgetItem *item)
{
    KJotsEntry *entry = dynamic_cast<KJotsEntry*>(item);
    if ( entry ) {
        entry->topLevelBook()->setDirty(true);
    }
}

/*!
    \brief Called when selected from the context menu.
*/
void Bookshelf::copyLinkAddress()
{
    QList<QTreeWidgetItem*> selection = selectedItems();
    if ( selection.size() != 1 ) return;

    KJotsEntry *entry = static_cast<KJotsEntry*>(selection.at(0));
    QMimeData *mimeData = new QMimeData();

    QString link = QString("<a href=\"%1\">%2</a>").arg(entry->kjotsLinkUrl()).arg(entry->title());

    mimeData->setData("kjots/internal_link", link.toUtf8());
    mimeData->setText(entry->title());
    QApplication::clipboard()->setMimeData(mimeData);
    return;
}

/*!
    Return everything selected, but don't let a page of a book and a book
    both be in the list because then there would be duplication.
*/
QList<KJotsEntry*> Bookshelf::selected(void)
{
    QList<KJotsEntry*> entries;

    foreach ( QTreeWidgetItem *item, selectedItems() ) {
        entries << static_cast<KJotsEntry*>(item);
    }

    //The book itself overrides its pages.
    foreach ( KJotsEntry *entry, entries ) {
        if ( entry->isBook() ) {
            KJotsBook *book = static_cast<KJotsBook*>(entry);
            foreach ( KJotsEntry *content, book->contents() ) {
                entries.removeAll(content);
            }
        }
    }

    return entries;
}

/*!
    Returns a representation of the current selection.
*/
QString Bookshelf::currentCaption(const QString &sep)
{
    QString caption;

    QList<QTreeWidgetItem*> selection = selectedItems();
    int selectionSize = selection.size();
    if ( selectionSize > 1 ) {
        caption = i18n("Multiple selections");
    } else if ( selectionSize == 1 ) {
        QTreeWidgetItem *item = selection[0];
        while ( item ) {
            QTreeWidgetItem *parentBook = item->parent();

            if ( parentBook ) {
                caption = sep + item->text(0) + caption;
            } else {
                caption = item->text(0) + caption;
            }

            item=parentBook;
        }
    }

    return caption;
}

void Bookshelf::changeColor()
{
    QColor myColor;
    int result = KColorDialog::getColor( myColor );
    if ( result == KColorDialog::Accepted ) {
        foreach( KJotsEntry *entry, selected() ) {
            entry->setBackgroundColor(0, myColor);
            entry->topLevelBook()->setDirty(true);
            if ( entry->isBook() ) {
                KJotsBook *book = static_cast<KJotsBook*>(entry);
                foreach( KJotsEntry *entry2, book->contents() ) {
                    entry2->setBackgroundColor(0, myColor);
                }
            }
        }
    }
}

/*!
    \brief Moves to the next book after the current selection.
*/
void Bookshelf::nextBook()
{
    QTreeWidgetItem *item = currentEntry();
    if ( !item ) return;

    QTreeWidgetItemIterator it ( item, QTreeWidgetItemIterator::HasChildren );
    if ( *it == item ) it++;

    if ( *it == 0 ) {
        // Just set to first book.
        it = QTreeWidgetItemIterator ( topLevelItem(0) );
    }

    if ( *it && *it != item ) {
        jumpToEntry(*it);
    }

    return;
}

/*!
    \brief Moves to the previous book before the current selection.
*/
void Bookshelf::prevBook()
{
    KJotsEntry *entry = currentEntry();
    QTreeWidgetItem *item = entry;
    if ( !item ) return;

    QTreeWidgetItemIterator it ( item );
    while ( *it ) {
        --it;

        if ( *it == 0 ) {
            QModelIndex index = moveCursor(QAbstractItemView::MoveEnd, Qt::NoModifier);
            it = QTreeWidgetItemIterator ( itemFromIndex(index) );
        }

        if ( static_cast<KJotsEntry*>(*it)->isBook() ) {
            break;
        }
    }

    if ( *it && *it != item ) {
        jumpToEntry(*it);
    }

    return;
}

/*!
    \brief Moves to the next page after the current selection.
*/
void Bookshelf::nextPage()
{
    QTreeWidgetItem *item = currentEntry();
    if ( !item ) return;

    QTreeWidgetItemIterator it ( item, QTreeWidgetItemIterator::NoChildren );
    if ( *it == item ) it++;

    if ( *it == 0 ) {
        it = QTreeWidgetItemIterator ( topLevelItem(0),
            QTreeWidgetItemIterator::NoChildren );
    }

    if ( *it && *it != item ) {
        jumpToEntry(*it);
    }

    return;
}

/*!
    \brief Moves to the previous page before the current selection.
*/
void Bookshelf::prevPage()
{
    KJotsEntry *entry = currentEntry();
    QTreeWidgetItem *item = entry;
    if ( !item ) return;

    QTreeWidgetItemIterator it ( item );
    while ( *it ) {
        --it;

        if ( *it == 0 ) {
            QModelIndex index = moveCursor(QAbstractItemView::MoveEnd, Qt::NoModifier);
            it = QTreeWidgetItemIterator ( itemFromIndex(index) );
        }

        if ( static_cast<KJotsEntry*>(*it)->isBook() == false ) {
            break;
        }
    }

    if ( *it && *it != item ) {
        jumpToEntry(*it);
    }

    return;
}

void Bookshelf::onHeaderClick(int)
{
    sortBook(invisibleRootItem());
    if ( !header()->isSortIndicatorShown() ) {
        header()->setSortIndicatorShown(true);
    }
    header()->setSortIndicator(0, sortOrder);
    header()->update();
    return;
}

// Sorting with the Qt widgets sucks, so we make our own. Believe it
// or not but yes; all this crap is actually necessary to get it to
// work. Things fail spectaculalry if you don't make sure all the
// books are closed before sorting them.
// --
// The problem is that sorting the children of an item does not rearrange the children of that
// item when it is sorted. Fix scheduled for Qt4.5.
// http://trolltech.com/developer/task-tracker/index_html?method=entry&id=233975
// -- Stephen Kelly, 16th Jan 2009.
void Bookshelf::sortBook(QTreeWidgetItem *book )
{
    setUpdatesEnabled(false);

    QList<QTreeWidgetItem*> openedBooks;
    QTreeWidgetItemIterator it ( book, QTreeWidgetItemIterator::HasChildren );
    while ( *it ) {
        if ( (*it)->isExpanded() ) {
            openedBooks << *it;
            (*it)->setExpanded(false);
        }
        it++;
    }

    sortOrder = (sortOrder==Qt::DescendingOrder) ?
        Qt::AscendingOrder : Qt::DescendingOrder;
    book->sortChildren(0, sortOrder);

    foreach ( QTreeWidgetItem *item, openedBooks ) {
        item->setExpanded(true);
    }

    setUpdatesEnabled(true);

    return;
}

#include "bookshelf.moc"
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
