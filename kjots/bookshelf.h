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

#ifndef bookshelf_included
#define bookshelf_included

#include <QTreeWidget>
#include <QTreeWidgetItem>

class QMimeData;

class KActionCollection;

class KJotsPage;
class KJotsBook;
class KJotsEntry;

class Bookshelf : public QTreeWidget
{
    Q_OBJECT

public:
    explicit Bookshelf ( QWidget* );

    void DelayedInitialization ( KActionCollection* );
    void prepareForExit ( void );

    void jumpToId(quint64);
    void jumpToEntry(QTreeWidgetItem*);

    KJotsEntry* entryFromId(quint64);
    KJotsEntry* currentEntry ( void );
    KJotsPage* currentPage ( void );
    KJotsBook* currentBook ( void );
    KJotsBook* currentTopLevelBook ( void );
    
    QList<KJotsEntry*> selected ( void );
    void remove(QTreeWidgetItem*);
    QString currentCaption(const QString &);

    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QList<QTreeWidgetItem*>) const;
    bool dropMimeData ( QTreeWidgetItem *, int, const QMimeData*, Qt::DropAction );

public slots:
    void onHeaderClick(int);
    void nextBook();
    void prevBook();
    void nextPage();
    void prevPage();
        
protected:
    void loadBooks ( void );
    void sortBook(QTreeWidgetItem * );

    virtual void dragEnterEvent ( QDragEnterEvent * );
    virtual void dragMoveEvent ( QDragMoveEvent * );
    virtual void contextMenuEvent( QContextMenuEvent* );

    Qt::SortOrder sortOrder;

protected slots:
    void entryRenamed(QTreeWidgetItem*, int);
    void itemWasExpanded(QTreeWidgetItem*);
    void itemWasCollapsed(QTreeWidgetItem*);
    void copyLinkAddress();
    void changeColor();

private:
    KActionCollection* m_actionCollection;
};

#endif //bookshelf_included
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
