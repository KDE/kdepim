/*
    This file is part of KMail.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "recipientseditor.h"

#include "recipientspicker.h"
#include "kwindowpositioner.h"
#include "distributionlistdialog.h"
#include "globalsettings.h"

#include <libemailfunctions/email.h>

#include <kapplication.h>
#include <kcompletionbox.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqscrollview.h>
#include <tqcombobox.h>
#include <tqhbox.h>
#include <tqtimer.h>
#include <tqpushbutton.h>
#include <tqstylesheet.h>

Recipient::Recipient( const TQString &email, Recipient::Type type )
  : mEmail( email ), mType( type )
{
}

void Recipient::setType( Type type )
{
  mType = type;
}

Recipient::Type Recipient::type() const
{
  return mType;
}

void Recipient::setEmail( const TQString &email )
{
  mEmail = email;
}

TQString Recipient::email() const
{
  return mEmail;
}

bool Recipient::isEmpty() const
{
  return mEmail.isEmpty();
}

int Recipient::typeToId( Recipient::Type type )
{
  return static_cast<int>( type );
}

Recipient::Type Recipient::idToType( int id )
{
  return static_cast<Type>( id );
}

TQString Recipient::typeLabel() const
{
  return typeLabel( mType );
}

TQString Recipient::typeLabel( Recipient::Type type )
{
  switch( type ) {
    case To:
      return i18n("To");
    case Cc:
      return i18n("CC");
    case Bcc:
      return i18n("BCC");
    case Undefined:
      break;
  }

  return i18n("<Undefined RecipientType>");
}

TQStringList Recipient::allTypeLabels()
{
  TQStringList types;
  types.append( typeLabel( To ) );
  types.append( typeLabel( Cc ) );
  types.append( typeLabel( Bcc ) );
  return types;
}


RecipientComboBox::RecipientComboBox( TQWidget *parent )
  : TQComboBox( parent )
{
}

void RecipientComboBox::keyPressEvent( TQKeyEvent *ev )
{
  if ( ev->key() == Key_Right ) emit rightPressed();
  else TQComboBox::keyPressEvent( ev );
}


void RecipientLineEdit::keyPressEvent( TQKeyEvent *ev )
{
  if ( ev->key() == Key_Backspace  &&  text().isEmpty() ) {
    ev->accept();
    emit deleteMe();
  } else if ( ev->key() == Key_Left && cursorPosition() == 0 ) {
    emit leftPressed();
  } else if ( ev->key() == Key_Right && cursorPosition() == (int)text().length() ) {
    emit rightPressed();
  } else {
    KMLineEdit::keyPressEvent( ev );
  }
}

RecipientLine::RecipientLine( TQWidget *parent )
  : TQWidget( parent ), mRecipientsCount( 0 ), mModified( false )
{
  TQBoxLayout *topLayout = new TQHBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  TQStringList recipientTypes = Recipient::allTypeLabels();

  mCombo = new RecipientComboBox( this );
  mCombo->insertStringList( recipientTypes );
  topLayout->addWidget( mCombo );
  TQToolTip::add( mCombo, i18n("Select type of recipient") );

  mEdit = new RecipientLineEdit( this );
  TQToolTip::add( mEdit,
                 i18n( "Set the list of email addresses to receive this message" ) );
  topLayout->addWidget( mEdit );
  connect( mEdit, TQT_SIGNAL( returnPressed() ), TQT_SLOT( slotReturnPressed() ) );
  connect( mEdit, TQT_SIGNAL( deleteMe() ), TQT_SLOT( slotPropagateDeletion() ) );
  connect( mEdit, TQT_SIGNAL( textChanged( const TQString & ) ),
    TQT_SLOT( analyzeLine( const TQString & ) ) );
  connect( mEdit, TQT_SIGNAL( focusUp() ), TQT_SLOT( slotFocusUp() ) );
  connect( mEdit, TQT_SIGNAL( focusDown() ), TQT_SLOT( slotFocusDown() ) );
  connect( mEdit, TQT_SIGNAL( rightPressed() ), TQT_SIGNAL( rightPressed() ) );

  connect( mEdit, TQT_SIGNAL( leftPressed() ), mCombo, TQT_SLOT( setFocus() ) );
  connect( mCombo, TQT_SIGNAL( rightPressed() ), mEdit, TQT_SLOT( setFocus() ) );

  connect( mCombo, TQT_SIGNAL( activated ( int ) ),
           this, TQT_SLOT( slotTypeModified() ) );

  mRemoveButton = new TQPushButton( this );
  mRemoveButton->setIconSet( KApplication::reverseLayout() ? SmallIconSet("locationbar_erase") : SmallIconSet( "clear_left" ) );
  topLayout->addWidget( mRemoveButton );
  connect( mRemoveButton, TQT_SIGNAL( clicked() ), TQT_SLOT( slotPropagateDeletion() ) );
  TQToolTip::add( mRemoveButton, i18n("Remove recipient line") );
}

void RecipientLine::slotFocusUp()
{
  emit upPressed( this );
}

void RecipientLine::slotFocusDown()
{
  emit downPressed( this );
}

void RecipientLine::slotTypeModified()
{
  mModified = true;

  emit typeModified( this );
}

void RecipientLine::analyzeLine( const TQString &text )
{
  TQStringList r = KPIM::splitEmailAddrList( text );
  if ( int( r.count() ) != mRecipientsCount ) {
    mRecipientsCount = r.count();
    emit countChanged();
  }
}

int RecipientLine::recipientsCount()
{
  return mRecipientsCount;
}

void RecipientLine::setRecipient( const Recipient &rec )
{
  mEdit->setText( rec.email() );
  mCombo->setCurrentItem( Recipient::typeToId( rec.type() ) );
}

void RecipientLine::setRecipient( const TQString &email )
{
  setRecipient( Recipient( email ) );
}

Recipient RecipientLine::recipient() const
{
  return Recipient( mEdit->text(),
    Recipient::idToType( mCombo->currentItem() ) );
}

void RecipientLine::setRecipientType( Recipient::Type type )
{
  mCombo->setCurrentItem( Recipient::typeToId( type ) );
}

Recipient::Type RecipientLine::recipientType() const
{
  return Recipient::idToType( mCombo->currentItem() );
}

void RecipientLine::activate()
{
  mEdit->setFocus();
}

bool RecipientLine::isActive()
{
  return mEdit->hasFocus();
}

bool RecipientLine::isEmpty()
{
  return mEdit->text().isEmpty();
}

bool RecipientLine::isModified()
{
  return mModified || mEdit->isModified();
}

void RecipientLine::clearModified()
{
  mModified = false;
  mEdit->clearModified();
}

void RecipientLine::slotReturnPressed()
{
  emit returnPressed( this );
}

void RecipientLine::slotPropagateDeletion()
{
  emit deleteLine( this );
}

void RecipientLine::keyPressEvent( TQKeyEvent *ev )
{
  if ( ev->key() == Key_Up ) {
    emit upPressed( this );
  } else if ( ev->key() == Key_Down ) {
    emit downPressed( this );
  }
}

int RecipientLine::setComboWidth( int w )
{
  w = QMAX( w, mCombo->sizeHint().width() );
  mCombo->setFixedWidth( w );
  mCombo->updateGeometry();
  parentWidget()->updateGeometry();
  return w;
}

void RecipientLine::fixTabOrder( TQWidget *previous )
{
  setTabOrder( previous, mCombo );
  setTabOrder( mCombo, mEdit );
  setTabOrder( mEdit, mRemoveButton );
}

TQWidget *RecipientLine::tabOut() const
{
  return mRemoveButton;
}

void RecipientLine::clear()
{
  mEdit->clear();
}

void RecipientLine::setRemoveLineButtonEnabled( bool b )
{
  mRemoveButton->setEnabled( b );
}


// ------------ RecipientsView ---------------------

RecipientsView::RecipientsView( TQWidget *parent )
  : TQScrollView( parent ), mCurDelLine( 0 ),
    mLineHeight( 0 ), mFirstColumnWidth( 0 ),
    mModified( false )
{
  mCompletionMode = KGlobalSettings::completionMode();
  setHScrollBarMode( AlwaysOff );
  setLineWidth( 0 );

  addLine();
  setResizePolicy( TQScrollView::Manual );
  setSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Expanding );

  viewport()->setPaletteBackgroundColor( paletteBackgroundColor() );
}

RecipientLine *RecipientsView::activeLine()
{
  return mLines.last();
}

RecipientLine *RecipientsView::emptyLine()
{
  RecipientLine *line;
  for( line = mLines.first(); line; line = mLines.next() ) {
    if ( line->isEmpty() ) return line;
  }

  return 0;
}

RecipientLine *RecipientsView::addLine()
{
  RecipientLine *line = new RecipientLine( viewport() );
  addChild( line, 0, mLines.count() * mLineHeight );
  line->mEdit->setCompletionMode( mCompletionMode );
  line->show();
  connect( line, TQT_SIGNAL( returnPressed( RecipientLine * ) ),
    TQT_SLOT( slotReturnPressed( RecipientLine * ) ) );
  connect( line, TQT_SIGNAL( upPressed( RecipientLine * ) ),
    TQT_SLOT( slotUpPressed( RecipientLine * ) ) );
  connect( line, TQT_SIGNAL( downPressed( RecipientLine * ) ),
    TQT_SLOT( slotDownPressed( RecipientLine * ) ) );
  connect( line, TQT_SIGNAL( rightPressed() ), TQT_SIGNAL( focusRight() ) );
  connect( line, TQT_SIGNAL( deleteLine( RecipientLine * ) ),
    TQT_SLOT( slotDecideLineDeletion( RecipientLine * ) ) );
  connect( line, TQT_SIGNAL( countChanged() ), TQT_SLOT( calculateTotal() ) );
  connect( line, TQT_SIGNAL( typeModified( RecipientLine * ) ),
    TQT_SLOT( slotTypeModified( RecipientLine * ) ) );
  connect( line->mEdit, TQT_SIGNAL( completionModeChanged( KGlobalSettings::Completion ) ),
    TQT_SLOT( setCompletionMode( KGlobalSettings::Completion ) ) );

  if ( mLines.last() ) {
    if ( mLines.count() == 1 ) {
      if ( GlobalSettings::self()->secondRecipientTypeDefault() ==
         GlobalSettings::EnumSecondRecipientTypeDefault::To ) {
        line->setRecipientType( Recipient::To );
      } else {
        if ( mLines.last()->recipientType() == Recipient::Bcc ) {
          line->setRecipientType( Recipient::To );
        } else {
          line->setRecipientType( Recipient::Cc );
        }
      }
    } else {
      line->setRecipientType( mLines.last()->recipientType() );
    }
    line->fixTabOrder( mLines.last()->tabOut() );
  }

  mLines.append( line );
  // If there is only one line, removing it makes no sense
  if ( mLines.count() == 1 ) {
    mLines.first()->setRemoveLineButtonEnabled( false );
  } else {
    mLines.first()->setRemoveLineButtonEnabled( true );
  }

  mFirstColumnWidth = line->setComboWidth( mFirstColumnWidth );

  mLineHeight = line->minimumSizeHint().height();

  line->resize( viewport()->width(), mLineHeight );

  resizeView();

  calculateTotal();

  ensureVisible( 0, mLines.count() * mLineHeight );

  return line;
}

void RecipientsView::slotTypeModified( RecipientLine *line )
{
  if ( mLines.count() == 2 ||
       ( mLines.count() == 3 && mLines.at( 2 )->isEmpty() ) ) {
    if ( mLines.at( 1 ) == line ) {
      if ( line->recipientType() == Recipient::To ) {
        GlobalSettings::self()->setSecondRecipientTypeDefault(
          GlobalSettings::EnumSecondRecipientTypeDefault::To );
      } else if ( line->recipientType() == Recipient::Cc ) {
        GlobalSettings::self()->setSecondRecipientTypeDefault(
          GlobalSettings::EnumSecondRecipientTypeDefault::Cc );
      }
    }
  }
}

void RecipientsView::calculateTotal()
{
  int count = 0;
  int empty = 0;

  RecipientLine *line;
  for( line = mLines.first(); line; line = mLines.next() ) {
    if ( line->isEmpty() ) ++empty;
    else count += line->recipientsCount();
  }

  if ( empty == 0 ) addLine();

  emit totalChanged( count, mLines.count() );
}

void RecipientsView::slotReturnPressed( RecipientLine *line )
{
  if ( !line->recipient().isEmpty() ) {
    RecipientLine *empty = emptyLine();
    if ( !empty ) empty = addLine();
    activateLine( empty );
  }
}

void RecipientsView::slotDownPressed( RecipientLine *line )
{
  int pos = mLines.find( line );
  if ( pos >= (int)mLines.count() - 1 ) {
    emit focusDown();
  } else if ( pos >= 0 ) {
    activateLine( mLines.at( pos + 1 ) );
  }
}

void RecipientsView::slotUpPressed( RecipientLine *line )
{
  int pos = mLines.find( line );
  if ( pos > 0 ) {
    activateLine( mLines.at( pos - 1 ) );
  } else {
    emit focusUp();
  }
}

void RecipientsView::slotDecideLineDeletion( RecipientLine *line )
{
  if ( !line->isEmpty() )
    mModified = true;
  if ( mLines.count() == 1 ) {
    line->clear();
  } else {
    mCurDelLine = line;
    TQTimer::singleShot( 0, this, TQT_SLOT( slotDeleteLine( ) ) );
  }
}

void RecipientsView::slotDeleteLine()
{
  if ( !mCurDelLine )
    return;

  RecipientLine *line = mCurDelLine;
  int pos = mLines.find( line );

  int newPos;
  if ( pos == 0 ) newPos = pos + 1;
  else newPos = pos - 1;

  // if there is something left to activate, do so
  if ( mLines.at( newPos ) )
    mLines.at( newPos )->activate();

  mLines.remove( line );
  removeChild( line );
  delete line;

  bool atLeastOneToLine = false;
  unsigned int firstCC = 0;
  for( uint i = pos; i < mLines.count(); ++i ) {
    RecipientLine *line = mLines.at( i );
    moveChild( line, childX( line ), childY( line ) - mLineHeight );
    if ( line->recipientType() == Recipient::To )
      atLeastOneToLine = true;
    else if ( ( line->recipientType() == Recipient::Cc ) && ( i == 0 ) )
      firstCC = i;
  }
  // only one left, can't remove that one
  if ( mLines.count() == 1 )
    mLines.first()->setRemoveLineButtonEnabled( false );

  if ( !atLeastOneToLine )
    mLines.at( firstCC )->setRecipientType( Recipient::To );

  calculateTotal();

  resizeView();
}

void RecipientsView::resizeView()
{
  resizeContents( width(), mLines.count() * mLineHeight );

  if ( mLines.count() < 6 ) {
//    setFixedHeight( mLineHeight * mLines.count() );
  }

  parentWidget()->layout()->activate();
  emit sizeHintChanged();
  TQTimer::singleShot( 0, this, TQT_SLOT(moveCompletionPopup()) );
}

void RecipientsView::activateLine( RecipientLine *line )
{
  line->activate();
  ensureVisible( 0, childY( line ) );
}

void RecipientsView::viewportResizeEvent ( TQResizeEvent *ev )
{
  for( uint i = 0; i < mLines.count(); ++i ) {
    mLines.at( i )->resize( ev->size().width(), mLineHeight );
  }
  ensureVisible( 0, mLines.count() * mLineHeight );
}

TQSize RecipientsView::sizeHint() const
{
  return TQSize( 200, mLineHeight * mLines.count() );
}

TQSize RecipientsView::minimumSizeHint() const
{
  int height;
  uint numLines = 5;
  if ( mLines.count() < numLines ) height = mLineHeight * mLines.count();
  else height = mLineHeight * numLines;
  return TQSize( 200, height );
}

Recipient::List RecipientsView::recipients() const
{
  Recipient::List recipients;

  TQPtrListIterator<RecipientLine> it( mLines );
  RecipientLine *line;
  while( ( line = it.current() ) ) {
    if ( !line->recipient().isEmpty() ) {
      recipients.append( line->recipient() );
    }

    ++it;
  }

  return recipients;
}

void RecipientsView::setCompletionMode ( KGlobalSettings::Completion mode )
{
  if ( mCompletionMode == mode )
    return;
  mCompletionMode = mode;

  TQPtrListIterator<RecipientLine> it( mLines );
  RecipientLine *line;
  while( ( line = it.current() ) ) {
    line->mEdit->blockSignals( true );
    line->mEdit->setCompletionMode( mode );
    line->mEdit->blockSignals( false );
    ++it;
  }
  emit completionModeChanged( mode ); //report change to RecipientsEditor
}

void RecipientsView::removeRecipient( const TQString & recipient,
                                      Recipient::Type type )
{
  // search a line which matches recipient and type
  TQPtrListIterator<RecipientLine> it( mLines );
  RecipientLine *line;
  while( ( line = it.current() ) ) {
    if ( ( line->recipient().email() == recipient ) &&
         ( line->recipientType() == type ) ) {
      break;
    }
    ++it;
  }
  if ( line )
    line->slotPropagateDeletion();
}

bool RecipientsView::isModified()
{
  if ( mModified )
    return true;

  TQPtrListIterator<RecipientLine> it( mLines );
  RecipientLine *line;
  while( ( line = it.current() ) ) {
    if ( line->isModified() ) {
      return true;
    }
    ++it;
  }

  return false;
}

void RecipientsView::clearModified()
{
  mModified = false;

  TQPtrListIterator<RecipientLine> it( mLines );
  RecipientLine *line;
  while( ( line = it.current() ) ) {
    line->clearModified();
    ++it;
  }
}

void RecipientsView::setFocus()
{
  if ( mLines.last()->isActive() ) setFocusBottom();
  else setFocusTop();
}

void RecipientsView::setFocusTop()
{
  RecipientLine *line = mLines.first();
  if ( line ) line->activate();
  else kdWarning() << "No first" << endl;
}

void RecipientsView::setFocusBottom()
{
  RecipientLine *line = mLines.last();
  if ( line ) line->activate();
  else  kdWarning() << "No last" << endl;
}

int RecipientsView::setFirstColumnWidth( int w )
{
  mFirstColumnWidth = w;

  TQPtrListIterator<RecipientLine> it( mLines );
  RecipientLine *line;
  while( ( line = it.current() ) ) {
    mFirstColumnWidth = line->setComboWidth( mFirstColumnWidth );
    ++it;
  }

  resizeView();
  return mFirstColumnWidth;
}

void RecipientsView::moveCompletionPopup()
{
  for( RecipientLine* line = mLines.first(); line; line = mLines.next() ) {
    if ( line->lineEdit()->completionBox( false ) ) {
      if ( line->lineEdit()->completionBox()->isVisible() ) {
        // ### trigger moving, is there a nicer way to do that?
        line->lineEdit()->completionBox()->hide();
        line->lineEdit()->completionBox()->show();
      }
    }
  }

}

RecipientsToolTip::RecipientsToolTip( RecipientsView *view, TQWidget *parent )
  : TQToolTip( parent ), mView( view )
{
}

TQString RecipientsToolTip::line( const Recipient &r )
{
  TQString txt = r.email();

  return "&nbsp;&nbsp;" + TQStyleSheet::escape( txt ) + "<br/>";
}

void RecipientsToolTip::maybeTip( const TQPoint & p )
{
  TQString text = "<qt>";

  TQString to;
  TQString cc;
  TQString bcc;

  Recipient::List recipients = mView->recipients();
  Recipient::List::ConstIterator it;
  for( it = recipients.begin(); it != recipients.end(); ++it ) {
    switch( (*it).type() ) {
      case Recipient::To:
        to += line( *it );
        break;
      case Recipient::Cc:
        cc += line( *it );
        break;
      case Recipient::Bcc:
        bcc += line( *it );
        break;
      default:
        break;
    }
  }

  text += i18n("<b>To:</b><br/>") + to;
  if ( !cc.isEmpty() ) text += i18n("<b>CC:</b><br/>") + cc;
  if ( !bcc.isEmpty() ) text += i18n("<b>BCC:</b><br/>") + bcc;

  text.append( "</qt>" );

  TQRect geometry( p + TQPoint( 2, 2 ), TQPoint( 400, 100 ) );

  tip( TQRect( p.x() - 20, p.y() - 20, 40, 40 ), text, geometry );
}


SideWidget::SideWidget( RecipientsView *view, TQWidget *parent )
  : TQWidget( parent ), mView( view ), mRecipientPicker( 0 )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );

  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->addStretch( 1 );

  mTotalLabel = new TQLabel( this );
  mTotalLabel->setAlignment( AlignCenter );
  topLayout->addWidget( mTotalLabel );
  mTotalLabel->hide();

  topLayout->addStretch( 1 );

  new RecipientsToolTip( view, mTotalLabel );

  mDistributionListButton = new TQPushButton( i18n("Save List..."), this );
  topLayout->addWidget( mDistributionListButton );
  mDistributionListButton->hide();
  connect( mDistributionListButton, TQT_SIGNAL( clicked() ),
    TQT_SIGNAL( saveDistributionList() ) );
  TQToolTip::add( mDistributionListButton,
    i18n("Save recipients as distribution list") );

  mSelectButton = new TQPushButton( i18n("Se&lect..."), this );
  topLayout->addWidget( mSelectButton );
  connect( mSelectButton, TQT_SIGNAL( clicked() ), TQT_SLOT( pickRecipient() ) );
  TQToolTip::add( mSelectButton, i18n("Select recipients from address book") );
}

SideWidget::~SideWidget()
{
}

RecipientsPicker* SideWidget::picker() const
{
  if ( !mRecipientPicker ) {
    // hacks to allow picker() to be const in the presence of lazy loading
    SideWidget *non_const_this = const_cast<SideWidget*>( this );
    mRecipientPicker = new RecipientsPicker( non_const_this );
    connect( mRecipientPicker, TQT_SIGNAL( pickedRecipient( const Recipient & ) ),
             non_const_this, TQT_SIGNAL( pickedRecipient( const Recipient & ) ) );
    mPickerPositioner = new KWindowPositioner( non_const_this, mRecipientPicker );
  }
  return mRecipientPicker;
}

void SideWidget::setFocus()
{
  mSelectButton->setFocus();
}

void SideWidget::setTotal( int recipients, int lines )
{
#if 0
  kdDebug() << "SideWidget::setTotal() recipients: " << recipients <<
    "  lines: " << lines << endl;
#endif

  TQString labelText;
  if ( recipients == 0 ) labelText = i18n("No recipients");
  else labelText = i18n("1 recipient","%n recipients", recipients );
  mTotalLabel->setText( labelText );

  if ( lines > 3 ) mTotalLabel->show();
  else mTotalLabel->hide();

  if ( lines > 2 ) mDistributionListButton->show();
  else mDistributionListButton->hide();
}

void SideWidget::pickRecipient()
{
#if 0
  TQString rec = KInputDialog::getText( "Pick Recipient",
    "Email address of recipient" );
  if ( !rec.isEmpty() ) emit pickedRecipient( rec );
#else
  RecipientsPicker *p = picker();
  p->setDefaultType( mView->activeLine()->recipientType() );
  p->setRecipients( mView->recipients() );
  p->show();
  mPickerPositioner->reposition();
  p->raise();
#endif
}


RecipientsEditor::RecipientsEditor( TQWidget *parent )
  : TQWidget( parent ), mModified( false )
{
  TQBoxLayout *topLayout = new TQHBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  mRecipientsView = new RecipientsView( this );
  topLayout->addWidget( mRecipientsView );
  connect( mRecipientsView, TQT_SIGNAL( focusUp() ), TQT_SIGNAL( focusUp() ) );
  connect( mRecipientsView, TQT_SIGNAL( focusDown() ), TQT_SIGNAL( focusDown() ) );
  connect( mRecipientsView, TQT_SIGNAL( completionModeChanged( KGlobalSettings::Completion ) ),
    TQT_SIGNAL( completionModeChanged( KGlobalSettings::Completion ) ) );

  mSideWidget = new SideWidget( mRecipientsView, this );
  topLayout->addWidget( mSideWidget );
  connect( mSideWidget, TQT_SIGNAL( pickedRecipient( const Recipient & ) ),
    TQT_SLOT( slotPickedRecipient( const Recipient & ) ) );
  connect( mSideWidget, TQT_SIGNAL( saveDistributionList() ),
    TQT_SLOT( saveDistributionList() ) );

  connect( mRecipientsView, TQT_SIGNAL( totalChanged( int, int ) ),
    mSideWidget, TQT_SLOT( setTotal( int, int ) ) );
  connect( mRecipientsView, TQT_SIGNAL( focusRight() ),
    mSideWidget, TQT_SLOT( setFocus() ) );

  connect( mRecipientsView, TQT_SIGNAL(sizeHintChanged()),
           TQT_SIGNAL(sizeHintChanged()) );
}

RecipientsEditor::~RecipientsEditor()
{
}

RecipientsPicker* RecipientsEditor::picker() const
{
  return mSideWidget->picker();
}

void RecipientsEditor::slotPickedRecipient( const Recipient &rec )
{
  RecipientLine *line = mRecipientsView->activeLine();
  if ( !line->isEmpty() ) line = mRecipientsView->addLine();

  Recipient r = rec;
  if ( r.type() == Recipient::Undefined ) {
    r.setType( line->recipientType() );
  }

  line->setRecipient( r );
  mModified = true;
}

void RecipientsEditor::saveDistributionList()
{
  DistributionListDialog *dlg = new DistributionListDialog( this );
  dlg->setRecipients( mRecipientsView->recipients() );
  dlg->exec();
  delete dlg;
}

Recipient::List RecipientsEditor::recipients() const
{
  return mRecipientsView->recipients();
}

void RecipientsEditor::setRecipientString( const TQString &str,
  Recipient::Type type )
{
  clear();

  int count = 1;

  TQStringList r = KPIM::splitEmailAddrList( str );
  TQStringList::ConstIterator it;
  for( it = r.begin(); it != r.end(); ++it ) {
    if ( count++ > GlobalSettings::self()->maximumRecipients() ) {
      KMessageBox::sorry( this,
        i18n("Truncating recipients list to %1 of %2 entries.")
        .arg( GlobalSettings::self()->maximumRecipients() )
        .arg( r.count() ) );
      break;
    }
    addRecipient( *it, type );
  }
}

TQString RecipientsEditor::recipientString( Recipient::Type type )
{
  TQString str;

  Recipient::List recipients = mRecipientsView->recipients();
  Recipient::List::ConstIterator it;
  for( it = recipients.begin(); it != recipients.end(); ++it ) {
    if ( (*it).type() == type ) {
      if ( !str.isEmpty() ) str += ", ";
      str.append( (*it).email() );
    }
  }

  return str;
}

void RecipientsEditor::addRecipient( const TQString & recipient,
                                     Recipient::Type type )
{
  RecipientLine *line = mRecipientsView->emptyLine();
  if ( !line ) line = mRecipientsView->addLine();
  line->setRecipient( Recipient( recipient, type ) );
}

void RecipientsEditor::removeRecipient( const TQString & recipient,
                                        Recipient::Type type )
{
  mRecipientsView->removeRecipient( recipient, type );
}

bool RecipientsEditor::isModified()
{
  return mModified || mRecipientsView->isModified();
}

void RecipientsEditor::clearModified()
{
  mModified = false;
  mRecipientsView->clearModified();
}

void RecipientsEditor::clear()
{
}

void RecipientsEditor::setFocus()
{
  mRecipientsView->setFocus();
}

void RecipientsEditor::setFocusTop()
{
  mRecipientsView->setFocusTop();
}

void RecipientsEditor::setFocusBottom()
{
  mRecipientsView->setFocusBottom();
}

int RecipientsEditor::setFirstColumnWidth( int w )
{
  return mRecipientsView->setFirstColumnWidth( w );
}

void RecipientsEditor::selectRecipients()
{
  mSideWidget->pickRecipient();
}

void RecipientsEditor::setCompletionMode( KGlobalSettings::Completion mode )
{
  mRecipientsView->setCompletionMode( mode );
}

#include "recipientseditor.moc"
