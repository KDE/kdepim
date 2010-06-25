/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#include <KPIMUtils/Email>

#include <KCompletionBox>
#include <KDialog>
#include <KDebug>
#include <KLocale>

#include <QBoxLayout>
#include <QMenu>

using namespace IncidenceEditorsNG;
using namespace KPIM;

IncidenceEditorsNG::AttendeeCheckBox::AttendeeCheckBox( QWidget* parent )
  : QCheckBox( parent )
{}

void AttendeeCheckBox::keyPressEvent(QKeyEvent* ev)
{
  if ( ev->key() == Qt::Key_Left )
    emit leftPressed();
  else  if ( ev->key() == Qt::Key_Right )
    emit rightPressed();
  else
    QAbstractButton::keyPressEvent( ev );
}

typedef QPair<QString, QIcon> TextIconPair;

AttendeeComboBox::AttendeeComboBox( QWidget* parent )
  : QToolButton( parent ), mMenu( new QMenu( this  ) ), mCurrentIndex( -1 )
{
  setPopupMode( QToolButton::InstantPopup );
  setToolButtonStyle( Qt::ToolButtonIconOnly );
  setMenu( mMenu );
}

void AttendeeComboBox::addItem( const QIcon& icon, const QString& text )
{
  mList.append( TextIconPair( text, icon )  );
  if( mCurrentIndex == -1 )
    setCurrentIndex( 0 );
  int index = mList.size() - 1;
  QAction *act = menu()->addAction( icon, text, this, SLOT( slotActionTriggered() ) );
  act->setData( index );
}

void AttendeeComboBox::addItems(const QStringList& texts)
{
  foreach( QString str, texts )
    addItem( QIcon(), str );
  if( mCurrentIndex == -1 )
    setCurrentIndex( 0 );
}

int AttendeeComboBox::currentIndex() const
{
  return mCurrentIndex;
}

void AttendeeComboBox::setCurrentIndex( int index )
{
  Q_ASSERT( index < mList.size() );
  mCurrentIndex = index;
  setIcon( mList.at( index ).second );
}

void AttendeeComboBox::slotActionTriggered()
{
  int index = qobject_cast<QAction*> ( sender() )->data().toInt();
  setCurrentIndex( index );
}

void AttendeeComboBox::keyPressEvent(QKeyEvent* ev)
{
  if ( ev->key() == Qt::Key_Left ) {
    emit leftPressed();
  } else if ( ev->key() == Qt::Key_Right ) {  
    emit rightPressed();
  } else {
    QToolButton::keyPressEvent( ev );
  }
}

AttendeeLineEdit::AttendeeLineEdit( QWidget* parent )
  : AddresseeLineEdit( parent, true )
{}

void AttendeeLineEdit::keyPressEvent( QKeyEvent* ev )
{
  if ( (ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return) &&
      !completionBox()->isVisible() )
  {
    emit downPressed();
    KPIM::AddresseeLineEdit::keyPressEvent( ev );
  } else if ( ev->key() == Qt::Key_Backspace  &&  text().isEmpty() ) {
    ev->accept();
    emit deleteMe();
  } else if ( ev->key() == Qt::Key_Left && cursorPosition() == 0 &&
              !ev->modifiers().testFlag( Qt::ShiftModifier ) ) {  // Shift would be pressed during selection
    emit leftPressed();
  } else if ( ev->key() == Qt::Key_Right && cursorPosition() == (int)text().length() &&
              !ev->modifiers().testFlag( Qt::ShiftModifier ) ) {  // Shift would be pressed during selection
    emit rightPressed();
  } else if ( ev->key() == Qt::Key_Down ) {
    emit downPressed();
  } else if ( ev->key() == Qt::Key_Up ) {
    emit upPressed();
  } else {
    KPIM::AddresseeLineEdit::keyPressEvent( ev );
  }
}

AttendeeLine::AttendeeLine(QWidget* parent)
  : MultiplyingLine( parent )
  , mRoleCombo(  new AttendeeComboBox( this ) )
  , mStateCombo( new AttendeeComboBox( this ) )
  , mResponseCheck( new AttendeeCheckBox( this ) )
  , mEdit( new AttendeeLineEdit( this ) )
  , mData( new AttendeeData( QString(), QString() ) )
  , mModified( false )
{
   setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );

  QBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );

  mRoleCombo->addItem( SmallIcon( "meeting-participant" ), // TODO: Icon
                       AttendeeData::roleName( KCal::Attendee::ReqParticipant ) );
  mRoleCombo->addItem( SmallIcon( "meeting-participant" ),
                       AttendeeData::roleName( KCal::Attendee::OptParticipant ) );
  mRoleCombo->addItem( SmallIcon( "meeting-observer" ),
                       AttendeeData::roleName( KCal::Attendee::NonParticipant ) );
  mRoleCombo->addItem( SmallIcon( "meeting-organizer" ),  // TODO: Icon chair
                       AttendeeData::roleName( KCal::Attendee::Chair ) );

  mRoleCombo->setToolTip( i18nc( "@info:tooltip", "Select the attendee participation role" ) );

  mEdit->setToolTip( i18n( "Enter the name or email address of the attendee." ) );
  mEdit->setClearButtonShown( true );
  

  mStateCombo->setToolTip( i18nc( "@info:tooltip", "Select the attendee participation status" ) );
  mStateCombo->setWhatsThis( i18nc( "@info:whatsthis",
                     "Edits the current attendance status of the attendee." ) );
  //TODO: the icons below aren't exactly correct
  mStateCombo->addItem( SmallIcon( "help-about" ),
                         AttendeeData::statusName( AttendeeData::NeedsAction ) );
  mStateCombo->addItem( SmallIcon( "dialog-ok-apply" ),
                         AttendeeData::statusName( AttendeeData::Accepted ) );
  mStateCombo->addItem( SmallIcon( "dialog-cancel" ),
                         AttendeeData::statusName( AttendeeData::Declined ) );
  mStateCombo->addItem( SmallIcon( "dialog-ok" ),
                         AttendeeData::statusName( AttendeeData::Tentative ) );
  mStateCombo->addItem( SmallIcon( "mail-forward" ),
                         AttendeeData::statusName( AttendeeData::Delegated ) );
  mStateCombo->addItem( SmallIcon( "mail-mark-read" ),
                         AttendeeData::statusName( AttendeeData::Completed ) ),
  mStateCombo->addItem( SmallIcon( "help-about" ),
                         AttendeeData::statusName( AttendeeData::InProcess ) );

  mResponseCheck->setText( i18nc( "@option:check", "Request RSVP" ) );
  mResponseCheck->setToolTip(i18nc( "@info:tooltip", "Request a response from the attendee" ) );
  mResponseCheck->setWhatsThis( i18nc( "@info:whatsthis",
           "Edits whether to send an email to the "
           "attendee to request a response concerning "
           "attendance." ) );

  // add them to the layout in the correct order
  topLayout->addWidget( mRoleCombo );
  topLayout->addWidget( mEdit );
  topLayout->addWidget( mStateCombo );
  topLayout->addWidget( mResponseCheck );

  connect( mEdit, SIGNAL( returnPressed() ), SLOT( slotReturnPressed() ) );
  connect( mEdit, SIGNAL( deleteMe() ), SLOT( slotPropagateDeletion() ) );
  connect( mEdit, SIGNAL( textChanged( const QString & ) ),
    SLOT( slotTextChanged( const QString & ) ) );
  connect( mEdit, SIGNAL( upPressed() ), SLOT( slotFocusUp() ) );
  connect( mEdit, SIGNAL( downPressed() ), SLOT( slotFocusDown() ) );
  


  connect( mRoleCombo, SIGNAL( rightPressed() ), mEdit, SLOT( setFocus() ) );
  connect( mEdit, SIGNAL( leftPressed() ), mRoleCombo, SLOT( setFocus() ) );

  connect( mEdit, SIGNAL( rightPressed() ), mStateCombo, SLOT( setFocus() ) );
  connect( mStateCombo, SIGNAL( leftPressed() ), mEdit, SLOT( setFocus() ) );

  connect( mStateCombo, SIGNAL( rightPressed() ), mResponseCheck, SLOT( setFocus() ) );

  connect( mResponseCheck, SIGNAL( leftPressed() ), mStateCombo, SLOT( setFocus() ) );
  connect( mResponseCheck, SIGNAL( rightPressed() ), SIGNAL( rightPressed() ) );
  
  connect( mEdit, SIGNAL( editingFinished() ), SLOT( slotEditingFinished() ) );
  connect( mEdit, SIGNAL( clearButtonClicked() ), SLOT( slotPropagateDeletion() ) );

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
  mResponseCheck->setChecked( true );
  mUid.clear();
}

void AttendeeLine::clearModified()
{
  mModified = false;
  mEdit->setModified( false );
}

MultiplyingLineData::Ptr AttendeeLine::data() const
{
  if( isModified() )
    const_cast<AttendeeLine*>( this )->dataFromFields();
  return mData;
}

void AttendeeLine::dataFromFields()
{
  if( !mData )
    return;
  mData->setEmail(  mEdit->text() );
  mData->setRole( AttendeeData::Role( mRoleCombo->currentIndex() ) );
  mData->setStatus( AttendeeData::PartStat( mStateCombo->currentIndex() ) );
  mData->setRSVP( mResponseCheck->isChecked() );
  mData->setUid( mUid );
}

void AttendeeLine::fieldsFromData()
{
  if( !mData )
    return;
  mEdit->setText( mData->email() );
  mRoleCombo->setCurrentIndex( mData->role() );
  AttendeeData::PartStat partStat = mData->status();
  if ( partStat != AttendeeData::None ) {
    mStateCombo->setCurrentIndex( partStat );
  } else {
    mStateCombo->setCurrentIndex( AttendeeData::NeedsAction );
  }
  mUid = mData->uid();
}

void AttendeeLine::fixTabOrder( QWidget* previous )
{
  setTabOrder( previous, mRoleCombo );
  setTabOrder( mRoleCombo, mEdit );
  setTabOrder( mEdit, mStateCombo );
  setTabOrder( mStateCombo, mResponseCheck );
}

QWidget* AttendeeLine::tabOut() const
{
  return mResponseCheck;
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

void AttendeeLine::setCompletionMode( KGlobalSettings::Completion mode )
{
  mEdit->setCompletionMode( mode );
}

void AttendeeLine::setData( const KPIM::MultiplyingLineData::Ptr& data )
{
  AttendeeData::Ptr attendee = qSharedPointerDynamicCast<AttendeeData>( data );
  if( !attendee )
    return;
  mData = attendee;
  fieldsFromData();
}

void AttendeeLine::slotEditingFinished()
{
  if ( mEdit->text().isEmpty() ) {
    emit deleteLine( this );
  } else {
    mEdit->setCursorPosition( 0 );
  }
}

void AttendeeLine::slotTextChanged( const QString& /*str*/ )
{
  
  //TODO: some verifying, auto completion and stuff
  //      to assist the user in selecting a valid contact
  //   KPIMUtils::isValidAddress( str );
  emit changed();
}

#include "attendeeline.moc"

