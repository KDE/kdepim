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

#include <qevent.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwidgetstack.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qhbox.h>
#include <qdragobject.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kabc/field.h>
#include <kabc/addressbook.h>

#include "undo.h"
#include "undocmds.h"
#include "viewmanager.h"
#include "configureviewdialog.h"
#include "viewwrapper.h"
#include "iconviewwrapper.h"
#include "tableviewwrapper.h"
#include "detailsviewcontainer.h"
#include "cardviewwrapper.h"
#include "addviewdialog.h"
#include "jumpbuttonbar.h"
#include "addresseeutil.h"
#include "addresseeeditorwidget.h"
#include "filterselectionwidget.h"

////////////////////////////////////////
// View Manager

ViewManager::ViewManager(KABC::AddressBook *doc, KConfig *config,
                         QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  mConfig = config;
  mDocument = doc;

  // Create the GUI
  initGUI();

  // Set the list to auto delete the views and the wrappers
  mViewDict.setAutoDelete(true);
  mViewWrapperDict.setAutoDelete(true);

  // Create the view wrappers
  createViewWrappers();

  mActiveView = 0;
}

ViewManager::~ViewManager()
{
  unloadViews();
  mViewWrapperDict.clear();
}

void ViewManager::readConfig()
{
  // Read the view names
  mConfig->setGroup("Views");
  mViewNameList = mConfig->readListEntry("Names");

  if (mViewNameList.size() == 0)
  {
    // Add a default
    mViewNameList << i18n("Default Table View");
  }

  mFilterList = Filter::restore(mConfig, "Filter");
  filtersChanged(mFilterList);
  mConfig->setGroup("Filter");
  if (mConfig->hasKey("Active"))
  {
      emit(setCurrentFilterName(mConfig->readEntry("Active")));
  }
  // Tell the views to reread their config, since they may have
  // been modified by global settings
  QDictIterator<KAddressBookView> iter(mViewDict);
  for (iter.toFirst(); iter.current(); ++iter)
  {
    mConfig->setGroup(iter.currentKey());
    iter.current()->readConfig(mConfig);
  }
}

void ViewManager::writeConfig()
{
    // Iterator through all the views
    QDictIterator<KAddressBookView> iter(mViewDict);
    for (iter.toFirst(); iter.current(); ++iter)
    {
        mConfig->setGroup(iter.currentKey());
        (*iter)->writeConfig(mConfig);
    }
    Filter::save(mConfig, QString("Filter"), mFilterList);
    mConfig->setGroup("Filter");
    mConfig->writeEntry("Active", mCurrentFilter.name());
    // write the view name list
    mConfig->setGroup("Views");
    mConfig->writeEntry("Names", mViewNameList);
}

QStringList ViewManager::selectedUids()
{
  return mActiveView->selectedUids();
}

void ViewManager::sendMail()
{
  QString emailAddrs = mActiveView->selectedEmails();
  kapp->invokeMailer( emailAddrs, "" );
}

void ViewManager::sendMail(const QString& addressee)
{
  kapp->invokeMailer(addressee, "");
}

void ViewManager::browse(const QString& url)
{
  kapp->invokeBrowser(url);
}

void ViewManager::deleteAddressee()
{
  KABC::Addressee a;

  // Get the selected uids
  QStringList uidList = mActiveView->selectedUids();

  if (uidList.size() > 0)
  {
    PwDeleteCommand *command = new PwDeleteCommand( mDocument, uidList );
    UndoStack::instance()->push( command );
    RedoStack::instance()->clear();

    // now if we deleted anything, refresh
    mActiveView->refresh();
    emit selected( QString::null );
    addresseeSelected( QString::null );

    emit modified();
  }
}

void ViewManager::paste()
{
  QClipboard *cb = QApplication::clipboard();
  PwPasteCommand *command = new PwPasteCommand( mDocument, cb->text() );
  UndoStack::instance()->push( command );
  RedoStack::instance()->clear();

  modified();
  mActiveView->refresh();
}

void ViewManager::copy()
{
  QStringList uidList = mActiveView->selectedUids();
  KABC::Addressee::List aList;
  KABC::Addressee a;
  QString clipText;

  QStringList::Iterator iter;
  for ( iter = uidList.begin(); iter != uidList.end(); ++iter ) {
    aList.append(mDocument->findByUid(*iter));
  }

  clipText = AddresseeUtil::addresseesToClipboard(aList);
  kdDebug() << "ViewManager::copy: " << clipText << endl;
  QClipboard *cb = QApplication::clipboard();
  cb->setText( clipText );
}

void ViewManager::cut()
{
  QStringList uidList = mActiveView->selectedUids();

  if (uidList.size() > 0)
  {
    PwCutCommand *command = new PwCutCommand(mDocument, uidList);
    UndoStack::instance()->push( command );
    RedoStack::instance()->clear();

    mActiveView->refresh();
    emit modified();
  }
}

void ViewManager::setSelected(QString uid, bool selected)
{
  mActiveView->setSelected(uid, selected);
}

void ViewManager::unloadViews()
{
  mViewDict.clear();
  mActiveView = 0;
}

void ViewManager::setActiveView(const QString &name)
{
    // Find the view
    KAddressBookView *view = 0;

    // Check that this isn't the same as the current active view
    if (mActiveView && (mActiveView->name() == name))
        return;

    // At this point we know the view that should be active is not
    // currently active. We will try to find the new on in the list. If
    // we can't find it, it means it hasn't been instantiated, so we will
    // create it on demand.

    view = mViewDict.find(name);

    // Check if we found the view. If we didn't, then we need to create it
    if (view == 0)
    {
      KConfig *config = kapp->config();
      config->setGroup(name);
      QString type = config->readEntry("Type", "Table");

      kdDebug() << "ViewManager::setActiveView: creating view - "
                << name << endl;

      // Find the wrapper, ask it to create the view
      ViewWrapper *wrapper = mViewWrapperDict.find(type);
      if (wrapper)
          view = wrapper->createView(mDocument, mViewWidgetStack,
                                     name.latin1());

      if (view)
      {
        mViewDict.insert(name, view);
        mViewWidgetStack->addWidget(view);
        view->readConfig(config);

        // The manager just relays the signals
        connect(view, SIGNAL(selected(const QString &)),
                this, SIGNAL(selected(const QString &)));
        connect(view, SIGNAL(selected(const QString &)),
                this, SLOT(addresseeSelected(const QString &)));
        connect(view, SIGNAL(executed(const QString &)),
                this, SIGNAL(executed(const QString &)));
        connect(view, SIGNAL(modified()),
                this, SIGNAL(modified()));
        connect(view, SIGNAL(dropped(QDropEvent*)),
                this, SLOT(dropped(QDropEvent*)));
        connect(view, SIGNAL(startDrag()), this, SLOT(startDrag()));
      }
    }

    // If we found or created the view, raise it and refresh it
    if ( view )
    {
      mActiveView = view;
      mViewWidgetStack->raiseWidget(view);
      // Set the proper filter in the view. By setting the combo
      // box, the activated slot will be called, which will push
      // the filter to the view and refresh it.
      if (view->defaultFilterType() == KAddressBookView::None)
      {
          emit(setCurrentFilter(0));
      }
      else if (view->defaultFilterType() == KAddressBookView::Active)
      {
          emit(setCurrentFilterName(mCurrentFilter.name()));
      }
      else   // KAddressBookView::Specific
      {
        QString filterName = view->defaultFilterName();
        emit(setCurrentFilterName(filterName));
      }

      // Update the inc search combo to show the fields in the new active
      // view.
      refreshIncrementalSearchCombo();

      mActiveView->refresh( QString::null );
    }
    else
    {
      kdDebug() << "ViewManager::setActiveView: unable to find view\n";
    }
}

void ViewManager::refresh(QString uid)
{
    mActiveView->refresh(uid);
    addresseeSelected(uid);
}

void ViewManager::modifyView()
{
  // Find the wrapper for the type they are modifying
  ViewWrapper *wrapper;
  ConfigureViewDialog *dialog = 0;

  // Find the wrapper
  wrapper = mViewWrapperDict.find(mActiveView->type());

  if (wrapper)
  {
    // Save the filters so the dialog has the latest set
    Filter::save(mConfig, "Filter", mFilterList);

    dialog = wrapper->createConfigureViewDialog(mActiveView->name(),
                                                mDocument,
                                                this, "ConfigureViewDialog");
  }

  // If we found the wrapper and it successfully created a dialog, display it
  if (dialog)
  {
    // Set the config group
    mConfig->setGroup(mActiveView->name());
    dialog->readConfig(mConfig);
    // Let the dialog run (it is modal)
    if (dialog->exec())
    {
      // The user accepted
      dialog->writeConfig(mConfig);
      mActiveView->readConfig(mConfig);

      // Set the proper filter in the view. By setting the combo
      // box, the activated slot will be called, which will push
      // the filter to the view and refresh it.
      if (mActiveView->defaultFilterType() == KAddressBookView::None)
      {
          emit(setCurrentFilter(0));
      }
      else if (mActiveView->defaultFilterType() == KAddressBookView::Active)
      {
          emit(setCurrentFilterName(mCurrentFilter.name()));
      }
      else   // KAddressBookView::Specific
      {
        QString filterName = mActiveView->defaultFilterName();
        emit(setCurrentFilterName(filterName));
      }

      mActiveView->refresh();

      // cleanup
      delete dialog;
    }
  }
}

void ViewManager::deleteView()
{
  // Confirm with the user
  QString text = i18n("Are you sure that you want to delete the view \"%1\"?").arg( mActiveView->name() );

  QString caption = i18n("Confirm Delete");

  if (KMessageBox::questionYesNo(this, text, caption) == KMessageBox::Yes)
  {
    mViewNameList.remove(mActiveView->name());

    // remove the view from the config file
    KConfig *config = kapp->config();
    config->deleteGroup( mActiveView->name() );

    mViewDict.remove(mActiveView->name());
    mActiveView = 0;

    // we are in an invalid state now, but that should be fixed after
    // we emit the signal
    emit viewConfigChanged(QString::null);
  }
}

void ViewManager::addView()
{
  // Display the add view dialog
  AddViewDialog dialog(&mViewWrapperDict, this, "AddViewDialog");

  if (dialog.exec())
  {

    QString newName = dialog.viewName();
    QString type = dialog.viewType();

    // Check for name conflicts
    bool firstConflict = true;
    int numTries = 1;
    while (mViewNameList.contains(newName) > 0)
    {
      if (!firstConflict)
      {
        newName = newName.left(newName.length()-4);
        firstConflict = false;
      }

      newName.sprintf("%s <%d>", newName.latin1(), numTries);
      numTries++;
    }

    // Add the new one to the list
    mViewNameList << newName;

    // write the view to the config file,
    // launch the view config dialog
    KConfig *config = kapp->config();
    config->deleteGroup(newName);   // Incase they had this view before
    config->setGroup(newName);
    config->writeEntry("Type", type);

    // try to set the active view
    emit viewConfigChanged(newName);

    // Now let the user modify it
    modifyView();
  }
}

void ViewManager::createViewWrappers()
{
  ViewWrapper *wrapper;

  // View Developers: Add an entry here to create the wrapper for your view
  // type and add it to the list. Thats it :D

  wrapper = new IconViewWrapper();
  mViewWrapperDict.insert(wrapper->type(), wrapper);

  wrapper = new TableViewWrapper();
  mViewWrapperDict.insert(wrapper->type(), wrapper);

  wrapper = new CardViewWrapper();
  mViewWrapperDict.insert(wrapper->type(), wrapper);
}

void ViewManager::initGUI()
{
  QHBoxLayout *l = new QHBoxLayout( this );
  l->setMargin( KDialogBase::marginHint() );
  l->setSpacing( KDialogBase::spacingHint() );

  mQSpltFeatures = new QSplitter( this );
  mQSpltFeatures->setOrientation( Qt::Vertical );

  mQSpltDetails = new QSplitter( mQSpltFeatures );

  mFeatures = new QTabWidget( mQSpltFeatures );

  mViewWidgetStack = new QWidgetStack( mQSpltDetails, "mViewWidgetStack" );

  mDetails = new ViewContainer( mQSpltDetails );
  connect( mDetails, SIGNAL(addresseeChanged()), SLOT(addresseeModified()) );
  connect( mDetails, SIGNAL(sendEmail(const QString&)),
            SLOT(sendMail(const QString&)) );
  connect( mDetails, SIGNAL(browse(const QString&)),
            SLOT(browse(const QString&)) );

  mJumpButtonBar = new JumpButtonBar( this, "mJumpButtonBar" );
  // ----- create the quick edit widget as part of the features tabwidget
  //       (THIS WILL BE REMOVED!):
  mQuickEdit = new AddresseeEditorWidget( mFeatures, "mQuickEdit" );
  mFeatures->addTab( mQuickEdit, i18n("QuickEdit") );
  connect( mQuickEdit, SIGNAL(modified()), SLOT(addresseeModified()) );

  connect( mJumpButtonBar, SIGNAL(jumpToLetter(const QChar &)),
            this, SLOT(jumpToLetter(const QChar &)) );

  l->addWidget( mQSpltFeatures );
  l->setStretchFactor( mQSpltFeatures, 100 );
  l->addWidget( mJumpButtonBar );
  l->setStretchFactor( mJumpButtonBar, 1 );
}

void ViewManager::refreshIncrementalSearchCombo()
{
    QStringList items;

    // Insert all the items
    // Note: There is currently a problem with i18n here. If we translate each
    // item, the user gets the right display, but then the incrementalSearch
    // function is called with the translated text, which is incorrect.
    // Hmm.. how to fix? -mpilone
    KABC::Field::List fields = mActiveView->fields();

    mIncrementalSearchFields.clear();

    KABC::Field::List::ConstIterator it;
    int i = 0;
    for( it = fields.begin(); it != fields.end(); ++it ) {
        items.append((*it)->label());
        mIncrementalSearchFields.append( *it );
        ++i;
    }
    mCurrentIncSearchField=mIncrementalSearchFields.first(); // we assume there are always columns?
    emit(setIncSearchFields(items));
}

void ViewManager::incSearch(const QString& text, int field)
{
    mCurrentIncSearchField=mIncrementalSearchFields[field];
    mActiveView->incrementalSearch(text, mCurrentIncSearchField);
}

void ViewManager::jumpToLetter(const QChar &ch)
{
  // Jumping always works based on the first field
    mActiveView->incrementalSearch(QString(ch), mCurrentIncSearchField);
}

void ViewManager::setJumpButtonBarVisible(bool visible)
{
    if (visible)
      mJumpButtonBar->show();
    else
      mJumpButtonBar->hide();
}

void ViewManager::setFeaturesVisible(bool visible)
{
  if (visible)
    mFeatures->show();
  else
    mFeatures->hide();
}

void ViewManager::setDetailsVisible(bool visible)
{
  if( visible ) {
    mDetails->show();
  } else {
    mDetails->hide();
  }
}

// WORK_TO_DO: obsolete
bool ViewManager::isQuickEditVisible()
{
  return !mQuickEdit->isHidden();
}

void ViewManager::dropped(QDropEvent *e)
{
  kdDebug() << "ViewManager::dropped: got a drop event" << endl;

  QString clipText;
  QStrList  urls;


  if (QUriDrag::decode(e, urls)) {
     QPtrListIterator<char> it (urls);
     int c = urls.count();
     if ( c > 1 ) {
       QString questionString = i18n( "Import one contact into your addressbook?","Import %n contacts into your addressbook?", c );
       if ( KMessageBox::questionYesNo( this, questionString, i18n( "Import Contacts?" ) ) == KMessageBox::Yes ) {
         for ( ; it.current(); ++it) {
           KURL url(*it);
           emit importVCard( url.path(), false );
         }
       }
     } else if (c == 1) {
       KURL url(*it);
       emit importVCard( url.path(), true );
     }
  } else if (QTextDrag::decode(e, clipText)) {
     KABC::Addressee::List aList;
     aList = AddresseeUtil::clipboardToAddressees(clipText);

     KABC::Addressee::List::Iterator iter;
     for (iter = aList.begin(); iter != aList.end(); ++iter)
     {
       mDocument->insertAddressee(*iter);
     }

     mActiveView->refresh();
   }

}

void ViewManager::startDrag()
{
  kdDebug() << "ViewManager::startDrag: starting to drag" << endl;

  // Get the list of all the selected addressees
  KABC::Addressee::List aList;
  QStringList uidList = selectedUids();
  QStringList::Iterator iter;
  for (iter = uidList.begin(); iter != uidList.end(); ++iter)
    aList.append(mDocument->findByUid(*iter));

  QDragObject *drobj;
  drobj = new QTextDrag( AddresseeUtil::addresseesToClipboard(aList), this );
  drobj->setPixmap(KGlobal::iconLoader()->loadIcon("vcard",
                                                     KIcon::Desktop));
  drobj->dragCopy();
}

void ViewManager::addresseeSelected(const QString &uid)
{
  KABC::Addressee a = mDocument->findByUid(uid);
  mQuickEdit->setAddressee(a);
  mDetails->setAddressee(a);
}

void ViewManager::addresseeModified()
{
    KABC::Addressee a;
    // WORK_TO_DO: obsolete after port of Quick Edit to be a Details View Style
    mQuickEdit->save();
    // a = mQuickEdit->addressee();
    // save the changes:
    // WORK_TO_DO: check for emittances during build up
    a = mDetails->addressee();
    mDocument->insertAddressee(a);
    mActiveView->refresh(a.uid());

    emit modified();
}

void ViewManager::filtersChanged(const Filter::List &list)
{
  mFilterList = list;

  QStringList names;
  Filter::List::Iterator iter;
  for (iter = mFilterList.begin(); iter != mFilterList.end(); ++iter)
    names << (*iter).name();

  // Update the combo
  emit(setFilterNames(names));
  mCurrentFilter=Filter();
}

void ViewManager::filterActivated(int index)
{
  if (index < 0) {
    mCurrentFilter = Filter();
  } else {
    mCurrentFilter = mFilterList[ index ];
  }

  // Check if we have a view. Since the filter combo is created before
  // the view, this slot could be called before there is a valid view.
  if ( mActiveView ) {
    mActiveView->setFilter( mCurrentFilter );
    mActiveView->refresh();
  }
}

#include "viewmanager.moc"
