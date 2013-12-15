/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "vacationeditwidget.h"

#include <KLocalizedString>
#include <KIntSpinBox>
#include <KLineEdit>

#include <pimcommon/texteditor/richtexteditor/richtexteditorwidget.h>

#include <kmime/kmime_header_parsing.h>


#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <kdialog.h>

using KMime::Types::AddrSpecList;
using KMime::Types::AddressList;
using KMime::Types::MailboxList;
using KMime::HeaderParsing::parseAddressList;


using namespace KSieveUi;
VacationEditWidget::VacationEditWidget(QWidget *parent)
    : QWidget(parent)
{
    int row = -1;

    QGridLayout * glay = new QGridLayout( this );
    glay->setSpacing( KDialog::spacingHint() );
    glay->setMargin( 0 );
    glay->setColumnStretch( 1, 1 );

    // explanation label:
    ++row;
    glay->addWidget( new QLabel( i18n("Configure vacation "
                                      "notifications to be sent:"),
                                 this ), row, 0, 1, 2 );

    // Activate checkbox:
    ++row;
    mActiveCheck = new QCheckBox( i18n("&Activate vacation notifications"), this );
    glay->addWidget( mActiveCheck, row, 0, 1, 2 );

    // Message text edit:
    ++row;
    glay->setRowStretch( row, 1 );
    mTextEdit = new PimCommon::RichTextEditorWidget( this );
    mTextEdit->setObjectName( QLatin1String("mTextEdit") );
    mTextEdit->setAcceptRichText( false );
    glay->addWidget( mTextEdit, row, 0, 1, 2 );

    // "Resent only after" spinbox and label:
    ++row;
    int defDayInterval = 7; //default day interval
    mIntervalSpin = new KIntSpinBox( 1, 356, 1, defDayInterval, this );
    mIntervalSpin->setObjectName( QLatin1String("mIntervalSpin") );
    mIntervalSpin->setSuffix( i18np(" day", " days", defDayInterval) );
    connect(mIntervalSpin, SIGNAL(valueChanged(int)), SLOT(slotIntervalSpinChanged(int)) );
    QLabel *label = new QLabel( i18n("&Resend notification only after:"), this );
    label->setBuddy( mIntervalSpin );
    glay->addWidget( label, row, 0 );
    glay->addWidget( mIntervalSpin, row, 1 );

    // "Send responses for these addresses" lineedit and label:
    ++row;
    mMailAliasesEdit = new KLineEdit( this );
    mMailAliasesEdit->setObjectName( QLatin1String("mMailAliasesEdit") );
    mMailAliasesEdit->setClearButtonShown( true );
    QLabel *tmpLabel = new QLabel( i18n("&Send responses for these addresses:"), this );
    tmpLabel->setBuddy( mMailAliasesEdit );
    glay->addWidget( tmpLabel, row, 0 );
    glay->addWidget( mMailAliasesEdit, row, 1 );

    // "Send responses also to SPAM mail" checkbox:
    ++row;
    mSpamCheck = new QCheckBox( i18n("Do not send vacation replies to spam messages"), this );
    mSpamCheck->setObjectName( QLatin1String("mSpamCheck") );
    mSpamCheck->setChecked( true );
    glay->addWidget( mSpamCheck, row, 0, 1, 2 );

    //  domain checkbox and linedit:
    ++row;
    mDomainCheck = new QCheckBox( i18n("Only react to mail coming from domain"), this );
    mDomainCheck->setObjectName( QLatin1String("mDomainCheck") );
    mDomainCheck->setChecked( false );
    mDomainEdit = new KLineEdit( this );
    mDomainEdit->setObjectName( QLatin1String("mDomainEdit") );
    mDomainEdit->setClearButtonShown( true );
    mDomainEdit->setEnabled( false );
    mDomainEdit->setValidator( new QRegExpValidator( QRegExp( QLatin1String("[a-zA-Z0-9+-]+(?:\\.[a-zA-Z0-9+-]+)*") ), mDomainEdit ) );
    glay->addWidget( mDomainCheck, row, 0 );
    glay->addWidget( mDomainEdit, row, 1 );
    connect( mDomainCheck, SIGNAL(toggled(bool)),
             mDomainEdit, SLOT(setEnabled(bool)) );
}

VacationEditWidget::~VacationEditWidget()
{

}

bool VacationEditWidget::activateVacation() const
{
    return mActiveCheck->isChecked();
}

void VacationEditWidget::setActivateVacation( bool activate )
{
    mActiveCheck->setChecked( activate );
}

QString VacationEditWidget::messageText() const
{
    return mTextEdit->toPlainText().trimmed();
}

void VacationEditWidget::setMessageText( const QString &text )
{
    mTextEdit->setPlainText( text );
    const int height = ( mTextEdit->fontMetrics().lineSpacing() + 1 ) * 11;
    mTextEdit->setMinimumHeight( height );
}

int VacationEditWidget::notificationInterval() const
{
    return mIntervalSpin->value();
}

void VacationEditWidget::setNotificationInterval( int days )
{
    mIntervalSpin->setValue( days );
}

AddrSpecList VacationEditWidget::mailAliases() const
{
    QByteArray text = mMailAliasesEdit->text().toLatin1(); // ### IMAA: !ok
    AddressList al;
    const char * s = text.begin();
    parseAddressList( s, text.end(), al );

    AddrSpecList asl;
    AddressList::const_iterator end(al.constEnd());
    for ( AddressList::const_iterator it = al.constBegin() ; it != end; ++it ) {
        const MailboxList & mbl = (*it).mailboxList;
        for ( MailboxList::const_iterator jt = mbl.constBegin() ; jt != mbl.constEnd() ; ++jt )
            asl.push_back( (*jt).addrSpec() );
    }
    return asl;
}

void VacationEditWidget::setMailAliases( const AddrSpecList &aliases )
{
    QStringList sl;
    AddrSpecList::const_iterator end(aliases.constEnd());
    for ( AddrSpecList::const_iterator it = aliases.constBegin() ; it != end; ++it )
        sl.push_back( (*it).asString() );
    mMailAliasesEdit->setText( sl.join(QLatin1String(", ")) );
}

void VacationEditWidget::setMailAliases( const QString &aliases )
{
    mMailAliasesEdit->setText( aliases );
}

void VacationEditWidget::slotIntervalSpinChanged ( int value )
{
    mIntervalSpin->setSuffix( i18np(" day", " days", value) );
}

QString VacationEditWidget::domainName() const
{
    return mDomainCheck->isChecked() ? mDomainEdit->text() : QString() ;
}

void VacationEditWidget::setDomainName( const QString &domain )
{
    if ( !domain.isEmpty() ) {
        mDomainEdit->setText( domain );
        mDomainCheck->setChecked( true );
    }
}

bool VacationEditWidget::domainCheck() const
{
    return mDomainCheck->isChecked();
}

void VacationEditWidget::setDomainCheck( bool check )
{
    mDomainCheck->setChecked( check );
}

bool VacationEditWidget::sendForSpam() const
{
    return !mSpamCheck->isChecked();
}

void VacationEditWidget::setSendForSpam( bool enable )
{
    mSpamCheck->setChecked( !enable );
}

void VacationEditWidget::enableDomainAndSendForSpam( bool enable )
{
    mDomainCheck->setEnabled( enable );
    mDomainEdit->setEnabled( enable && mDomainCheck->isChecked() );
    mSpamCheck->setEnabled( enable );
}

