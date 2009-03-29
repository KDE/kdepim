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

#ifndef KJOTSCOMPONENT_H
#define KJOTSCOMPONENT_H

#include <QPushButton>

#include <kdialog.h>
#include <kxmlguiwindow.h>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTextCharFormat>


class QTimer;
class QTextEdit;
class QStackedWidget;
class QCheckBox;

class KAction;
class KActionMenu;
class KActionCollection;
class KFindDialog;
class KReplaceDialog;

class Bookshelf;
class KJotsPage;
class KJotsEdit;
class KJotsBrowser;
class KJob;


class KJotsComponent : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KJotsComponent")

    public:
        KJotsComponent(QWidget* parent, KActionCollection *actionCollection);
        ~KJotsComponent();

        QTextEdit* activeEditor();
        QString currentCaption();
        bool queryClose();

    protected:
        enum ExportType { Ascii, HTML, Native };

    public slots:
        void updateCaption();
        void updateMenu();
        Q_SCRIPTABLE void newPage();
        Q_SCRIPTABLE bool createNewBook();

    signals:
        void captionChanged(QString captionText);

        /**
        Signals that the text cursor in the editor is now on a different anchor, or not on
        an anchor anymore.
        @param anchorTarget The href of the focused anchor.
        @param anchorText The display text of the focused anchor.
        */
        void activeAnchorChanged(const QString &anchorTarget, const QString &anchorText);

    protected slots:
        void DelayedInitialization();
        void deleteBook();
        void deletePage();
        void deleteMultiple();
        void onRenameEntry();
        void configure();
        void updateConfiguration();
        void bookshelfEditItemFinished( QWidget *, QAbstractItemDelegate::EndEditHint );

        void copy();
        void copySelection();
        void insertDate();
        void onPrint();

        void onShowSearch();
        void onUpdateSearch();
        void onStartSearch();
        void onRepeatSearch();
        void onEndSearch();

        void onShowReplace();
        void onUpdateReplace();
        void onStartReplace();
        void onRepeatReplace();
        void onEndReplace();

        void onItemRenamed(QTreeWidgetItem*, int);
        void saveAscii();
        void saveHtml();
        void saveNative();
        void saveToFile(ExportType);
        void saveFinished(KJob *);
        void importBook();
        void currentCharFormatChanged(const QTextCharFormat &);

        void autoSave(void);

        /**
        Saves all books, whether marked as dirty or not.
        Called when closing KJots or when manual save action is invoked.
        */
        void saveAll(void);

        /**
        Saves and backs up all books.
        */
        void saveAndBackupAll();

    protected:
        int search(bool);
        void cleanupOldBackups();

private:

        KJotsEdit      *editor;
        KJotsBrowser   *browser;
        Bookshelf      *bookshelf;
        QSplitter      *splitter;
        QStackedWidget *stackedWidget;
        QFont           m_font;
        QTimer         *m_autosaveTimer;

        QString activeAnchor;

        KActionMenu *exportMenu, *bookmarkMenu;
        QSet<QAction*> entryActions, pageActions, bookActions, multiselectionActions;
        KActionCollection *actionCollection;


        KFindDialog *searchDialog;
        QStringList searchHistory;
        int searchBeginPos, searchEndPos, searchPos;
        QCheckBox *searchAllPages;

        KReplaceDialog *replaceDialog;
        QStringList replaceHistory;
        int replaceBeginPos, replaceEndPos, replacePos;
        QCheckBox *replaceAllPages;
        KJotsPage *replaceStartPage;
};




#endif // KJotsComponent_included
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
