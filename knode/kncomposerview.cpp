/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2007 the KNode authors.
    See file AUTHORS for details

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

#include <KPIMIdentities/IdentityCombo>
#include <KPIMIdentities/Identity>
#include <KPIMIdentities/IdentityManager>
#include <QGridLayout>
#include <klocale.h>
#include <QPushButton>
#include <QGroupBox>
#include <Q3Header>
#include <KComboBox>


namespace KNode {
namespace Composer {


View::View( KNComposer *composer )
  : QSplitter( Qt::Vertical, composer ),
    a_ttWidget( 0 ), a_ttView( 0 ), v_iewOpen( false )
{
  setupUi( this );


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
  connect( mGroupsEdit, SIGNAL(textChanged(QString)),
           this, SLOT(slotGroupsChanged(QString)) );
  connect( mGroupsButton, SIGNAL(clicked()),
           parent(), SLOT(slotGroupsBtnClicked()) );

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
  if(v_iewOpen) {
    KConfigGroup conf( knGlobals.config(), "POSTNEWS");

    conf.writeEntry("Att_Splitter",sizes());   // save splitter pos

    QList<int> lst;                        // save header sizes
    Q3Header *h=a_ttView->header();
    for (int i=0; i<5; i++)
      lst << h->sectionSize(i);
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
  if (mode != KNComposer::news) {
    mToLabel->show();
    mToEdit->show();
    mToButton->show();
  } else {
    mToLabel->hide();
    mToEdit->hide();
    mToButton->hide();
  }
  if (mode != KNComposer::mail) {
    mGroupsLabel->show();
    mFollowuptoLabel->show();
    mGroupsEdit->show();
    mFollowuptoEdit->show();
    mGroupsButton->show();

  } else {
    mGroupsLabel->hide();
    mFollowuptoLabel->hide();
    mGroupsEdit->hide();
    mFollowuptoEdit->hide();
    mGroupsButton->hide();
  }
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
  Q_UNUSED( uoid );
  // TODO: populate this method when necessary.
}


const QStringList View::groups() const
{
  const QRegExp r = QRegExp( "\\s*,\\s*", Qt::CaseInsensitive, QRegExp::RegExp2 );
  return mGroupsEdit->text().split( r, QString::SkipEmptyParts );
}

void View::setGroups( const QString &groups )
{
  mGroupsEdit->setText( groups );
}

void View::slotGroupsChanged( const QString &/*groupText*/ )
{
  QStringList groupsList = groups();
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
  mFollowuptoEdit->setEditText( followupTo );
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


void View::showAttachmentView()
{
  if(!a_ttWidget) {
    a_ttWidget=new QWidget(this);
    QGridLayout *topL=new QGridLayout(a_ttWidget);
    topL->setSpacing(4);
    topL->setMargin(4);

    a_ttView = new KNComposer::AttachmentView( a_ttWidget );
    topL->addWidget(a_ttView, 0, 0, 3, 1);

    //connections
    connect(a_ttView, SIGNAL(currentChanged(Q3ListViewItem*)),
            parent(), SLOT(slotAttachmentSelected(Q3ListViewItem*)));
    connect(a_ttView, SIGNAL(clicked ( Q3ListViewItem * )),
            parent(), SLOT(slotAttachmentSelected(Q3ListViewItem*)));

    connect(a_ttView, SIGNAL(contextMenu(K3ListView*, Q3ListViewItem*, const QPoint&)),
            parent(), SLOT(slotAttachmentPopup(K3ListView*, Q3ListViewItem*, const QPoint&)));
    connect(a_ttView, SIGNAL(delPressed(Q3ListViewItem*)),
            parent(), SLOT(slotAttachmentRemove(Q3ListViewItem*)));
    connect(a_ttView, SIGNAL(doubleClicked(Q3ListViewItem*)),
            parent(), SLOT(slotAttachmentEdit(Q3ListViewItem*)));
    connect(a_ttView, SIGNAL(returnPressed(Q3ListViewItem*)),
            parent(), SLOT(slotAttachmentEdit(Q3ListViewItem*)));

    //buttons
    a_ttAddBtn=new QPushButton(i18n("A&dd..."),a_ttWidget);
    connect(a_ttAddBtn, SIGNAL(clicked()), parent(), SLOT(slotAttachFile()));
    topL->addWidget(a_ttAddBtn, 0,1);

    a_ttRemoveBtn=new QPushButton(i18n("&Remove"), a_ttWidget);
    a_ttRemoveBtn->setEnabled(false);
    connect(a_ttRemoveBtn, SIGNAL(clicked()), parent(), SLOT(slotRemoveAttachment()));
    topL->addWidget(a_ttRemoveBtn, 1,1);

    a_ttEditBtn=new QPushButton(i18n("&Properties"), a_ttWidget);
    a_ttEditBtn->setEnabled(false);
    connect(a_ttEditBtn, SIGNAL(clicked()), parent(), SLOT(slotAttachmentProperties()));
    topL->addWidget(a_ttEditBtn, 2,1, Qt::AlignTop);

    topL->setRowStretch(2,1);
    topL->setColumnStretch(0,1);
  }

  if(!v_iewOpen) {
    v_iewOpen=true;
    a_ttWidget->show();

    KConfigGroup conf(knGlobals.config(), "POSTNEWS");

    QList<int> lst = conf.readEntry("Att_Splitter",QList<int>());
    if(lst.count()!=2)
      lst << 267 << 112;
    setSizes(lst);

    lst=conf.readEntry("Att_Headers",QList<int>());
    if(lst.count()==5) {
      QList<int>::Iterator it = lst.begin();

      Q3Header *h=a_ttView->header();
      for(int i=0; i<5; i++) {
        h->resizeSection(i,(*it));
        ++it;
      }
    }
  }
}


void View::hideAttachmentView()
{
  if(v_iewOpen) {
    a_ttWidget->hide();
    v_iewOpen=false;
  }
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
