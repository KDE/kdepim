/*  -*- c++ -*-
    vacationdialog.cpp

    KMail, the KDE mail client.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vacationdialog.h"

#include <kmime_header_parsing.h>
using KMime::Types::AddrSpecList;
using KMime::Types::AddressList;
using KMime::Types::MailboxList;
using KMime::HeaderParsing::parseAddressList;

#include <knuminput.h>
#include <klocale.h>
#include <kdebug.h>
#include <kwin.h>
#include <kapplication.h>

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqlineedit.h>
#include <tqtextedit.h>
#include <tqvalidator.h>

namespace KMail {

  VacationDialog::VacationDialog( const TQString & caption, TQWidget * parent,
				  const char * name, bool modal )
    : KDialogBase( Plain, caption, Ok|Cancel|Default, Ok, parent, name, modal )
  {
    KWin::setIcons( winId(), kapp->icon(), kapp->miniIcon() );

    static const int rows = 7;
    int row = -1;

    TQGridLayout * glay = new TQGridLayout( plainPage(), rows, 2, 0, spacingHint() );
    glay->setColStretch( 1, 1 );

    // explanation label:
    ++row;
    glay->addMultiCellWidget( new TQLabel( i18n("Configure vacation "
					       "notifications to be sent:"),
					  plainPage() ), row, row, 0, 1 );

    // Activate checkbox:
    ++row;
    mActiveCheck = new TQCheckBox( i18n("&Activate vacation notifications"), plainPage() );
    glay->addMultiCellWidget( mActiveCheck, row, row, 0, 1 );

    // Message text edit:
    ++row;
    glay->setRowStretch( row, 1 );
    mTextEdit = new TQTextEdit( plainPage(), "mTextEdit" );
    mTextEdit->setTextFormat( TQTextEdit::PlainText );
    glay->addMultiCellWidget( mTextEdit, row, row, 0, 1 );

    // "Resent only after" spinbox and label:
    ++row;
    int defDayInterval = 7; //default day interval
    mIntervalSpin = new KIntSpinBox( 1, 356, 1, defDayInterval, 10, plainPage(), "mIntervalSpin" );
    mIntervalSpin->setSuffix( i18n(" day", " days", defDayInterval) );
    connect(mIntervalSpin, TQT_SIGNAL( valueChanged( int )), TQT_SLOT( slotIntervalSpinChanged( int ) ) );
    glay->addWidget( new TQLabel( mIntervalSpin, i18n("&Resend notification only after:"), plainPage() ), row, 0 );
    glay->addWidget( mIntervalSpin, row, 1 );

    // "Send responses for these addresses" lineedit and label:
    ++row;
    mMailAliasesEdit = new TQLineEdit( plainPage(), "mMailAliasesEdit" );
    glay->addWidget( new TQLabel( mMailAliasesEdit, i18n("&Send responses for these addresses:"), plainPage() ), row, 0 );
    glay->addWidget( mMailAliasesEdit, row, 1 );

    // "Send responses also to SPAM mail" checkbox:
    ++row;
    mSpamCheck = new TQCheckBox( i18n("Do not send vacation replies to spam messages"), plainPage(), "mSpamCheck" );
    mSpamCheck->setChecked( true );
    glay->addMultiCellWidget( mSpamCheck, row, row, 0, 1 );

    //  domain checkbox and linedit:
    ++row;
    mDomainCheck = new TQCheckBox( i18n("Only react to mail coming from domain"), plainPage(), "mDomainCheck" );
    mDomainCheck->setChecked( false );
    mDomainEdit = new TQLineEdit( plainPage(), "mDomainEdit" );
    mDomainEdit->setEnabled( false );
    mDomainEdit->setValidator( new TQRegExpValidator( TQRegExp( "[a-zA-Z0-9+-]+(?:\\.[a-zA-Z0-9+-]+)*" ), mDomainEdit ) );
    glay->addWidget( mDomainCheck, row, 0 );
    glay->addWidget( mDomainEdit, row, 1 );
    connect( mDomainCheck, TQT_SIGNAL(toggled(bool)),
             mDomainEdit, TQT_SLOT(setEnabled(bool)) );

    Q_ASSERT( row == rows - 1 );
  }

  VacationDialog::~VacationDialog() {
    kdDebug(5006) << "~VacationDialog()" << endl;
  }

  bool VacationDialog::activateVacation() const {
    return mActiveCheck->isChecked();
  }

  void VacationDialog::setActivateVacation( bool activate ) {
    mActiveCheck->setChecked( activate );
  }

  TQString VacationDialog::messageText() const {
    return mTextEdit->text().stripWhiteSpace();
  }

  void VacationDialog::setMessageText( const TQString & text ) {
    mTextEdit->setText( text );
    const int height = ( mTextEdit->fontMetrics().lineSpacing() + 1 ) * 11;
    mTextEdit->setMinimumHeight( height );
  }

  int VacationDialog::notificationInterval() const {
    return mIntervalSpin->value();
  }

  void VacationDialog::setNotificationInterval( int days ) {
    mIntervalSpin->setValue( days );
  }

  AddrSpecList VacationDialog::mailAliases() const {
    TQCString text = mMailAliasesEdit->text().latin1(); // ### IMAA: !ok
    AddressList al;
    const char * s = text.begin();
    parseAddressList( s, text.end(), al );

    AddrSpecList asl;
    for ( AddressList::const_iterator it = al.begin() ; it != al.end() ; ++it ) {
      const MailboxList & mbl = (*it).mailboxList;
      for ( MailboxList::const_iterator jt = mbl.begin() ; jt != mbl.end() ; ++jt )
	asl.push_back( (*jt).addrSpec );
    }
    return asl;
  }

  void VacationDialog::setMailAliases( const AddrSpecList & aliases ) {
    TQStringList sl;
    for ( AddrSpecList::const_iterator it = aliases.begin() ; it != aliases.end() ; ++it )
      sl.push_back( (*it).asString() );
    mMailAliasesEdit->setText( sl.join(", ") );
  }

  void VacationDialog::setMailAliases( const TQString & aliases ) {
    mMailAliasesEdit->setText( aliases );
  }

  void VacationDialog::slotIntervalSpinChanged ( int value ) {
    mIntervalSpin->setSuffix( i18n(" day", " days", value) );
  }

  TQString VacationDialog::domainName() const {
    return mDomainCheck->isChecked() ? mDomainEdit->text() : TQString::null ;
  }

  void VacationDialog::setDomainName( const TQString & domain ) {
    if ( !domain.isEmpty() ) {
      mDomainEdit->setText( domain );
      mDomainCheck->setChecked( true );
    }
  }

  bool VacationDialog::domainCheck() const
  {
    return mDomainCheck->isChecked();
  }

  void VacationDialog::setDomainCheck( bool check )
  {
    mDomainCheck->setChecked( check );
  }

  bool VacationDialog::sendForSpam() const
  {
    return !mSpamCheck->isChecked();
  }

  void VacationDialog::setSendForSpam( bool enable )
  {
    mSpamCheck->setChecked( !enable );
  }

  /* virtual*/
  void KMail::VacationDialog::enableDomainAndSendForSpam( bool enable )
  {
    mDomainCheck->setEnabled( enable );
    mDomainEdit->setEnabled( enable && mDomainCheck->isChecked() );
    mSpamCheck->setEnabled( enable );
  }

} // namespace KMail

#include "vacationdialog.moc"
