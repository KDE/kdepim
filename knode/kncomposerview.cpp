/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2007 the KNode authors.
    See file AUTHORS for details
    Copyright (c) 2010 Olivier Trichet <nive@nivalis.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "kncomposerview.h"

#include "kncomposereditor.h"
#include "knglobals.h"
#include "settings.h"

#include <KLocale>
#include <KPIMIdentities/IdentityCombo>
#include <KPIMIdentities/Identity>
#include <KPIMIdentities/IdentityManager>
#include <KPIMUtils/Email>


namespace KNode {
namespace Composer {


View::View( KNComposer *composer )
  : QSplitter( Qt::Vertical, composer ),
    mAttachmentSetup( false )
{
  setupUi( this );

  setChildrenCollapsible( false );
  mAttachmentWidget->hide();

  //From
  mFromEdit->setView( this );
  mFromEdit->enableCompletion( false );
  mEdtList.append( mFromEdit );
  showFrom( false );

  //To
  mToEdit->setView( this );
  mToEdit->enableCompletion( true );
  mEdtList.append( mToEdit );
  connect( mToButton, SIGNAL(clicked(bool)),
           parent(), SLOT(slotToBtnClicked()) );

  //Newsgroups
  mGroupsEdit->setView( this );
  mGroupsEdit->enableCompletion( false );
  mEdtList.append( mGroupsEdit );
  connect( mGroupsEdit, SIGNAL(editingFinished()),
           this, SLOT(slotGroupsChanged()) );
  connect( mGroupsButton, SIGNAL(clicked()),
           parent(), SLOT(slotGroupsBtnClicked()) );

  //Followup-to
  connect( mFollowuptoEdit, SIGNAL(focused()),
           this, SLOT(hideFollowuptoHint()) );

  //subject
  mSubjectEdit->setView( this );
  mSubjectEdit->enableCompletion( false );
  mEdtList.append( mSubjectEdit );
  connect( mSubjectEdit, SIGNAL(textChanged(QString)),
           parent(), SLOT(slotSubjectChanged(QString)) );

  //Editors
  mEditor->switchToPlainText();
  mEditor->setMinimumHeight(50);

  connect( mExternalKillSwitch, SIGNAL(clicked(bool)),
           this, SIGNAL(closeExternalEditor()) );
  hideExternalNotification();
  mExternalKillSwitch->setIcon( KIcon( "application-exit" ) );

  // Identities
  connect( mIdentitySelector, SIGNAL(identityChanged(uint)),
           this, SLOT(slotIdentityChanged(uint)) );
  setIdentity( KNGlobals::self()->identityManager()->defaultIdentity().uoid() );
}


View::~View()
{
  if ( mAttachmentsList->topLevelItemCount() > 0 ) { // The attachment view was visible
    KConfigGroup conf( knGlobals.config(), "POSTNEWS");

    conf.writeEntry("Att_Splitter",sizes());   // save splitter pos

    QList<int> lst;                        // save header sizes
    QHeaderView *h = mAttachmentsList->header();
    for ( int i = 0 ; i < h->count() ; ++i ) {
      lst << h->sectionSize(i);
    }
    conf.writeEntry("Att_Headers",lst);
  }
}


void View::completeSetup( bool firstEdit, KNComposer::MessageMode mode )
{
  if ( firstEdit ) {
    // now we place the cursor at the end of the quoted text / below the attribution line
    if ( KNGlobals::self()->settings()->cursorOnTop() ) {
      // FIXME: hack: counting the number of \n\n to catch end of introduction (see KNArticleFactory::createReply())
      int dbleLfCount = KNGlobals::self()->settings()->intro().count( "%L%L" );
      const QString text = mEditor->textOrHtml();
      int pos = 0;
      while ( dbleLfCount >= 0 ) {
        pos = text.indexOf( QLatin1String( "\n\n" ), pos );
        pos += 2;
        --dbleLfCount;
      }
      mEditor->setCursorPositionFromStart( pos - 1 );
    } else {
      mEditor->setCursorPositionFromStart( mEditor->document()->characterCount() - 1 );
    }

    if ( knGlobals.settings()->appendOwnSignature() ) {
      appendSignature();
    }
  } else {
     mEditor->setCursorPositionFromStart( 0 );
  }
  mEditor->document()->setModified( false );

  setMessageMode( mode );

  // Focus
  mEditor->setFocus();
  if ( mSubjectEdit->text().length() == 0 ) {
    mSubjectEdit->setFocus();
  }
  if ( mGroupsEdit->text().length() == 0 && mode == KNComposer::news ) {
    mGroupsEdit->setFocus();
  }
  if ( mToEdit->text().length() == 0 && mode == KNComposer::mail ) {
    mToEdit->setFocus();
  }
}


void View::focusNextPrevEdit( const QWidget *aCur, bool aNext )
{
  QList<QWidget*>::Iterator it;

  if ( !aCur ) {
    it = --( mEdtList.end() );
  } else {
    for ( QList<QWidget*>::Iterator it2 = mEdtList.begin(); it2 != mEdtList.end(); ++it2 ) {
      if ( (*it2) == aCur ) {
        it = it2;
        break;
      }
    }
    if ( it == mEdtList.end() )
      return;
    if ( aNext )
      ++it;
    else {
      if ( it != mEdtList.begin() )
        --it;
      else
        return;
    }
  }
  if ( it != mEdtList.end() ) {
    if ( (*it)->isVisible() )
      (*it)->setFocus();
  } else if ( aNext )
    mEditor->setFocus();
}

void View::setComposingFont( const QFont &font )
{
  mSubjectEdit->setFont( font );
  mToEdit->setFont( font );
  mGroupsEdit->setFont( font );
  mFollowuptoEdit->setFont( font );
  mEditor->setFontForWholeText( font );
}


void View::setMessageMode( KNComposer::MessageMode mode )
{
  showTo( mode != KNComposer::news );

  showGroups( mode != KNComposer::mail );
  showFollowupto( mode != KNComposer::mail );
}


uint View::selectedIdentity() const
{
  return mIdentitySelector->currentIdentity();
}

void View::setIdentity( uint uoid )
{
  mIdentitySelector->setCurrentIdentity( uoid );
  // mIdentitySelector will emit its identityChanged(uint) signal
  // that is connected to slotIdentityChanged(uint)
}

void View::slotIdentityChanged( uint uoid )
{
  KPIMIdentities::IdentityManager *im = KNGlobals::self()->identityManager();
  KPIMIdentities::Identity identity = im->identityForUoid( uoid );
  setFrom( identity.fullEmailAddr() );
  if ( KPIMUtils::isValidAddress( from() ) != KPIMUtils::AddressOk ) {
    showFrom( true );
  }
}


const QString View::from()
{
  return mFromEdit->text();
}

void View::setFrom( const QString& from )
{
  mFromEdit->setText( from );
}


const QStringList View::groups() const
{
  const QRegExp r = QRegExp( "\\s*,\\s*", Qt::CaseInsensitive, QRegExp::RegExp2 );
  return mGroupsEdit->text().split( r, QString::SkipEmptyParts );
}

void View::setGroups( const QString &groups )
{
  mGroupsEdit->setText( groups );
  slotGroupsChanged(); // update the followup-to
}

void View::slotGroupsChanged()
{
  QStringList groupsList = groups();
  int groupsCount = groupsList.size();
  groupsList.append( QString() );

  const QString currFup2 = mFollowuptoEdit->currentText();
  int i = groupsList.indexOf( currFup2 );
  if ( i == -1 ) {
    groupsList.prepend( currFup2 );
  } else {
    groupsList.move( i, 0 );
  }

  mFollowuptoEdit->clear();
  mFollowuptoEdit->addItems( groupsList );

  // Display an hint about fu2
  if ( groupsCount > 1 ) {
    displayFollowuptoHint();
  } else {
    hideFollowuptoHint();
  }
}


const QString View::emailRecipient() const
{
  return mToEdit->text();
}

void View::setEmailRecipient( const QString& to )
{
  mToEdit->setText( to );
}


const QStringList View::followupTo() const
{
  const QRegExp r = QRegExp( "\\s*,\\s*", Qt::CaseInsensitive, QRegExp::RegExp2 );
  return mFollowuptoEdit->currentText().split( r, QString::SkipEmptyParts );
}

void View::setFollowupTo( const QString &followupTo )
{
  hideFollowuptoHint();
  mFollowuptoEdit->setEditText( followupTo );
}

void View::displayFollowuptoHint()
{
  const QString hint = i18nc( "@info/plain This message is place, as an inactive text, in the Followup-To "
                              "line edit of the message composer when the user select more than one "
                              "group to post his/her message.",
                              "Choose an appropriate group to redirect replies..." );
  if ( mFollowuptoEdit->currentText().isEmpty() ) {
    QLineEdit *l = mFollowuptoEdit->lineEdit();
    QPalette palette = l->palette();
    KColorScheme::adjustForeground( palette, KColorScheme::InactiveText );
    l->setPalette( palette );
    l->setText( hint );
  }
}

void View::hideFollowuptoHint()
{
  const QString hint = i18nc( "@info/plain This message is place, as an inactive text, in the Followup-To "
                              "line edit of the message composer when the user select more than one "
                              "group to post his/her message.",
                              "Choose an appropriate group to redirect replies..." );
  if ( mFollowuptoEdit->currentText() == hint ) {
    QLineEdit *l = mFollowuptoEdit->lineEdit();
    QPalette palette = l->palette();
    KColorScheme::adjustForeground( palette, KColorScheme::NormalText );
    l->setPalette( palette );
    l->setText( QString() );
  }
}


const QString View::subject() const
{
  return mSubjectEdit->text();
}

void View::setSubject( const QString &subject )
{
  mSubjectEdit->setText( subject );
}


KNComposerEditor * View::editor() const
{
  return mEditor;
}



void View::appendSignature()
{
  KPIMIdentities::IdentityManager *im = KNGlobals::self()->identityManager();
  KPIMIdentities::Identity identity = im->identityForUoid( selectedIdentity() );
  identity.signature().insertIntoTextEdit( KPIMIdentities::Signature::End,
                                           KPIMIdentities::Signature::AddSeparator,
                                           mEditor );
}



void View::showIdentity( bool show )
{
  mIdentitySelectorLabel->setVisible( show );
  mIdentitySelector->setVisible( show );
}

void View::showFrom( bool show )
{
  mFromLabel->setVisible( show );
  mFromEdit->setVisible( show );
}

void View::showTo( bool show )
{
  mToLabel->setVisible( show );
  mToEdit->setVisible( show );
  mToButton->setVisible( show );
}

void View::showGroups( bool show )
{
  mGroupsLabel->setVisible( show );
  mGroupsEdit->setVisible( show );
  mGroupsButton->setVisible( show );
}

void View::showFollowupto( bool show )
{
  mFollowuptoLabel->setVisible( show );
  mFollowuptoEdit->setVisible( show );
}

void View::showSubject( bool show )
{
  mSubjetLabel->setVisible( show );
  mSubjectEdit->setVisible( show );
}



void View::showAttachmentView()
{
  if ( !mAttachmentSetup ) {
    mAttachmentSetup = true;

    //connections
    connect( mAttachmentsList, SIGNAL(itemSelectionChanged()),
             this, SLOT(slotAttachmentSelectionChanged()) );

    connect( mAttachmentsList, SIGNAL(contextMenuRequested(QPoint)),
             parent(), SLOT(slotAttachmentPopup(QPoint)) );

    connect( mAttachmentsList, SIGNAL(deletePressed()),
             this, SLOT(removeCurrentAttachment()) );
    connect( mAttachmentsList, SIGNAL(attachmentRemoved(KNAttachment::Ptr,bool)),
             parent(), SLOT(slotAttachmentRemoved(KNAttachment::Ptr,bool)) );

    connect( mAttachmentsList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
             mAttachmentsList, SLOT(editCurrentAttachment()) );
    connect( mAttachmentsList, SIGNAL(returnPressed()),
             mAttachmentsList, SLOT(editCurrentAttachment()) );
    connect( mAttachmentsList, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
             parent(), SLOT(slotAttachmentChanged()) );

    //buttons
    connect( mAttachmentAddButton, SIGNAL(clicked()),
             parent(), SLOT(slotAttachFile()) );

    mAttachmentRemoveButton->setEnabled( false );
    connect( mAttachmentRemoveButton, SIGNAL(clicked()),
             this, SLOT(removeCurrentAttachment()) );

    mAttachmentPropertiesButton->setEnabled( false );
    connect( mAttachmentPropertiesButton, SIGNAL(clicked()),
             mAttachmentsList, SLOT(editCurrentAttachment()) );
  }

  if ( !mAttachmentWidget->isVisible() ) {
    mAttachmentWidget->show();

    KConfigGroup conf(knGlobals.config(), "POSTNEWS");

    QList<int> lst = conf.readEntry("Att_Splitter",QList<int>());
    if(lst.count()!=2)
      lst << 267 << 112;
    setSizes(lst);

    lst=conf.readEntry("Att_Headers",QList<int>());
    QHeaderView *h = mAttachmentsList->header();
    if ( lst.count() == h->count() ) {
      for( int i = 0 ; i < h->count() ; ++i) {
        h->resizeSection( i, lst[ i ] );
      }
    }
  }
}

void View::hideAttachmentView()
{
  mAttachmentWidget->hide();
}

void View::addAttachment( KNAttachment::Ptr attachment )
{
  AttachmentViewItem *item = new AttachmentViewItem( mAttachmentsList, attachment );
  mAttachmentsList->addTopLevelItem( item );
}

const QList<KNAttachment::Ptr> View::attachments() const
{
  return mAttachmentsList->attachments();
}

void View::removeCurrentAttachment()
{
  mAttachmentsList->removeCurrentAttachment();
}

void View::editCurrentAttachment()
{
  mAttachmentsList->editCurrentAttachment();
}

void View::slotAttachmentSelectionChanged()
{
  bool e = !mAttachmentsList->selectedItems().isEmpty();
  mAttachmentRemoveButton->setEnabled( e );
  mAttachmentPropertiesButton->setEnabled( e );
}



void View::showExternalNotification()
{
  mEditorsStack->setCurrentWidget( mExternalEditorNotification );
}


void View::hideExternalNotification()
{
  mEditorsStack->setCurrentWidget( mEditor );
}


} // namespace Composer
} // namespace KNode

