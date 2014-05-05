/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (c) 2009-2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "attendeeline.h"
#include "attendeedata.h"

#include <KCalUtils/kcalutils/Stringify>

#include <KPIMUtils/kpimutils/Email>

#include <KCompletionBox>
#include <KDebug>
#include <KDialog>
#include <KLocalizedString>
#include <KIconLoader>

#include <QBoxLayout>
#include <QKeyEvent>
#include <QMenu>

using namespace IncidenceEditorNG;

typedef QPair<QString, QIcon> TextIconPair;

AttendeeComboBox::AttendeeComboBox( QWidget *parent )
  : QToolButton( parent ), mMenu( new QMenu( this ) ), mCurrentIndex( -1 )
{
  setPopupMode( QToolButton::InstantPopup );
  setToolButtonStyle( Qt::ToolButtonIconOnly );
  setMenu( mMenu );
}

void AttendeeComboBox::addItem( const QIcon &icon, const QString &text )
{
  mList.append( TextIconPair( text, icon ) );
  if ( mCurrentIndex == -1 ) {
    setCurrentIndex( 0 );
  }
  int index = mList.size() - 1;
  QAction *act = menu()->addAction( icon, text, this, SLOT(slotActionTriggered()) );
  act->setData( index );
}

void AttendeeComboBox::addItems( const QStringList &texts )
{
  foreach ( const QString &str, texts ) {
    addItem( QIcon(), str );
  }
  if ( mCurrentIndex == -1 ) {
    setCurrentIndex( 0 );
  }
}

int AttendeeComboBox::currentIndex() const
{
  return mCurrentIndex;
}

void AttendeeComboBox::clear()
{
  mCurrentIndex = -1;
  mMenu->clear();
  mList.clear();
}

void AttendeeComboBox::setCurrentIndex( int index )
{
  Q_ASSERT( index < mList.size() );
  const int old = mCurrentIndex;
  mCurrentIndex = index;
  setIcon( mList.at( index ).second );
  setToolTip( mList.at( index ).first );
  if ( old != index ) {
    emit itemChanged();
  }
}

void AttendeeComboBox::slotActionTriggered()
{
  int index = qobject_cast<QAction*> ( sender() )->data().toInt();
  setCurrentIndex( index );
}

void AttendeeComboBox::keyPressEvent( QKeyEvent *ev )
{
  if ( ev->key() == Qt::Key_Left ) {
    emit leftPressed();
  } else if ( ev->key() == Qt::Key_Right ) {
    emit rightPressed();
  } else {
    QToolButton::keyPressEvent( ev );
  }
}

AttendeeLineEdit::AttendeeLineEdit( QWidget *parent )
  : AddresseeLineEdit( parent, true )
{
}

void AttendeeLineEdit::keyPressEvent( QKeyEvent *ev )
{
  if ( ( ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return ) &&
       !completionBox()->isVisible() ) {
    emit downPressed();
    KPIM::AddresseeLineEdit::keyPressEvent( ev );
  } else if ( ev->key() == Qt::Key_Backspace  &&  text().isEmpty() ) {
    ev->accept();
    emit deleteMe();
  } else if ( ev->key() == Qt::Key_Left && cursorPosition() == 0 &&
              !ev->modifiers().testFlag( Qt::ShiftModifier ) ) {
    // Shift would be pressed during selection
    emit leftPressed();
  } else if ( ev->key() == Qt::Key_Right && cursorPosition() == (int)text().length() &&
              !ev->modifiers().testFlag( Qt::ShiftModifier ) ) {
    // Shift would be pressed during selection
    emit rightPressed();
  } else if ( ev->key() == Qt::Key_Down ) {
    emit downPressed();
  } else if ( ev->key() == Qt::Key_Up ) {
    emit upPressed();
  } else {
    KPIM::AddresseeLineEdit::keyPressEvent( ev );
  }
}

AttendeeLine::AttendeeLine( QWidget *parent )
  : MultiplyingLine( parent ),
    mRoleCombo( new AttendeeComboBox( this ) ),
    mStateCombo( new AttendeeComboBox( this ) ),
    mResponseCombo( new AttendeeComboBox( this ) ),
    mEdit( new AttendeeLineEdit( this ) ),
    mData( new AttendeeData( QString(), QString() ) ),
    mModified( false )
{
  setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );

  QBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );
#ifdef KDEPIM_MOBILE_UI
  mRoleCombo->addItem( DesktopIcon( "meeting-participant", 48 ),
                       KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::ReqParticipant ) );
  mRoleCombo->addItem( DesktopIcon( "meeting-participant-optional", 48 ),
                       KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::OptParticipant ) );
  mRoleCombo->addItem( DesktopIcon( "meeting-observer", 48 ),
                       KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::NonParticipant ) );
  mRoleCombo->addItem( DesktopIcon( "meeting-chair", 48 ),
                       KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::Chair ) );

  mResponseCombo->addItem( DesktopIcon( "meeting-participant-request-response", 48 ),
                           i18nc( "@item:inlistbox", "Request Response" ) );
  mResponseCombo->addItem( DesktopIcon( "meeting-participant-no-response", 48 ),
                           i18nc( "@item:inlistbox", "Request No Response" ) );
#else
  mRoleCombo->addItem( SmallIcon( "meeting-participant" ),
                       KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::ReqParticipant ) );
  mRoleCombo->addItem( SmallIcon( "meeting-participant-optional" ),
                       KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::OptParticipant ) );
  mRoleCombo->addItem( SmallIcon( "meeting-observer" ),
                       KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::NonParticipant ) );
  mRoleCombo->addItem( SmallIcon( "meeting-chair" ),
                       KCalUtils::Stringify::attendeeRole( KCalCore::Attendee::Chair ) );

  mResponseCombo->addItem( SmallIcon( "meeting-participant-request-response" ),
                           i18nc( "@item:inlistbox", "Request Response" ) );
  mResponseCombo->addItem( SmallIcon( "meeting-participant-no-response" ),
                           i18nc( "@item:inlistbox", "Request No Response" ) );
#endif

  mEdit->setToolTip( i18nc( "@info:tooltip",
                            "Enter the name or email address of the attendee." ) );
  mEdit->setClearButtonShown( true );

  mStateCombo->setWhatsThis( i18nc( "@info:whatsthis",
                                    "Edits the current attendance status of the attendee." ) );

  mRoleCombo->setWhatsThis( i18nc( "@info:whatsthis",
                                   "Edits the role of the attendee." ) );

  mEdit->setWhatsThis( i18nc( "@info:whatsthis",
                              "The email address or name of the attendee. An invitation "
                              "can be sent to the user if an email address is provided." ) );

  setActions( EventActions );

  mResponseCombo->setToolTip( i18nc( "@info:tooltip", "Request a response from the attendee" ) );
  mResponseCombo->setWhatsThis( i18nc( "@info:whatsthis",
                                       "Edits whether to send an email to the "
                                       "attendee to request a response concerning "
                                       "attendance." ) );

  // add them to the layout in the correct order
  topLayout->addWidget( mRoleCombo );
  topLayout->addWidget( mEdit );
  topLayout->addWidget( mStateCombo );
  topLayout->addWidget( mResponseCombo );

  connect( mEdit, SIGNAL(returnPressed()), SLOT(slotReturnPressed()) );
  connect( mEdit, SIGNAL(deleteMe()),
           SLOT(slotPropagateDeletion()), Qt::QueuedConnection );
  connect( mEdit, SIGNAL(textChanged(QString)),
           SLOT(slotTextChanged(QString)), Qt::QueuedConnection );
  connect( mEdit, SIGNAL(upPressed()), SLOT(slotFocusUp()) );
  connect( mEdit, SIGNAL(downPressed()), SLOT(slotFocusDown()) );

  connect( mRoleCombo, SIGNAL(rightPressed()), mEdit, SLOT(setFocus()) );
  connect( mEdit, SIGNAL(leftPressed()), mRoleCombo, SLOT(setFocus()) );

  connect( mEdit, SIGNAL(rightPressed()), mStateCombo, SLOT(setFocus()) );
  connect( mStateCombo, SIGNAL(leftPressed()), mEdit, SLOT(setFocus()) );

  connect( mStateCombo, SIGNAL(rightPressed()), mResponseCombo, SLOT(setFocus()) );

  connect( mResponseCombo, SIGNAL(leftPressed()), mStateCombo, SLOT(setFocus()) );
  connect( mResponseCombo, SIGNAL(rightPressed()), SIGNAL(rightPressed()) );

  connect( mEdit, SIGNAL(editingFinished()),
           SLOT(slotHandleChange()), Qt::QueuedConnection );
  connect( mEdit, SIGNAL(textCompleted()),
           SLOT(slotHandleChange()), Qt::QueuedConnection );
  connect( mEdit, SIGNAL(clearButtonClicked()),
           SLOT(slotPropagateDeletion()), Qt::QueuedConnection );

  connect( mRoleCombo, SIGNAL(itemChanged()), this, SLOT(slotComboChanged()) );
  connect( mStateCombo, SIGNAL(itemChanged()), this, SLOT(slotComboChanged()) );
  connect( mResponseCombo, SIGNAL(itemChanged()), this, SLOT(slotComboChanged()) );

}

void AttendeeLine::activate()
{
  mEdit->setFocus();
}

void AttendeeLine::clear()
{
  mEdit->clear();
  mRoleCombo->setCurrentIndex( 0 );
  mStateCombo->setCurrentIndex( 0 );
  mResponseCombo->setCurrentIndex( 0 );
  mUid.clear();
}

void AttendeeLine::clearModified()
{
  mModified = false;
  mEdit->setModified( false );
}

KPIM::MultiplyingLineData::Ptr AttendeeLine::data() const
{
  if ( isModified() ) {
    const_cast<AttendeeLine*>( this )->dataFromFields();
  }
  return mData;
}

void AttendeeLine::dataFromFields()
{
  if ( !mData ) {
    return;
  }

  KCalCore::Attendee::Ptr oldAttendee( mData->attendee() );

  QString email, name;
  KPIMUtils::extractEmailAddressAndName( mEdit->text(), email, name );

  mData->setName( name );
  mData->setEmail( email );

  mData->setRole( AttendeeData::Role( mRoleCombo->currentIndex() ) );
  mData->setStatus( AttendeeData::PartStat( mStateCombo->currentIndex() ) );
  mData->setRSVP( mResponseCombo->currentIndex() == 0 );
  mData->setUid( mUid );

  clearModified();
  if ( !( oldAttendee == mData->attendee() ) && !email.isEmpty() ) {
    // if email is empty, we don't want to update anything
    kDebug() << oldAttendee->email() << mData->email();
    emit changed( oldAttendee, mData->attendee() );
  }
}

void AttendeeLine::fieldsFromData()
{
  if( !mData ) {
    return;
  }

  mEdit->setText( mData->fullName() );
  mRoleCombo->setCurrentIndex( mData->role() );
  AttendeeData::PartStat partStat = mData->status();
  if ( partStat != AttendeeData::None ) {
    mStateCombo->setCurrentIndex( partStat );
  } else {
    mStateCombo->setCurrentIndex( AttendeeData::NeedsAction );
  }
  mResponseCombo->setCurrentIndex( mData->RSVP() ? 0 : 1 );
  mUid = mData->uid();
}

void AttendeeLine::fixTabOrder( QWidget *previous )
{
  setTabOrder( previous, mRoleCombo );
  setTabOrder( mRoleCombo, mEdit );
  setTabOrder( mEdit, mStateCombo );
  setTabOrder( mStateCombo, mResponseCombo );
}

QWidget *AttendeeLine::tabOut() const
{
  return mResponseCombo;
}

bool AttendeeLine::isActive() const
{
  return mEdit->hasFocus();
}

bool AttendeeLine::isEmpty() const
{
  return mEdit->text().isEmpty();
}

bool AttendeeLine::isModified() const
{
  return mModified || mEdit->isModified();
}

void AttendeeLine::moveCompletionPopup()
{
  if ( mEdit->completionBox( false ) ) {
    if ( mEdit->completionBox()->isVisible() ) {
      // ### trigger moving, is there a nicer way to do that?
      mEdit->completionBox()->hide();
      mEdit->completionBox()->show();
    }
  }
}

int AttendeeLine::setColumnWidth( int w )
{
  w = qMax( w, mRoleCombo->sizeHint().width() );
  mRoleCombo->setFixedWidth( w );
  mRoleCombo->updateGeometry();
  parentWidget()->updateGeometry();
  return w;
}

void AttendeeLine::setActions( AttendeeActions actions )
{
  mStateCombo->clear();
  if ( actions == EventActions ) {
#ifdef KDEPIM_MOBILE_UI
    mStateCombo->addItem( DesktopIcon( "task-attention", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::NeedsAction ) );
    mStateCombo->addItem( DesktopIcon( "task-accepted", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Accepted ) );
    mStateCombo->addItem( DesktopIcon( "task-reject", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Declined ) );
    mStateCombo->addItem( DesktopIcon( "task-attempt", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Tentative ) );
    mStateCombo->addItem( DesktopIcon( "task-delegate", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Delegated ) );
#else
    mStateCombo->addItem( SmallIcon( "task-attention" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::NeedsAction ) );
    mStateCombo->addItem( SmallIcon( "task-accepted" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Accepted ) );
    mStateCombo->addItem( SmallIcon( "task-reject" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Declined ) );
    mStateCombo->addItem( SmallIcon( "task-attempt" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Tentative ) );
    mStateCombo->addItem( SmallIcon( "task-delegate" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Delegated ) );
#endif
  } else {
#ifdef KDEPIM_MOBILE_UI
    mStateCombo->addItem( DesktopIcon( "task-attention", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::NeedsAction ) );
    mStateCombo->addItem( DesktopIcon( "task-accepted", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Accepted ) );
    mStateCombo->addItem( DesktopIcon( "task-reject", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Declined ) );
    mStateCombo->addItem( DesktopIcon( "task-attempt", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Tentative ) );
    mStateCombo->addItem( DesktopIcon( "task-delegate", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Delegated ) );
    mStateCombo->addItem( DesktopIcon( "task-complete", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Completed ) );
    mStateCombo->addItem( DesktopIcon( "task-ongoing", 48 ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::InProcess ) );
#else
    mStateCombo->addItem( SmallIcon( "task-attention" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::NeedsAction ) );
    mStateCombo->addItem( SmallIcon( "task-accepted" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Accepted ) );
    mStateCombo->addItem( SmallIcon( "task-reject" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Declined ) );
    mStateCombo->addItem( SmallIcon( "task-attempt" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Tentative ) );
    mStateCombo->addItem( SmallIcon( "task-delegate" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Delegated ) );
    mStateCombo->addItem( SmallIcon( "task-complete" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::Completed ) );
    mStateCombo->addItem( SmallIcon( "task-ongoing" ),
                          KCalUtils::Stringify::attendeeStatus( AttendeeData::InProcess ) );
#endif
  }
}

void AttendeeLine::setCompletionMode( KGlobalSettings::Completion mode )
{
//QT5
  //mEdit->setCompletionMode( mode );
}

void AttendeeLine::setData( const KPIM::MultiplyingLineData::Ptr &data )
{
  AttendeeData::Ptr attendee = qSharedPointerDynamicCast<AttendeeData>( data );
  if ( !attendee ) {
    return;
  }
  mData = attendee;
  fieldsFromData();
}

void AttendeeLine::slotHandleChange()
{
  if ( mEdit->text().isEmpty() ) {
    emit deleteLine( this );
  } else {
    // has bad side-effects, and I have no idea what this was supposed to be doing
//    mEdit->setCursorPosition( 0 );
    emit editingFinished( this );
    dataFromFields();
  }
}

void AttendeeLine::slotTextChanged( const QString &str )
{
  Q_UNUSED( str );
  mModified = true;
  emit changed(); // TODO: This doesn't seem connected to anywhere in incidenceattendee.cpp.
                  // but the important code is run in slotHandleChange() anyway so we don't see any bug
}

void AttendeeLine::slotComboChanged()
{
  mModified = true;
  // If mUid is empty, we're still populating the widget, don't write fields to data yet
  if ( !mUid.isEmpty() )
    dataFromFields();
}

void AttendeeLine::aboutToBeDeleted()
{
  if ( !mData ) {
    return;
  }

  emit changed( mData->attendee(), KCalCore::Attendee::Ptr( new KCalCore::Attendee( "", "" ) ) );
}


