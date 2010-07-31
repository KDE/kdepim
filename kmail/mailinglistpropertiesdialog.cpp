/*******************************************************************************
**
** Filename   : mailinglistpropertiesdialog.cpp
** Created on : 30 January, 2005
** Copyright  : (c) 2005 Till Adam
** Email      : adam@kde.org
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   In addition, as a special exception, the copyright holders give
**   permission to link the code of this program with any edition of
**   the Qt library by Trolltech AS, Norway (or with modified versions
**   of Qt that use the same license as Qt), and distribute linked
**   combinations including the two.  You must obey the GNU General
**   Public License in all respects for all of the code used other than
**   Qt.  If you modify this file, you may extend this exception to
**   your version of the file, but you are not obligated to do so.  If
**   you do not wish to do so, delete this exception statement from
**   your version.
*******************************************************************************/
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqgroupbox.h>
#include <tqcheckbox.h>
#include <tqpushbutton.h>

#include <klocale.h>
#include <keditlistbox.h>
#include <kdialogbase.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kmcommands.h>

#include "kmfolder.h"
#include "mailinglist-magic.h"
#include "mailinglistpropertiesdialog.h"

using namespace KMail;

MailingListFolderPropertiesDialog::MailingListFolderPropertiesDialog( TQWidget* parent, KMFolder *folder )
    : KDialogBase( parent, "mailinglist_properties", false, i18n( "Mailinglist Folder Properties" ), 
                   KDialogBase::Ok|KDialogBase::Cancel, 
                   KDialogBase::Ok, true ),
      mFolder( folder )
{
  setWFlags( getWFlags() | WDestructiveClose );
  TQLabel* label;
  mLastItem = 0;

  TQVBoxLayout *topLayout = new TQVBoxLayout( layout(), spacingHint(),
                                            "topLayout" );

  TQGroupBox *mlGroup = new TQGroupBox( i18n("Associated Mailing List" ), this );
  mlGroup->setColumnLayout( 0,  Qt::Vertical );
  TQGridLayout *groupLayout = new TQGridLayout( mlGroup->layout(), 6, 3, spacingHint() );
  topLayout->addWidget( mlGroup );
  setMainWidget( mlGroup );

  mHoldsMailingList = new TQCheckBox( i18n("&Folder holds a mailing list"), mlGroup );
  TQObject::connect( mHoldsMailingList, TQT_SIGNAL(toggled(bool)),
                    TQT_SLOT(slotHoldsML(bool)) );
  groupLayout->addMultiCellWidget( mHoldsMailingList, 0, 0, 0, 2 );

  groupLayout->addItem( new TQSpacerItem( 0, 10 ), 1, 0 );

  mDetectButton = new TQPushButton( i18n("Detect Automatically"), mlGroup );
  mDetectButton->setEnabled( false );
  TQObject::connect( mDetectButton, TQT_SIGNAL(pressed()), TQT_SLOT(slotDetectMailingList()) );
  groupLayout->addWidget( mDetectButton, 2, 1 );

  groupLayout->addItem( new TQSpacerItem( 0, 10 ), 3, 0 );

  label = new TQLabel( i18n("Mailing list description:"), mlGroup );
  label->setEnabled( false );
  TQObject::connect( mHoldsMailingList, TQT_SIGNAL(toggled(bool)),
		    label, TQT_SLOT(setEnabled(bool)) );
  groupLayout->addWidget( label, 4, 0 );
  mMLId = new TQLabel( label, "", mlGroup );
  groupLayout->addMultiCellWidget( mMLId, 4, 4, 1, 2 );
  mMLId->setEnabled( false );

  //FIXME: add QWhatsThis
  label = new TQLabel( i18n("Preferred handler:"), mlGroup );
  label->setEnabled(false);
  TQObject::connect( mHoldsMailingList, TQT_SIGNAL(toggled(bool)),
		    label, TQT_SLOT(setEnabled(bool)) );
  groupLayout->addWidget( label, 5, 0 );
  mMLHandlerCombo = new TQComboBox( mlGroup );
  mMLHandlerCombo->insertItem( i18n("KMail"), MailingList::KMail );
  mMLHandlerCombo->insertItem( i18n("Browser"), MailingList::Browser );
  mMLHandlerCombo->setEnabled( false );
  groupLayout->addMultiCellWidget( mMLHandlerCombo, 5, 5, 1, 2 );
  TQObject::connect( mMLHandlerCombo, TQT_SIGNAL(activated(int)),
                    TQT_SLOT(slotMLHandling(int)) );
  label->setBuddy( mMLHandlerCombo );

  label = new TQLabel( i18n("&Address type:"), mlGroup );
  label->setEnabled(false);
  TQObject::connect( mHoldsMailingList, TQT_SIGNAL(toggled(bool)),
		    label, TQT_SLOT(setEnabled(bool)) );
  groupLayout->addWidget( label, 6, 0 );
  mAddressCombo = new TQComboBox( mlGroup );
  label->setBuddy( mAddressCombo );
  groupLayout->addWidget( mAddressCombo, 6, 1 );
  mAddressCombo->setEnabled( false );

  //FIXME: if the mailing list actions have either KAction's or toolbar buttons
  //       associated with them - remove this button since it's really silly
  //       here
  TQPushButton *handleButton = new TQPushButton( i18n( "Invoke Handler" ), mlGroup );
  handleButton->setEnabled( false );
  if( mFolder)
  {
  	TQObject::connect( mHoldsMailingList, TQT_SIGNAL(toggled(bool)),
			    handleButton, TQT_SLOT(setEnabled(bool)) );
  	TQObject::connect( handleButton, TQT_SIGNAL(clicked()),
                    TQT_SLOT(slotInvokeHandler()) );
  }
  groupLayout->addWidget( handleButton, 6, 2 );

  mEditList = new KEditListBox( mlGroup );
  mEditList->setEnabled( false );
  groupLayout->addMultiCellWidget( mEditList, 7, 7, 0, 3 );

  TQStringList el;

  //Order is important because the activate handler and fillMLFromWidgets
  //depend on it
  el << i18n( "Post to List" )
     << i18n( "Subscribe to List" )
     << i18n( "Unsubscribe from List" )
     << i18n( "List Archives" )
     << i18n( "List Help" );
  mAddressCombo->insertStringList( el );
  TQObject::connect( mAddressCombo, TQT_SIGNAL(activated(int)),
                    TQT_SLOT(slotAddressChanged(int)) );

  load();
  resize( TQSize(295, 204).expandedTo(minimumSizeHint()) );
  clearWState( WState_Polished );
}

void MailingListFolderPropertiesDialog::slotOk()
{
  save();
  KDialogBase::slotOk();
}

void MailingListFolderPropertiesDialog::load()
{
  if (mFolder) mMailingList = mFolder->mailingList();
  mMLId->setText( (mMailingList.id().isEmpty() ? i18n("Not available") : mMailingList.id()) );
  mMLHandlerCombo->setCurrentItem( mMailingList.handler() );
  mEditList->insertStringList( mMailingList.postURLS().toStringList() );

  mAddressCombo->setCurrentItem( mLastItem );
  mHoldsMailingList->setChecked( mFolder && mFolder->isMailingListEnabled() );
}

//-----------------------------------------------------------------------------
bool MailingListFolderPropertiesDialog::save()
{
  if( mFolder )
  {
    // settings for mailingList
    mFolder->setMailingListEnabled( mHoldsMailingList && mHoldsMailingList->isChecked() );
    fillMLFromWidgets();
    mFolder->setMailingList( mMailingList );
  }
  return true;
}

//----------------------------------------------------------------------------
void MailingListFolderPropertiesDialog::slotHoldsML( bool holdsML )
{
  mMLHandlerCombo->setEnabled( holdsML );
  if ( mFolder && mFolder->count() )
    mDetectButton->setEnabled( holdsML );
  mAddressCombo->setEnabled( holdsML );
  mEditList->setEnabled( holdsML );
  mMLId->setEnabled( holdsML );
}

//----------------------------------------------------------------------------
void MailingListFolderPropertiesDialog::slotDetectMailingList()
{
  if ( !mFolder ) return; // in case the folder was just created
  int num = mFolder->count();

  kdDebug(5006)<<k_funcinfo<<" Detecting mailing list"<<endl;

  /* FIXME Till - make work without the folder tree
  // first try the currently selected message
  KMFolderTree *folderTree = static_cast<KMFolderTree *>( mDlg->parent() );
  int curMsgIdx = folderTree->mainWidget()->headers()->currentItemIndex();
  if ( curMsgIdx > 0 ) {
    KMMessage *mes = mFolder->getMsg( curMsgIdx );
    if ( mes )
      mMailingList = MailingList::detect( mes );
  }
  */

  // next try the 5 most recently added messages
  if ( !( mMailingList.features() & MailingList::Post ) ) {
    const int maxchecks = 5;
    for( int i = --num; i > num-maxchecks; --i ) {
      KMMessage *mes = mFolder->getMsg( i );
      if ( !mes )
        continue;
      mMailingList = MailingList::detect( mes );
      if ( mMailingList.features() & MailingList::Post )
        break;
    }
  }
  if ( !(mMailingList.features() & MailingList::Post) ) {
    KMessageBox::error( this,
              i18n("KMail was unable to detect a mailing list in this folder. "
                   "Please fill the addresses by hand.") );
  } else {
    mMLId->setText( (mMailingList.id().isEmpty() ? i18n("Not available.") : mMailingList.id() ) );
    fillEditBox();
  }
}

//----------------------------------------------------------------------------
void MailingListFolderPropertiesDialog::slotMLHandling( int element )
{
  mMailingList.setHandler( static_cast<MailingList::Handler>( element ) );
}

//----------------------------------------------------------------------------
void MailingListFolderPropertiesDialog::slotAddressChanged( int i )
{
  fillMLFromWidgets();
  fillEditBox();
  mLastItem = i;
}

//----------------------------------------------------------------------------
void MailingListFolderPropertiesDialog::fillMLFromWidgets()
{
  if ( !mHoldsMailingList->isChecked() )
    return;

  // make sure that email addresses are prepended by "mailto:"
  bool changed = false;
  TQStringList oldList = mEditList->items();
  TQStringList newList; // the correct string list
  for ( TQStringList::ConstIterator it = oldList.begin();
        it != oldList.end(); ++it ) {
    if ( !(*it).startsWith("http:") && !(*it).startsWith("https:") &&
         !(*it).startsWith("mailto:") && ( (*it).find('@') != -1 ) ) {
      changed = true;
      newList << "mailto:" + *it;
    }
    else {
      newList << *it;
    }
  }
  if ( changed ) {
    mEditList->clear();
    mEditList->insertStringList( newList );
  }

  //mMailingList.setHandler( static_cast<MailingList::Handler>( mMLHandlerCombo->currentItem() ) );
  switch ( mLastItem ) {
  case 0:
    mMailingList.setPostURLS( mEditList->items() );
    break;
  case 1:
    mMailingList.setSubscribeURLS( mEditList->items() );
    break;
  case 2:
    mMailingList.setUnsubscribeURLS( mEditList->items() );
    break;
  case 3:
    mMailingList.setArchiveURLS( mEditList->items() );
    break;
  case 4:
    mMailingList.setHelpURLS( mEditList->items() );
    break;
  default:
    kdWarning( 5006 )<<"Wrong entry in the mailing list entry combo!"<<endl;
  }
}

void MailingListFolderPropertiesDialog::fillEditBox()
{
  mEditList->clear();
  switch ( mAddressCombo->currentItem() ) {
  case 0:
    mEditList->insertStringList( mMailingList.postURLS().toStringList() );
    break;
  case 1:
    mEditList->insertStringList( mMailingList.subscribeURLS().toStringList() );
    break;
  case 2:
    mEditList->insertStringList( mMailingList.unsubscribeURLS().toStringList() );
    break;
  case 3:
    mEditList->insertStringList( mMailingList.archiveURLS().toStringList() );
    break;
  case 4:
    mEditList->insertStringList( mMailingList.helpURLS().toStringList() );
    break;
  default:
    kdWarning( 5006 )<<"Wrong entry in the mailing list entry combo!"<<endl;
  }
}

void MailingListFolderPropertiesDialog::slotInvokeHandler()
{
  KMCommand *command =0;
  switch ( mAddressCombo->currentItem() ) {
  case 0:
    command = new KMMailingListPostCommand( this, mFolder );
    break;
  case 1:
    command = new KMMailingListSubscribeCommand( this, mFolder );
    break;
  case 2:
    command = new KMMailingListUnsubscribeCommand( this, mFolder );
    break;
  case 3:
    command = new KMMailingListArchivesCommand( this, mFolder );
    break;
  case 4:
    command = new KMMailingListHelpCommand( this, mFolder );
    break;
  default:
    kdWarning( 5006 )<<"Wrong entry in the mailing list entry combo!"<<endl;
  }
  if ( command ) command->start();
}

#include "mailinglistpropertiesdialog.moc"
