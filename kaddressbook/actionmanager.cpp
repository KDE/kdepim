/*
  This file is part of KAddressBook.
  Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include <qapplication.h>
#include <qclipboard.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>

#include "actionmanager.h"
#include "kaddressbook.h"
#include "viewmanager.h"
#include "undo.h"

ActionManager::ActionManager(KXMLGUIClient *client, KAddressBook *widget,
                             bool readWrite, QObject *parent)
    : QObject(parent)
{
    mGUIClient = client;
    mACollection = mGUIClient->actionCollection();

    mWidget = widget;
    connect( mWidget, SIGNAL( addresseeSelected( bool ) ),
             SLOT( addresseeSelected( bool ) ) );
    connect( mWidget, SIGNAL( modified( bool ) ),
             SLOT( modified( bool ) ) );

    mViewManager = mWidget->viewManager();
    connect( mViewManager, SIGNAL( viewConfigChanged(const QString &) ),
             SLOT( viewConfigChanged(const QString &) ) );

    connect( QApplication::clipboard(), SIGNAL( dataChanged() ),
             SLOT( clipboardDataChanged() ) );

    mReadWrite = readWrite;
    initReadOnlyActions();
    if (mReadWrite)
        initReadWriteActions();

    // Read our own config
    KConfig *config = kapp->config();
    config->setGroup("Views");
    mActiveViewName = config->readEntry("Active");
    config->setGroup("MainWindow");
    mActionJumpBar->setChecked(config->readBoolEntry("JumpBar", false));
    mActionFeatures->setChecked(config->readBoolEntry("Features", false));
    mActionDetails->setChecked(config->readBoolEntry("Details", true));
    // Set the defaults
    addresseeSelected(false);
    modified(false);
    quickToolsAction();

    mActionViewList.setAutoDelete(true);

    // Connect to the signals from the undo/redo stacks so we can update the
    // edit menu
    connect(UndoStack::instance(), SIGNAL(changed()), SLOT(updateEditMenu()));
    connect(RedoStack::instance(), SIGNAL(changed()), SLOT(updateEditMenu()));
}

ActionManager::~ActionManager()
{
    // Write our own config
    KConfig *config = kapp->config();

    config->setGroup("Views");
    config->writeEntry("Active", mActiveViewName);

    config->setGroup("MainWindow");
    config->writeEntry("JumpBar", mActionJumpBar->isChecked());
    config->writeEntry("Features", mActionFeatures->isChecked());
    config->writeEntry("Details", mActionDetails->isChecked());

    config->sync();
}

void ActionManager::setReadWrite(bool rw)
{
    if (rw == mReadWrite)
        return;

    mReadWrite = rw;
    if (mReadWrite)
        initReadWriteActions();
    else
        destroyReadWriteActions();
}


void ActionManager::clipboardDataChanged()
{
    if (mReadWrite)
        mActionPaste->setEnabled( !QApplication::clipboard()->text().isEmpty() );
}

void ActionManager::initReadOnlyActions()
{
    // File menu
    mActionSave = new KAction(i18n("&Save"), "filesave", CTRL+Key_S, mWidget,
                              SLOT(save()), mACollection,"file_sync");

    new KAction(i18n("&New Contact..."), "filenew", CTRL+Key_N, mWidget,
                SLOT(newAddressee()),mACollection,"file_new_contact");

    mActionMail = KStdAction::mail(mViewManager, SLOT(sendMail()),
                                   mACollection);
    mActionEditAddressee = new KAction(i18n("&Edit Contact..."), "edit", 0,
                                       mWidget, SLOT(editAddressee()),
                                       mACollection, "file_properties");

    KStdAction::print(mWidget, SLOT(print()), mACollection);

    new KAction(i18n("Import &KDE 2 Address Book..."), 0, mWidget,
                SLOT(importKDE2()), mACollection, "file_import_kde2");

    new KAction(i18n("Import vCard..."), 0, mWidget, SLOT(importVCardSimple()),
                mACollection, "file_import_vcard");

    new KAction(i18n("&Import List..."), 0, mWidget, SLOT(importCSV()),
                mACollection, "file_import_csv");

    new KAction(i18n("&Export List..."), 0, mWidget, SLOT(exportCSV()),
                mACollection, "file_export_csv");

    new KAction(i18n("&Export vCard 3.0..."), 0, mWidget, SLOT(exportVCard30()),
                mACollection, "file_export_vcard30");

    // Edit menu
    mActionCopy = KStdAction::copy(mViewManager, SLOT(copy()),
                                   mACollection);

    KStdAction::selectAll(mViewManager, SLOT(setSelected()), mACollection);

    mActionDelete = new KAction(i18n("&Delete Contact"), "editdelete",
                                Key_Delete, mViewManager,
                                SLOT(deleteAddressee()), mACollection,
                                "edit_delete");

    mActionUndo = KStdAction::undo(mWidget, SLOT(undo()), mACollection);
    mActionUndo->setEnabled(false);

    mActionRedo = KStdAction::redo(mWidget, SLOT(redo()), mACollection);
    mActionRedo->setEnabled( false );

    // View menu
    new KAction(i18n("Modify View..."), "configure", 0, mViewManager,
                SLOT(modifyView()), mACollection,
                "view_modify");
    new KAction(i18n("Add View..."), "window_new", 0, mViewManager,
                SLOT(addView()), mACollection,
                "view_add");
    mActionDeleteView = new KAction(i18n("Delete View"), "view_remove", 0,
                                    mViewManager,
                                    SLOT(deleteView()), mACollection,
                                    "view_delete");
    new KAction(i18n("Refresh View"), "reload", 0, mViewManager,
                SLOT(refresh()), mACollection,
                "view_refresh");

    // Only enable LDAP lookup if we can handle the protocol
    if( KProtocolInfo::isKnownProtocol( KURL("ldap://localhost") )) {
        // LDAP button on toolbar
        new KAction(i18n("&Lookup addresses in directory"),
                    "ldap_lookup", 0, mWidget, SLOT(slotOpenLDAPDialog()),
                    mACollection,"ldap_lookup");
    }

    // Settings menu
    mActionJumpBar = new KToggleAction(i18n("Show Jump Bar"), "next", 0,
                                       this, SLOT(quickToolsAction()),
                                       mACollection,
                                       "options_show_jump_bar");
    mActionFeatures = new KToggleAction(i18n("Show Features Bar"), "features",
                                        0, this, SLOT(quickToolsAction()),
                                        mACollection,
                                        "options_show_features");
    mActionDetails = new KToggleAction(i18n("Show Details"), "details",
                                       0, this, SLOT(quickToolsAction()),
                                       mACollection,
                                       "options_show_details");
    (void) new KAction(i18n("Edit &Filters..."), "filter",
                       0, mWidget, SLOT(configureFilters()),
                       mACollection, "options_edit_filters");
    mActionSelectFilter = new KSelectAction(i18n("Select Filter"), 0,
                                            0,
                                            this, SLOT(slotFilterActivated(int)),
                                            mACollection, "select_filter");
#if KDE_VERSION >= 307
    mActionSelectFilter->setRemoveAmpersandsInCombo( true );
#endif

    connect(mActionSelectFilter, SIGNAL(activated(int)),
            SLOT(slotFilterActivated(int)));
    connect(this, SIGNAL(filterActivated(int)),
            mViewManager, SLOT(filterActivated(int)));
    connect(mViewManager, SIGNAL(setFilterNames(const QStringList&)),
            SLOT(setFilterNames(const QStringList&)));
    connect(mViewManager, SIGNAL(setCurrentFilterName(const QString&)),
            SLOT(setCurrentFilterName(const QString&)));
    connect(mViewManager, SIGNAL(setCurrentFilter(int)),
            SLOT(setCurrentFilter(int)));
}

void ActionManager::initReadWriteActions()
{
    // Edit menu
    mActionCut = KStdAction::cut(mViewManager, SLOT(cut()), mACollection);
    mActionPaste = KStdAction::paste(mViewManager, SLOT(paste()),
                                     mACollection);
    clipboardDataChanged();
}

void ActionManager::destroyReadWriteActions()
{
    delete mActionCut;
    delete mActionPaste;
}

void ActionManager::updateEditMenu()
{
    UndoStack *undo = UndoStack::instance();
    RedoStack *redo = RedoStack::instance();

    if (undo->isEmpty())
        mActionUndo->setText( i18n( "Undo" ) );
    else
        mActionUndo->setText( i18n( "Undo %1" ).arg(undo->top()->name()) );
    mActionUndo->setEnabled( !undo->isEmpty() );

    if (!redo->top())
        mActionRedo->setText( i18n( "Redo" ) );
    else
        mActionRedo->setText( i18n( "Redo %1" ).arg(redo->top()->name()) );
    mActionRedo->setEnabled( !redo->isEmpty() );
}

void ActionManager::addresseeSelected(bool selected)
{
    if (mReadWrite)
    {
        mActionCut->setEnabled(selected);
    }

    mActionCopy->setEnabled(selected);
    mActionDelete->setEnabled(selected);
    mActionEditAddressee->setEnabled(selected);
    mActionMail->setEnabled(selected);
}

void ActionManager::modified(bool mod)
{
    mModified = mod;
    mActionSave->setEnabled(mod);
}

void ActionManager::initActionViewList()
{
    // Create the view actions, and set the active view
    // Find the last active view
    QStringList viewNameList = mViewManager->viewNames();
    KToggleAction *viewAction = 0;

    // Just incast there is no active view!
    if (mActiveViewName.isEmpty() ||
        (viewNameList.contains(mActiveViewName) == 0))
        mActiveViewName = *(viewNameList).at(0);

    // unplug the current ones
    mGUIClient->factory()->unplugActionList(mGUIClient, "view_loadedviews");

    // delete the current ones
    mActionViewList.clear();
    mActiveActionView = 0L;

    // Now find the active one, check the menu item, and raise it to the top
    QStringList::Iterator iter;
    QString viewName;
    for (iter = viewNameList.begin(); iter != viewNameList.end(); ++iter)
    {
        viewName = *iter;

        viewAction = new KToggleAction(viewName, QString::null, this,
                                       SLOT(selectViewAction()), mACollection,
                                       viewName.latin1());

        if (mActiveViewName == viewName)
        {
            mViewManager->setActiveView(viewName);

            viewAction->setChecked(true);
            mActiveActionView = viewAction;
        }

        mActionViewList.append(viewAction);
    }

    // Now append all the actions to the menu.
    mGUIClient->factory()->plugActionList(mGUIClient, "view_loadedviews",
                                          mActionViewList);
}

void ActionManager::viewConfigChanged(const QString &newActive)
{
    if (!newActive.isEmpty())
    {
        mActiveViewName = newActive;
    }

    // we need to rebuild the actions
    initActionViewList();

    // Only enable delete if there is more than one view
    mActionDeleteView->setEnabled(mViewManager->viewNames().size() > 1);
}

void ActionManager::selectViewAction()
{
    // See if we can find the selected action
    KToggleAction *action = 0;

    QString activatedName = sender()->name();
    QPtrListIterator<KAction> iter(mActionViewList);
    for (iter.toFirst(); iter.current(); ++iter)
    {
        action = dynamic_cast<KToggleAction*>(*iter);

        if (action->name() != activatedName)
            action->setChecked(false);
        else
        {
            mActiveActionView = action;
            mActiveActionView->setChecked(true);
            mActiveViewName = mActiveActionView->name();

            // Last, tell the main widget to set the view.
            mViewManager->setActiveView(mActiveViewName);
        }
    }
}

void ActionManager::quickToolsAction()
{
    mViewManager->setJumpButtonBarVisible(mActionJumpBar->isChecked());
    mViewManager->setFeaturesVisible(mActionFeatures->isChecked());
    mViewManager->setDetailsVisible(mActionDetails->isChecked());
}

void ActionManager::setFilterNames(const QStringList& list)
{
    QString current = mActionSelectFilter->currentText();

    QStringList items;
    items.append(i18n("None"));
    items+=list;
    mActionSelectFilter->setItems(items);

    setCurrentFilterName( current );
}

void ActionManager::slotFilterActivated(int index)
{
    emit filterActivated(index-1);
}

void ActionManager::setCurrentFilterName(const QString& name)
{
    QStringList items=mActionSelectFilter->items();
    int index=items.findIndex(name);
    if( index != -1 )
        setCurrentFilter(index);
}

void ActionManager::setCurrentFilter(int index)
{
    mActionSelectFilter->setCurrentItem(index);
    emit filterActivated(index-1);
}

bool ActionManager::isModified()
{
  return mModified;
}

#include "actionmanager.moc"
