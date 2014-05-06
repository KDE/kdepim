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

#include "recipientline.h"

#include <KPIMUtils/kpimutils/email.h>
#include <KDialog>
#include <KLocalizedString>
#include <KCompletionBox>
#include <QDebug>

#include <QKeyEvent>
#include <QHBoxLayout>

using namespace MessageComposer;
using namespace KPIM;

RecipientComboBox::RecipientComboBox( QWidget *parent )
    : KComboBox( parent )
{
}

void RecipientComboBox::keyPressEvent( QKeyEvent *ev )
{
    if ( ev->key() == Qt::Key_Right ) emit rightPressed();
    else KComboBox::keyPressEvent( ev );
}

RecipientLineEdit::RecipientLineEdit ( QWidget* parent ) : ComposerLineEdit ( parent )
{
}

void RecipientLineEdit::keyPressEvent( QKeyEvent *ev )
{
    //Laurent Bug:280153
    /*if ( ev->key() == Qt::Key_Backspace  &&  text().isEmpty() ) {
    ev->accept();
    emit deleteMe();
  } else */
    if ( ev->key() == Qt::Key_Left && cursorPosition() == 0 &&
         !ev->modifiers().testFlag( Qt::ShiftModifier ) ) {  // Shift would be pressed during selection
        emit leftPressed();
    } else if ( ev->key() == Qt::Key_Right && cursorPosition() == (int)text().length() &&
                !ev->modifiers().testFlag( Qt::ShiftModifier ) ) {  // Shift would be pressed during selection
        emit rightPressed();
    } else {
        MessageComposer::ComposerLineEdit::keyPressEvent( ev );
    }
}


RecipientLineNG::RecipientLineNG( QWidget* parent )
    : MultiplyingLine( parent ), mRecipientsCount( 0 ), mModified( false ), mData( new Recipient )
{
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );

    QBoxLayout *topLayout = new QHBoxLayout( this );
    topLayout->setSpacing( KDialog::spacingHint() );
    topLayout->setMargin( 0 );

    const QStringList recipientTypes = Recipient::allTypeLabels();

    mCombo = new RecipientComboBox( this );
    mCombo->addItems( recipientTypes );
    topLayout->addWidget( mCombo );
    mCombo->setToolTip( i18nc("@label:listbox","Select type of recipient") );
    mEdit = new RecipientLineEdit( this );
    mEdit->setToolTip( i18n( "Set the list of email addresses to receive this message" ) );
    mEdit->setClearButtonShown( true );
    topLayout->addWidget( mEdit );

    connect( mEdit, SIGNAL(returnPressed()), SLOT(slotReturnPressed()) );
    connect( mEdit, SIGNAL(deleteMe()), SLOT(slotPropagateDeletion()) );
    connect( mEdit, SIGNAL(textChanged(QString)),
             SLOT(analyzeLine(QString)) );
    connect( mEdit, SIGNAL(focusUp()), SLOT(slotFocusUp()) );
    connect( mEdit, SIGNAL(focusDown()), SLOT(slotFocusDown()) );
    connect( mEdit, SIGNAL(rightPressed()), SIGNAL(rightPressed()) );

    connect( mEdit, SIGNAL(leftPressed()), mCombo, SLOT(setFocus()) );
    connect( mEdit, SIGNAL(editingFinished()), SLOT(slotEditingFinished()) );
    connect( mEdit, SIGNAL(clearButtonClicked()), SLOT(slotPropagateDeletion()) );
    connect( mCombo, SIGNAL(rightPressed()), mEdit, SLOT(setFocus()) );

    connect( mCombo, SIGNAL(activated(int)),
             this, SLOT(slotTypeModified()) );
}

void RecipientLineNG::slotEditingFinished()
{
    if ( mEdit->text().isEmpty() ) {
        emit deleteLine( this );
    }
}

void RecipientLineNG::slotTypeModified()
{
    mModified = true;

    emit typeModified( this );
}

void RecipientLineNG::analyzeLine( const QString &text )
{
    const QStringList r = KPIMUtils::splitAddressList( text );
    if ( int( r.count() ) != mRecipientsCount ) {
        mRecipientsCount = r.count();
    }
    mModified = true;
    emit countChanged();
}

int RecipientLineNG::recipientsCount() const
{
    return mRecipientsCount;
}

void RecipientLineNG::setData( const MultiplyingLineData::Ptr &data )
{
    Recipient::Ptr rec = qSharedPointerDynamicCast<Recipient>( data );
    if( !rec )
        return;
    //TODO laurent: mem leak ????? mData(new Recipient) in constructor, never deleted.
    mData = rec;
    fieldsFromData();
}

MultiplyingLineData::Ptr RecipientLineNG::data() const
{
    if( isModified() )
        const_cast<RecipientLineNG*>(this)->dataFromFields();
    return mData;
}

void RecipientLineNG::dataFromFields()
{
    if( !mData )
        return;
    mData->setEmail(  mEdit->text() );
    mData->setType( Recipient::idToType( mCombo->currentIndex() ) );
    mModified = false;
}

void RecipientLineNG::fieldsFromData()
{
    if( !mData )
        return;
    mCombo->setCurrentIndex( Recipient::typeToId( mData->type() ) );
    mEdit->setText( mData->email() );
}

void RecipientLineNG::activate()
{
    mEdit->setFocus();
}

bool RecipientLineNG::isActive() const
{
    return mEdit->hasFocus();
}

bool RecipientLineNG::isEmpty() const
{
    return mEdit->text().isEmpty();
}

bool RecipientLineNG::isModified() const
{
    return mModified || mEdit->isModified();
}

void RecipientLineNG::clearModified()
{
    mModified = false;
    mEdit->setModified( false );
}

int RecipientLineNG::setColumnWidth( int w )
{
    w = qMax( w, mCombo->sizeHint().width() );
    mCombo->setFixedWidth( w );
    mCombo->updateGeometry();
    parentWidget()->updateGeometry();
    return w;
}

void RecipientLineNG::fixTabOrder( QWidget *previous )
{
    setTabOrder( previous, mCombo );
    setTabOrder( mCombo, mEdit );
}

QWidget* RecipientLineNG::tabOut() const
{
    return mEdit;
}

void RecipientLineNG::clear()
{
    mEdit->clear();
}

void RecipientLineNG::moveCompletionPopup()
{
    if ( mEdit->completionBox( false ) ) {
        if ( mEdit->completionBox()->isVisible() ) {
            // ### trigger moving, is there a nicer way to do that?
            mEdit->completionBox()->hide();
            mEdit->completionBox()->show();
        }
    }
}

void RecipientLineNG::setCompletionMode( KCompletion::CompletionMode mode )
{
    mEdit->setCompletionMode( mode );
}

Recipient::Type RecipientLineNG::recipientType() const
{
    return Recipient::idToType( mCombo->currentIndex() );
}

void RecipientLineNG::setRecipientType( Recipient::Type type )
{
    mCombo->setCurrentIndex( Recipient::typeToId( type ) );
    slotTypeModified();
}

void RecipientLineNG::setRecentAddressConfig( KConfig* config )
{
    mEdit->setRecentAddressConfig( config );
}

Recipient::Ptr RecipientLineNG::recipient() const
{
    return qSharedPointerDynamicCast<Recipient>( data() );
}


