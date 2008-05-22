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
#include "kjotscomponent.h"

#include <q3header.h>
#include <QStackedWidget>
#include <QtDBus/QDBusConnection>
#include <QPainter>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>
#include <QAbstractTextDocumentLayout>
#include <QTextCodec>
#include <QCheckBox>
#include <QLabel>
#include <QGridLayout>
#include <QTextFrame>
#include <QFont>

#include <QFrame>
#include <QVBoxLayout>
#include <kactionmenu.h>

#include <kio/job.h>
#include <kio/copyjob.h>
#include <kio/netaccess.h>
#include <kicon.h>
#include <kfinddialog.h>
#include <kfind.h>
#include <kreplacedialog.h>
#include <kreplace.h>
#include <ktemporaryfile.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kbookmarkmenu.h>
#include <kconfigdialog.h>
#include <kencodingfiledialog.h>
#include <kglobalsettings.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kstandardshortcut.h>
#include <kstandardaction.h>

#include "KJotsSettings.h"
#include "kjotsbookmarks.h"
#include "kjotsedit.h"
#include "kjotsbrowser.h"
#include "kjotsentry.h"
#include "bookshelf.h"
#include "kjotsreplacenextdialog.h"

#include <memory>

//----------------------------------------------------------------------
// KJOTSMAIN
//----------------------------------------------------------------------
KJotsComponent::KJotsComponent(QWidget* parent, KActionCollection *collection) : QWidget(parent)
{
    actionCollection = collection;
    selectionOnlyActions = pageOnlyActions = 0;
    searchDialog = 0;

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/KJotsComponent", this, QDBusConnection::ExportScriptableSlots);

    //
    // Main widget
    //

    splitter = new QSplitter(this);
    splitter->setOpaqueResize( KGlobalSettings::opaqueResize() );

    bookshelf = new Bookshelf(splitter);
    stackedWidget = new QStackedWidget(splitter);
    editor = new KJotsEdit(stackedWidget);
    editor->createActions(actionCollection);
    editor->setEnabled(false);
    stackedWidget->addWidget(editor);
    browser = new KJotsBrowser(stackedWidget);
    browser->setEnabled(false);
    stackedWidget->addWidget(browser);

    QVBoxLayout *bookGrid = new QVBoxLayout(this);
    bookGrid->setMargin(KDialog::marginHint());
    bookGrid->setSpacing(KDialog::spacingHint());
    bookGrid->addWidget(splitter, 0, 0);
    bookGrid->setMargin(0);

    splitter->setStretchFactor(1, 1);

    // I've moved as much I could into DelayedInitialization(), but the XML
    // gui builder won't insert things properly if they don't get in there early.
    QAction *action;
    action = actionCollection->addAction( "go_next_book");
    action->setText( i18n("Next Book") );
    action->setIcon(KIcon("go-next"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    connect(action, SIGNAL(triggered()), bookshelf, SLOT(nextBook()));


    action = actionCollection->addAction( "go_prev_book");
    action->setText( i18n("Previous Book") );
    action->setIcon(KIcon("go-previous"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_D));
    connect(action, SIGNAL(triggered()), bookshelf, SLOT(prevBook()));

    action = actionCollection->addAction( "go_next_page");
    action->setText( i18n("Next Page") );
    action->setIcon(KIcon("go-next"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageDown));
    connect(action, SIGNAL(triggered()), bookshelf, SLOT(nextPage()));


    action = actionCollection->addAction( "go_prev_page" );
    action->setText( i18n("Previous Page") );
    action->setIcon(KIcon("go-previous"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageUp));
    connect(action, SIGNAL(triggered()), bookshelf, SLOT(prevPage()));

    action = actionCollection->addAction(  "new_page");
    action->setText( i18n("&New Page") );
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    action->setIcon(KIcon("document-new"));
    connect(action, SIGNAL(triggered()), SLOT(newPage()));

    action = actionCollection->addAction("new_book");
    action->setText(i18n("New &Book..."));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N));
    action->setIcon(KIcon("x-office-address-book"));
    // should rather be "address-book-new", use that once it exists in Oxygen
    connect(action, SIGNAL(triggered()), SLOT(createNewBook()));

    exportMenu = actionCollection->add<KActionMenu>("save_to");
    exportMenu->setText(i18n("Export"));
    exportMenu->setIcon(KIcon("document-export"));
    action = actionCollection->addAction("save_to_ascii");
    action->setText(i18n("To Text File..."));
    action->setIcon(KIcon("text-plain"));
    connect(action, SIGNAL(triggered()), SLOT(saveAscii()));
    exportMenu->menu()->addAction( action );

    action = actionCollection->addAction("save_to_html");
    action->setText(i18n("To HTML File..."));
    action->setIcon(KIcon("text-html"));
    connect(action, SIGNAL(triggered()), SLOT(saveHtml()));
    exportMenu->menu()->addAction( action );

    action = actionCollection->addAction("save_to_book");
    action->setText(i18n("To Book File..."));
    action->setIcon(KIcon("x-office-address-book"));
    connect(action, SIGNAL(triggered()), SLOT(saveNative()));
    exportMenu->menu()->addAction( action );

    action = actionCollection->addAction("import");
    action->setText(i18n("Import..."));
    action->setIcon(KIcon("document-import"));
    connect(action, SIGNAL(triggered()), SLOT(importBook()));

    action = actionCollection->addAction("del_page");
    action->setText(i18n("&Delete Page"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Delete));
    action->setIcon(KIcon("edit-delete-shred"));
    connect(action, SIGNAL(triggered()), SLOT(deletePage()));

    action = actionCollection->addAction("del_folder");
    action->setText(i18n("Delete Boo&k"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Delete));
    action->setIcon(KIcon("edit-delete"));
    connect(action, SIGNAL(triggered()), SLOT(deleteBook()));

    action = actionCollection->addAction("del_mult");
    action->setText(i18n("Delete Selected"));
    action->setIcon(KIcon("edit-delete"));
    connect(action, SIGNAL(triggered()), SLOT(deleteMultiple()));

    action = actionCollection->addAction("manual_save");
    action->setText(i18n("Manual Save"));
    action->setIcon(KIcon("document-save"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    connect(action, SIGNAL(triggered()), SLOT(autoSave()));

    action = actionCollection->addAction("auto_bullet");
    action->setText(i18n("Auto Bullets"));
    action->setIcon(KIcon("format-list-unordered"));
    action->setCheckable(true);

    action = actionCollection->addAction("manage_link");
    action->setText(i18n("Link"));
    action->setIcon(KIcon("insert-link"));

    action = actionCollection->addAction("insert_checkmark");
    action->setText(i18n("Insert Checkmark"));
    action->setIcon(KIcon("checkmark"));
    action->setEnabled(false);

    KStandardAction::print(this, SLOT(onPrint()), actionCollection);
    KStandardAction::quit(this, SLOT(onQuit()), actionCollection);

    action = KStandardAction::cut(editor, SLOT(cut()), actionCollection);
    connect(editor, SIGNAL(copyAvailable(bool)), action, SLOT(setEnabled(bool)));
    action->setEnabled(false);
    action = KStandardAction::copy(editor, SLOT(copy()), actionCollection);
    connect(editor, SIGNAL(copyAvailable(bool)), action, SLOT(setEnabled(bool)));
    action->setEnabled(false);

    action = actionCollection->addAction("copyIntoTitle");
    action->setText(i18n("Copy &into Page Title"));
    action->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_T));
    action->setIcon(KIcon("edit-copy"));
    connect(action, SIGNAL(triggered()), SLOT(copySelection()));
    connect(editor, SIGNAL(copyAvailable(bool)), action, SLOT(setEnabled(bool)));
    action->setEnabled(false);

    KStandardAction::pasteText(editor, SLOT(paste()), actionCollection);

    KStandardAction::find( this, SLOT( onShowSearch() ), actionCollection );
    action = KStandardAction::findNext( this, SLOT( onRepeatSearch() ), actionCollection );
    KStandardAction::replace( this, SLOT( onShowReplace() ), actionCollection );

    action = actionCollection->addAction("rename_entry");
    action->setText(i18n("Rename..."));
    action->setIcon(KIcon("edit-rename"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
    connect(action, SIGNAL(triggered()), SLOT(onRenameEntry()));

    action = actionCollection->addAction("insert_date");
    action->setText(i18n("Insert Date"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
    action->setIcon(KIcon("view-calendar-time-spent"));
    connect(action, SIGNAL(triggered()), SLOT(insertDate()));

    action = actionCollection->addAction("change_color");
    action->setIcon(KIcon("format-fill-color"));
    action->setText(i18n("Change Color..."));

    KStandardAction::preferences(this, SLOT(configure()), actionCollection);

    bookmarkMenu = actionCollection->add<KActionMenu>("bookmarks");
    bookmarkMenu->setText(i18n("&Bookmarks"));
    bookmarkMenu->setIcon(KIcon("bookmarks"));
    KJotsBookmarks* bookmarks = new KJotsBookmarks(bookshelf);
    /*KBookmarkMenu *bmm =*/ new KBookmarkMenu(
        KBookmarkManager::managerForFile(KStandardDirs::locateLocal("data","kjots/bookmarks.xml"), "kjots"),
        bookmarks, bookmarkMenu->menu(), actionCollection);

    m_autosaveTimer = new QTimer(this);

    //
    // Set startup size.
    //
    if (!KJotsSettings::splitterSizes().isEmpty())
    {
        splitter->setSizes(KJotsSettings::splitterSizes());
    }

    updateConfiguration();

    QTimer::singleShot(0, this, SLOT(DelayedInitialization()));

    //connect new slots
    connect(bookshelf, SIGNAL(itemSelectionChanged()), SLOT(updateCaption()));
    connect(bookshelf, SIGNAL(itemSelectionChanged()), SLOT(updateMenu()));
    connect(bookshelf, SIGNAL(itemChanged(QTreeWidgetItem*, int)), SLOT(onItemRenamed(QTreeWidgetItem*, int)));
    connect(m_autosaveTimer, SIGNAL(timeout()), SLOT(autoSave()));
}

void KJotsComponent::DelayedInitialization()
{
    //TODO: Save previous searches in settings file?
    searchDialog = new KFindDialog ( this, 0, QStringList(), false );
    QGridLayout *layout = new QGridLayout(searchDialog->findExtension());
    layout->setMargin(0);
    searchAllPages = new QCheckBox(i18n("Search all pages"), searchDialog->findExtension());
    layout->addWidget(searchAllPages, 0, 0);

    connect(searchDialog, SIGNAL(okClicked()), this, SLOT(onStartSearch()) );
    connect(searchDialog, SIGNAL(cancelClicked()), this, SLOT(onEndSearch()) );
    connect(bookshelf, SIGNAL(itemSelectionChanged()), SLOT(onUpdateSearch()) );
    connect(searchDialog, SIGNAL(optionsChanged()), SLOT(onUpdateSearch()) );
    connect(searchAllPages, SIGNAL(stateChanged(int)), SLOT(onUpdateSearch()) );

    replaceDialog = new KReplaceDialog ( this, 0, searchHistory, replaceHistory, false );
    QGridLayout *layout2 = new QGridLayout(replaceDialog->findExtension());
    layout2->setMargin(0);
    replaceAllPages = new QCheckBox(i18n("Search all pages"), replaceDialog->findExtension());
    layout2->addWidget(replaceAllPages, 0, 0);

    connect(replaceDialog, SIGNAL(okClicked()), this, SLOT(onStartReplace()) );
    connect(replaceDialog, SIGNAL(cancelClicked()), this, SLOT(onEndReplace()) );
    connect(replaceDialog, SIGNAL(optionsChanged()), SLOT(onUpdateReplace()) );
    connect(replaceAllPages, SIGNAL(stateChanged(int)), SLOT(onUpdateReplace()) );

    // Actions that are used only when an entry is selected.
    selectionOnlyActions = new KActionCollection((QObject*)this);
    selectionOnlyActions->addAction( "del_folder", actionCollection->action("del_folder") );
    selectionOnlyActions->addAction( "del_page", actionCollection->action("del_page") );
    selectionOnlyActions->addAction( "del_mult", actionCollection->action("del_mult") );
    selectionOnlyActions->addAction( "save_to", actionCollection->action("save_to") );
    selectionOnlyActions->addAction( "save_to_ascii", actionCollection->action("save_to_ascii") );
    selectionOnlyActions->addAction( "save_to_html", actionCollection->action("save_to_html") );
    selectionOnlyActions->addAction( "insert_date", actionCollection->action("insert_date") );
    selectionOnlyActions->addAction( "rename_entry", actionCollection->action("rename_entry") );
    selectionOnlyActions->addAction( "manual_save", actionCollection->action("manual_save") );
    selectionOnlyActions->addAction( "go_next_book", actionCollection->action("go_next_book") );
    selectionOnlyActions->addAction( "go_next_page", actionCollection->action("go_next_page") );
    selectionOnlyActions->addAction( "go_prev_book", actionCollection->action("go_prev_book") );
    selectionOnlyActions->addAction( "go_prev_page", actionCollection->action("go_prev_page") );
    selectionOnlyActions->addAction( "new_page", actionCollection->action("new_page") );
    selectionOnlyActions->addAction( KStandardAction::name(KStandardAction::Cut),
                                     actionCollection->action(KStandardAction::name(KStandardAction::Cut)) );
    selectionOnlyActions->addAction( KStandardAction::name(KStandardAction::Paste),
                                     actionCollection->action(KStandardAction::name(KStandardAction::Paste)) );
    selectionOnlyActions->addAction( KStandardAction::name(KStandardAction::Replace),
                                     actionCollection->action(KStandardAction::name(KStandardAction::Replace)) );
    selectionOnlyActions->addAction( KStandardAction::name(KStandardAction::Find),
                                     actionCollection->action(KStandardAction::name(KStandardAction::Find)) );
    selectionOnlyActions->addAction( KStandardAction::name(KStandardAction::Print),
                                     actionCollection->action(KStandardAction::name(KStandardAction::Print)) );
    selectionOnlyActions->addAction( "change_color", actionCollection->action("change_color") );

    // Actions that are used only when a page is selected.
    pageOnlyActions = new KActionCollection((QObject*)this);
    pageOnlyActions->addAction( KStandardAction::name(KStandardAction::Cut),
                                actionCollection->action(KStandardAction::name(KStandardAction::Cut)) );
    pageOnlyActions->addAction( KStandardAction::name(KStandardAction::Paste),
                                actionCollection->action(KStandardAction::name(KStandardAction::Paste)) );
    pageOnlyActions->addAction( KStandardAction::name(KStandardAction::Replace),
                                actionCollection->action(KStandardAction::name(KStandardAction::Replace)) );
    pageOnlyActions->addAction( "del_page", actionCollection->action("del_page") );
    pageOnlyActions->addAction( "insert_date", actionCollection->action("insert_date") );
    pageOnlyActions->addAction( "auto_bullet", actionCollection->action("auto_bullet") );
    pageOnlyActions->addAction( "manage_link", actionCollection->action("manage_link") );
    pageOnlyActions->addAction( "insert_checkmark", actionCollection->action("insert_checkmark") );

    // Actions that are used only when a page is selected.
    bookOnlyActions = new KActionCollection((QObject*)this);
    bookOnlyActions->addAction( "save_to_book", actionCollection->action("save_to_book") );

    bookshelf->DelayedInitialization(actionCollection);
    editor->DelayedInitialization(actionCollection, bookshelf);
    browser->DelayedInitialization(bookshelf);

    if (bookshelf->topLevelItemCount() == 0) {
        if ( !createNewBook() ) {
            editor->setEnabled(false);
        }
    } else {
        quint64 currentSelection = KJotsSettings::currentSelection();
        bookshelf->jumpToId(currentSelection);
    }
}

inline QTextEdit* KJotsComponent::activeEditor() {
    if ( browser->isVisible() ) {
        return browser;
    } else {
        return editor;
    }
}

bool KJotsComponent::createNewBook()
{
    KJotsBook* book = KJotsBook::createNewBook();
    bool success = false;

    if ( book ) {
        bookshelf->addTopLevelItem(book);
        bookshelf->setItemExpanded(book, true);
        QTreeWidgetItem *item = static_cast<QTreeWidgetItem*>(book);
        bookshelf->clearSelection();
        bookshelf->setItemSelected(item->child(0), true);
        success = true;
    }

    return success;
}

void KJotsComponent::onRenameEntry()
{
    KJotsEntry* entry = dynamic_cast<KJotsEntry*>(bookshelf->currentItem());

    if (entry)
    {
        entry->rename();
    }
}

/*!
*   Deletes the current book or the book that owns the current page.
*/
void KJotsComponent::deleteBook()
{
    KJotsBook *book = bookshelf->currentBook();
    if ( !book ) return;

    if ( KMessageBox::warningContinueCancel(topLevelWidget(),
        i18n("<qt>Are you sure you want to delete the <strong>%1</strong> book?</qt>", book->title()),
        i18n("Delete"), KStandardGuiItem::del(), KStandardGuiItem::cancel(), "DeleteBookWarning") == KMessageBox::Cancel) {
        return;
    }

    bookshelf->remove(book);
    updateMenu();
}

/*!
*   Deletes the current page and ONLY the current Page.
*/
void KJotsComponent::deletePage()
{
    KJotsPage *page = bookshelf->currentPage();
    if ( !page ) return;

    if ( KMessageBox::warningContinueCancel(topLevelWidget(),
        i18n("<qt>Are you sure you want to delete the <strong>%1</strong> page?</qt>", page->title()),
        i18n("Delete"), KStandardGuiItem::del(), KStandardGuiItem::cancel(), "DeletePageWarning") == KMessageBox::Cancel) {
        return;
    }

    bookshelf->remove(page);
    updateMenu();
}

/*!
*   Deletes anything selected.
*/
void KJotsComponent::deleteMultiple()
{
    QList<QTreeWidgetItem*> selection = bookshelf->selectedItems();
    if ( selection.size() <= 1 ) return; //sanity check

    if ( KMessageBox::warningContinueCancel(topLevelWidget(),
        i18n("<qt>Are you sure you want to delete these entries?</qt>"),
        i18n("Delete"), KStandardGuiItem::del(), KStandardGuiItem::cancel(), "DeleteMultipleWarning") == KMessageBox::Cancel) {
        return;
    }

    foreach ( QTreeWidgetItem *item, selection ) {
        bookshelf->remove(item);
    }

    updateMenu();
}

void KJotsComponent::newPage()
{
    KJotsBook* book = bookshelf->currentBook();

    if (book) {
        KJotsPage *page = book->addPage();
        bookshelf->clearSelection();
        bookshelf->setItemSelected(page, true);
    }

}

void KJotsComponent::configure()
{
    if(KConfigDialog::showDialog("settings"))
        return;

    KConfigDialog* dialog = new KConfigDialog(this, "settings", KJotsSettings::self());

    confPageMisc* cpMisc = new confPageMisc(0);
    dialog->addPage(cpMisc, i18n("Autosave"), "document-save");

    connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(updateConfiguration()));

    dialog->show();
}

/*!
    Shows the search dialog when "Find" is selected.
*/
void KJotsComponent::onShowSearch()
{
    onUpdateSearch();

    QTextEdit *browserOrEditor = activeEditor();

    if ( browserOrEditor->textCursor().hasSelection() ) {
        searchDialog->setHasSelection(true);
        long dialogOptions = searchDialog->options();
        dialogOptions |= KFind::SelectedText;
        searchDialog->setOptions(dialogOptions);
    } else {
        searchDialog->setHasSelection(false);
    }

    searchDialog->setFindHistory(searchHistory);
    searchDialog->show();
    onUpdateSearch();
}

/*!
    Updates the search dialog if the user is switching selections while it is open.
*/
void KJotsComponent::onUpdateSearch()
{
    if ( searchDialog->isVisible() ) {
        long searchOptions = searchDialog->options();
        if ( searchOptions & KFind::SelectedText ) {
            searchAllPages->setCheckState(Qt::Unchecked);
            searchAllPages->setEnabled(false);
        } else {
            searchAllPages->setEnabled(true);
        }

        if ( searchAllPages->checkState() == Qt::Checked ) {
            searchOptions &= ~KFind::SelectedText;
            searchDialog->setOptions(searchOptions);
            searchDialog->setHasSelection(false);
        } else {
            if ( activeEditor()->textCursor().hasSelection() ) {
                searchDialog->setHasSelection(true);
            }
        }

        if ( activeEditor()->textCursor().hasSelection() ) {
            if ( searchAllPages->checkState() == Qt::Unchecked ) {
                searchDialog->setHasSelection(true);
            }
        } else {
            searchOptions &= ~KFind::SelectedText;
            searchDialog->setOptions(searchOptions);
            searchDialog->setHasSelection(false);
        }
    }
}

/*!
    Called when the user presses OK in the search dialog.
*/
void KJotsComponent::onStartSearch()
{
    QString searchPattern = searchDialog->pattern();
    if ( !searchHistory.contains ( searchPattern ) ) {
        searchHistory.prepend(searchPattern);
    }

    QTextEdit *browserOrEditor = activeEditor();
    QTextCursor cursor = browserOrEditor->textCursor();

    long searchOptions = searchDialog->options();
    if ( searchOptions & KFind::FromCursor ) {
        searchPos = cursor.position();
        searchBeginPos = 0;
        cursor.movePosition(QTextCursor::End);
        searchEndPos = cursor.position();
    } else {
        if ( searchOptions & KFind::SelectedText ) {
            searchBeginPos = cursor.selectionStart();
            searchEndPos = cursor.selectionEnd();
        } else {
            searchBeginPos = 0;
            cursor.movePosition(QTextCursor::End);
            searchEndPos = cursor.position();
        }

        if ( searchOptions & KFind::FindBackwards ) {
            searchPos = searchEndPos;
        } else {
            searchPos = searchBeginPos;
        }
    }

    onRepeatSearch();
}

/*!
    Called when user chooses "Find Next"
*/
void KJotsComponent::onRepeatSearch()
{
    if ( search(false) == 0 ) {
        KMessageBox::sorry(0, i18n("<qt>No matches found.</qt>"));
    }
}

/*!
    Called when user presses Cancel in find dialog. Just a placeholder for now.
*/
void KJotsComponent::onEndSearch()
{
}

/*!
    Shows the replace dialog when "Replace" is selected.
*/
void KJotsComponent::onShowReplace()
{
    Q_ASSERT(editor->isVisible());

    if ( editor->textCursor().hasSelection() ) {
        replaceDialog->setHasSelection(true);
        long dialogOptions = replaceDialog->options();
        dialogOptions |= KFind::SelectedText;
        replaceDialog->setOptions(dialogOptions);
    } else {
        replaceDialog->setHasSelection(false);
    }

    replaceDialog->setFindHistory(searchHistory);
    replaceDialog->setReplacementHistory(replaceHistory);
    replaceDialog->show();
    onUpdateReplace();
}

/*!
    Updates the replace dialog if the user is switching selections while it is open.
*/
void KJotsComponent::onUpdateReplace()
{
    if ( replaceDialog->isVisible() ) {
        long replaceOptions = replaceDialog->options();
        if ( replaceOptions & KFind::SelectedText ) {
            replaceAllPages->setCheckState(Qt::Unchecked);
            replaceAllPages->setEnabled(false);
        } else {
            replaceAllPages->setEnabled(true);
        }

        if ( replaceAllPages->checkState() == Qt::Checked ) {
            replaceOptions &= ~KFind::SelectedText;
            replaceDialog->setOptions(replaceOptions);
            replaceDialog->setHasSelection(false);
        } else {
            if ( activeEditor()->textCursor().hasSelection() ) {
                replaceDialog->setHasSelection(true);
            }
        }
    }
}

/*!
    Called when the user presses OK in the replace dialog.
*/
void KJotsComponent::onStartReplace()
{
    QString searchPattern = replaceDialog->pattern();
    if ( !searchHistory.contains ( searchPattern ) ) {
        searchHistory.prepend(searchPattern);
    }

    QString replacePattern = replaceDialog->replacement();
    if ( !replaceHistory.contains ( replacePattern ) ) {
        replaceHistory.prepend(replacePattern);
    }

    QTextCursor cursor = editor->textCursor();

    long replaceOptions = replaceDialog->options();
    if ( replaceOptions & KFind::FromCursor ) {
        replacePos = cursor.position();
        replaceBeginPos = 0;
        cursor.movePosition(QTextCursor::End);
        replaceEndPos = cursor.position();
    } else {
        if ( replaceOptions & KFind::SelectedText ) {
            replaceBeginPos = cursor.selectionStart();
            replaceEndPos = cursor.selectionEnd();
        } else {
            replaceBeginPos = 0;
            cursor.movePosition(QTextCursor::End);
            replaceEndPos = cursor.position();
        }

        if ( replaceOptions & KFind::FindBackwards ) {
            replacePos = replaceEndPos;
        } else {
            replacePos = replaceBeginPos;
        }
    }

    replaceStartPage = bookshelf->currentPage();

    //allow KReplaceDialog to exit so the user can see.
    QTimer::singleShot(0, this, SLOT(onRepeatReplace()));
}

/*!
    Only called after onStartReplace. Kept the name scheme for consistancy.
*/
void KJotsComponent::onRepeatReplace()
{
    KJotsReplaceNextDialog *dlg = 0;

    QString searchPattern = replaceDialog->pattern();
    QString replacePattern = replaceDialog->replacement();
    int found = 0;
    int replaced = 0;

    long replaceOptions = replaceDialog->options();
    if ( replaceOptions & KReplaceDialog::PromptOnReplace ) {
        dlg = new KJotsReplaceNextDialog(this);
    }

    forever {
        if ( !search(true) ) {
            break;
        }

        QTextCursor cursor = editor->textCursor();
        if ( !cursor.hasSelection() ) {
            break;
        } else {
            ++found;
        }

        QString replacementText = replacePattern;
        if ( replaceOptions & KReplaceDialog::BackReference ) {
            QRegExp regExp ( searchPattern, (replaceOptions & Qt::CaseSensitive) ?
                Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::RegExp2 );
            regExp.indexIn(cursor.selectedText());
            int capCount = regExp.numCaptures();
            for ( int i=0; i<=capCount; i++ ) {
                QString c = QString( "\\%1" ).arg(i);
                replacementText.replace(c, regExp.cap(i));
            }
        }

        if ( replaceOptions & KReplaceDialog::PromptOnReplace ) {
            dlg->setLabel(cursor.selectedText(), replacementText);

            if ( !dlg->exec() ) {
                break;
            }

            if ( dlg->answer() != KDialog::User2 ) {
                cursor.insertText(replacementText);
                editor->setTextCursor(cursor);
                ++replaced;
            }

            if ( dlg->answer() == KDialog::User1 ) {
                replaceOptions |= ~KReplaceDialog::PromptOnReplace;
            }
        } else {
            cursor.insertText(replacementText);
            editor->setTextCursor(cursor);
            ++replaced;
        }
    }

    if (replaced == found)
    {
        KMessageBox::information(0, i18np("<qt>Replaced 1 occurrence.</qt>", "<qt>Replaced %1 occurrences.</qt>", replaced));
    }
    else if (replaced < found)
    {
        KMessageBox::information(0,
                i18np("<qt>Replaced %2 of 1 occurrence.</qt>", "<qt>Replaced %2 of %1 occurrences.</qt>", found, replaced));
    }

    if ( dlg ) {
        delete dlg;
    }
}

/*!
    Called when user presses Cancel in replace dialog. Just a placeholder for now.
*/
void KJotsComponent::onEndReplace()
{
}

/*!
    Searches for the given pattern, with the given options. This is huge and
    unwieldly function, but the operation we're performing is huge and unwieldly.
*/
int KJotsComponent::search( bool replacing )
{
    int rc = 0;
    int *beginPos = replacing ? &replaceBeginPos : &searchBeginPos;
    int *endPos = replacing ? &replaceEndPos : &searchEndPos;
    long options = replacing ? replaceDialog->options() : searchDialog->options();
    QString pattern = replacing ? replaceDialog->pattern() : searchDialog->pattern();
    int *curPos = replacing ? &replacePos : &searchPos;
    KJotsPage *startPage = replacing ? replaceStartPage : bookshelf->currentPage();

    bool allPages = false;
    QCheckBox *box = replacing ? replaceAllPages : searchAllPages;
    if ( box->isEnabled() && box->checkState() == Qt::Checked ) {
        allPages = true;
    }

    QTextDocument::FindFlags findFlags = 0;
    if ( options & Qt::CaseSensitive ) {
        findFlags |= QTextDocument::FindCaseSensitively;
    }

    if ( options & KFind::WholeWordsOnly ) {
        findFlags |= QTextDocument::FindWholeWords;
    }

    if ( options & KFind::FindBackwards ) {
        findFlags |= QTextDocument::FindBackward;
    }

    // We will find a match or return 0
    int attempts = 0;
    forever {
        ++attempts;

        QTextEdit *browserOrEditor = activeEditor();
        QTextDocument *theDoc = browserOrEditor->document();

        QTextCursor cursor;
        if ( options & KFind::RegularExpression ) {
            QRegExp regExp ( pattern, (options & Qt::CaseSensitive) ?
                Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::RegExp2 );
            cursor = theDoc->find(regExp, *curPos, findFlags);
        } else {
            cursor = theDoc->find(pattern, *curPos, findFlags);
        }

        if ( cursor.hasSelection() ) {
            if ( cursor.selectionStart() >= *beginPos && cursor.selectionEnd() <= *endPos ) {
                browserOrEditor->setTextCursor(cursor);
                browserOrEditor->ensureCursorVisible();
                *curPos = (options & KFind::FindBackwards) ?
                    cursor.selectionStart() : cursor.selectionEnd();
                rc = 1;
                break;
            }
        }

        //No match. Determine what to do next.

        if ( replacing && !(options & KFind::FromCursor) && !allPages) {
            break;
        }

        if ( (options & KFind::FromCursor) && !allPages) {
            if ( KMessageBox::questionYesNo(this,
                i18n("<qt>End of search area reached. Do you want to wrap around and continue?</qt>")) ==
                KMessageBox::No ) {
                rc = 3;
                break;
            }
        }

        if ( allPages ) {
            if ( options & KFind::FindBackwards ) {
                bookshelf->prevPage();
            } else {
                bookshelf->nextPage();
            }

            if ( startPage == bookshelf->currentPage() ) {
                rc = 0;
                break;
            }

            *beginPos = 0;
            cursor = editor->textCursor();
            cursor.movePosition(QTextCursor::End);
            *endPos = cursor.position();
            *curPos = (options & KFind::FindBackwards) ? *endPos : *beginPos;
            continue;
        }

        //By now, we should have figured out what to do. In all remaining cases we
        //will automatically loop and try to "find next" from the top/bottom, because
        //I like this behavior the best.
        if ( attempts <= 1 ) {
            *curPos = (options & KFind::FindBackwards) ? *endPos : *beginPos;
        } else {
            //We've already tried the loop and failed to find anything. Bail.
            rc = 0;
            break;
        }
    }

    return rc;
}

void KJotsComponent::updateConfiguration()
{
   static int encoding = -1;

   //Keep track of the encoding setting. If the user changes it, we
   //need to mark all books dirty so they are saved correctly.
   if ( encoding == -1 ) {
       encoding = KJotsSettings::unicode();
   } else {
       if ( (bool)encoding != KJotsSettings::unicode() ) {
           for ( int i=0; i<bookshelf->topLevelItemCount(); i++ ) {
               KJotsBook *book = dynamic_cast<KJotsBook*>(bookshelf->topLevelItem(i));
               if (book) {
                   book->setDirty(true);
               }
            }

           encoding = (int)KJotsSettings::unicode();
       }
   }

    if (KJotsSettings::autoSave())
        m_autosaveTimer->start(KJotsSettings::autoSaveInterval()*1000*60);
    else
        m_autosaveTimer->stop();
}

/*!
    \brief Saves any books that need saving.
*/
void KJotsComponent::autoSave()
{
    for ( int i=0; i<bookshelf->topLevelItemCount(); i++ ) {
        KJotsBook *book = dynamic_cast<KJotsBook*>(bookshelf->topLevelItem(i));
        if (book && book->dirty()) {
            book->saveBook();
        }
    }
}

void KJotsComponent::saveAscii()
{
    saveToFile(Ascii);
}

void KJotsComponent::saveHtml()
{
    saveToFile(HTML);
}

void KJotsComponent::saveNative()
{
    saveToFile(Native);
}

void KJotsComponent::saveToFile(KJotsComponent::ExportType type)
{
    autoSave();

    QString title;
    QList<KJotsEntry*> entries = bookshelf->selected();
    Q_ASSERT(entries.size());

    if ( entries.size() == 1 ) {
        title = entries[0]->title();
    } else {
        title = i18n("Multiple Selections");
    }

    KUrl saveUrl;
    QString encoding;
    if ( type == Native ) {
        KUrl startLocation = KUrl::fromPath(title + ".book");
        saveUrl = KFileDialog::getSaveUrl(startLocation, "*.book|" + i18n("KJots Books"));
    } else if ( type == HTML ) {
        KEncodingFileDialog::Result res;
        res = KEncodingFileDialog::getSaveUrlAndEncoding(
            QString("UTF-8"), title + ".html", "*.html|" + i18n("HTML Files"));
        saveUrl = res.URLs[0];
        encoding = res.encoding;
    } else if ( type == Ascii ) {
        KEncodingFileDialog::Result res;
        res = KEncodingFileDialog::getSaveUrlAndEncoding(QString("UTF-8"), title);
        saveUrl = res.URLs[0];
        encoding = res.encoding;
    }

    //Create an interim file for us to write to
    std::auto_ptr<KTemporaryFile> interimFile ( new KTemporaryFile );
    interimFile->setAutoRemove(false);
    interimFile->setPermissions(
            QFile::ReadUser|QFile::WriteUser|QFile::ReadGroup|QFile::ReadOther);
    if ( !interimFile->open() ) {
        KMessageBox::error(0, i18n("<qt>Error opening temporary file.</qt>"));
        return;
    }

    KUrl tempUrl = KUrl::fromPath(interimFile->fileName());

    if (!saveUrl.isEmpty()) {
        if ((type == HTML) || (type == Ascii))
        {
            QTextCodec *codec = QTextCodec::codecForName(encoding.toAscii());
            QByteArray data;
            if ( type == HTML ) {
                QTextDocument doc;
                QTextCursor cur ( &doc );

                foreach ( KJotsEntry *entry, entries ) {
                    entry->generateHtml ( entry, true, &cur );
                }

                doc.setMetaInformation(QTextDocument::DocumentTitle, title);
                data = codec->fromUnicode(doc.toHtml(encoding.toAscii()));

            }
            else if ( type == Ascii ) {
                foreach ( KJotsEntry *entry, entries ) {
                    data += codec->fromUnicode(entry->generateText());
                }

            }

            interimFile->write(data);
            interimFile->close();
        }

        else if ( type == Native ) {
            foreach ( KJotsEntry *entry, entries ) {
                KJotsBook *book = dynamic_cast<KJotsBook*>(entry);
                if ( !book ) continue;

                if ( book->dirty() ) {
                    book->saveBook();
                }

                QFile sourceFile ( book->fileName() );
                if ( !sourceFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
                    KMessageBox::error(0, i18n("<qt>Error opening internal file.</qt>"));
                    return;
                }

                QTextStream inStream ( &sourceFile );
                inStream.setCodec("UTF-8");
                QTextStream outStream ( interimFile.get() );
                outStream.setCodec("UTF-8");

                //Remove IDs from the file
                while ( !inStream.atEnd() ) {
                    QString line = inStream.readLine();
                    QString trimmed = line.trimmed();

                    if ( trimmed.startsWith("<ID>") && trimmed.endsWith("</ID>") ) {
                        outStream << "<ID>0</ID>\n";
                    } else {
                        outStream << line << '\n';
                    }
                }
                outStream.flush();
            }
        }

        interimFile.release();

        KJob *job = KIO::move(tempUrl, saveUrl);
        connect( job, SIGNAL( result(KJob*) ), this, SLOT( saveFinished(KJob*) ) );
    }

    return;
}

void KJotsComponent::saveFinished(KJob *job)
{
    //Remove the temporary file if the job was cancelled
    if ( job->error() ) {
        KIO::CopyJob *copyJob = static_cast<KIO::CopyJob*>(job);
        QFile::remove(copyJob->srcUrls()[0].toLocalFile());
    }
}

void KJotsComponent::importBook()
{
    KUrl openUrl = KFileDialog::getOpenUrl(KUrl(), "*.book|" + i18n("KJots Books"));

    if (!openUrl.isEmpty()) {
        KTemporaryFile file;
        file.setPrefix(KStandardDirs::locateLocal("data","kjots"));
        file.setSuffix(".book");
        file.setAutoRemove(false);

        if ( file.open() ) {
            KUrl bookUrl = KUrl::fromPath(file.fileName());
            KIO::Job* job = KIO::file_copy(openUrl, bookUrl, 0644, KIO::Overwrite);
            if ( job->exec() ) {
                KJotsBook* book = new KJotsBook();
                bookshelf->addTopLevelItem(book);
                book->openBook(file.fileName());
            }
        }
    }
}

void KJotsComponent::copySelection()
{
    QString newTitle( editor->textCursor().selectedText() );

    if ( !newTitle.isEmpty() ) {
        KJotsEntry* entry = dynamic_cast<KJotsEntry*>(bookshelf->currentItem());
        if ( entry ) {
            entry->setTitle(newTitle);
            entry->topLevelBook()->setDirty(true);
        }
    }
}

/*!
    Prints all the selected entries.
*/
void KJotsComponent::onPrint()
{
    autoSave();

    QPrinter *printer = new QPrinter();
    printer->setDocName("KJots Print");
    printer->setFullPage(false);
    printer->setCreator("KJots");
    //Not supported in Qt?
    //printer->setPageSelection(QPrinter::ApplicationSide);

    //KPrinter::pageList() only works with ApplicationSide. ApplicationSide
    //requires min/max pages. How am I supposed to tell how many pages there
    //are before I setup the printer?

    QPrintDialog printDialog(printer, this);
    printDialog.setWindowTitle(i18n("Send To Printer"));
    if (printDialog.exec()) {
        QTextDocument printDocument;
        QTextCursor printCursor ( &printDocument );

        foreach ( KJotsEntry *entry, bookshelf->selected() ) {
            entry->generatePrintData ( &printCursor );
        }

        QPainter p(printer);

        // Check that there is a valid device to print to.
        if (p.isActive()) {
            QTextDocument *doc = &printDocument;

            QRectF body = QRectF(QPointF(0, 0), doc->pageSize());
            QPointF pageNumberPos;

            QAbstractTextDocumentLayout *layout = doc->documentLayout();
            layout->setPaintDevice(p.device());

            const int dpiy = p.device()->logicalDpiY();

            const int margin = (int) ((2/2.54)*dpiy); // 2 cm margins
            QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
            fmt.setMargin(margin);
            doc->rootFrame()->setFrameFormat(fmt);

            body = QRectF(0, 0, p.device()->width(), p.device()->height());
            pageNumberPos = QPointF(body.width() - margin,
                            body.height() - margin
                            + QFontMetrics(doc->defaultFont(), p.device()).ascent()
                            + 5 * p.device()->logicalDpiY() / 72);

            doc->setPageSize(body.size());

            int docCopies = printer->numCopies();
            for (int copy = 0; copy < docCopies; ++copy) {

                int lastPage = layout->pageCount();
                for ( int page = 1; page <= lastPage ; page++ ) {
                    p.save();
                    p.translate(body.left(), body.top() - (page - 1) * body.height());
                    QRectF view(0, (page - 1) * body.height(), body.width(), body.height());

                    QAbstractTextDocumentLayout *layout = doc->documentLayout();
                    QAbstractTextDocumentLayout::PaintContext ctx;

                    p.setClipRect(view);
                    ctx.clip = view;

                    // don't use the system palette text as default text color, on HP/UX
                    // for example that's white, and white text on white paper doesn't
                    // look that nice
                    ctx.palette.setColor(QPalette::Text, Qt::black);

                    layout->draw(&p, ctx);

                    if (!pageNumberPos.isNull()) {
                        p.setClipping(false);
                        p.setFont(QFont(doc->defaultFont()));
                        const QString pageString = QString::number(page);

                        p.drawText(qRound(pageNumberPos.x() - p.fontMetrics().width(pageString)),
                          qRound(pageNumberPos.y() + view.top()),
                          pageString);
                    }

                    p.restore();

                    if ( (page+1) <= lastPage ) {
                        printer->newPage();
                    }
                }
            }
        }
    }

    delete printer;
}

void KJotsComponent::onQuit()
{
    close();
}

void KJotsComponent::insertDate()
{
    editor->insertPlainText(KGlobal::locale()->formatDateTime(QDateTime::currentDateTime(), KLocale::ShortDate) + ' ');
}

void KJotsComponent::updateMenu()
{
    // Sanity check. Too many things call updateMenu().
    if ( !selectionOnlyActions || !pageOnlyActions ) {
        return;
    }

    QList<QTreeWidgetItem*> selection = bookshelf->selectedItems();
    int selectionSize = selection.size();

    if ( !selectionSize ) {
        // no (meaningful?) selection
        foreach ( QAction* action, selectionOnlyActions->actions() )
            action->setEnabled(false);
        foreach ( QAction* action, pageOnlyActions->actions() )
            action->setEnabled(false);
        foreach ( QAction* action, bookOnlyActions->actions() )
            action->setEnabled(false);
        editor->setActionsEnabled( false );
    } else if ( selectionSize > 1 ) {
        // soon...
    } else { //selectionSize == 1
        foreach ( QAction* action, selectionOnlyActions->actions() )
            action->setEnabled(true);

        KJotsEntry* entry = static_cast<KJotsEntry*>(selection.at(0));
        if ( entry && entry->isBook() ) {
            foreach ( QAction* action, pageOnlyActions->actions() )
                action->setEnabled(false);
            foreach ( QAction* action, bookOnlyActions->actions() )
                action->setEnabled(true);
            editor->setActionsEnabled( false );
        } else {
            foreach ( QAction* action, pageOnlyActions->actions() )
                action->setEnabled(true);
            foreach ( QAction* action, bookOnlyActions->actions() )
                action->setEnabled(false);
            editor->setActionsEnabled( true );
            
        }
    }
}

/*!
    \brief Called just before KJots is closed.
*/
bool KJotsComponent::queryClose()
{
    autoSave();
    bookshelf->prepareForExit();

    KJotsSettings::setSplitterSizes(splitter->sizes());

    KJotsSettings::self()->writeConfig();
    return true;
}

QString KJotsComponent::currentCaption()
{
    return bookshelf->currentCaption(" / ");
}

/*!
    Sets the window caption.
*/
void KJotsComponent::updateCaption()
{
    emit captionChanged(bookshelf->currentCaption(" / "));
}

void KJotsComponent::onItemRenamed(QTreeWidgetItem* item, int  /*col*/)
{
    if ( item ) {
        emit captionChanged(bookshelf->currentCaption(" / "));
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
