/*
    Copyright (c) 2003 Andreas Gungl <a.gungl@gmx.de>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "redirectdialog.h"

#include "mailkernel.h"

#include <akonadi/contact/emailaddressselectiondialog.h>
#include <kpimutils/email.h>
#include <messagecomposer/composerlineedit.h>
#include <messageviewer/autoqpointer.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kvbox.h>

#include <QtCore/QStringList>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QTreeView>

using namespace MailCommon;

class RedirectDialog::Private
{
  public:
    Private( RedirectDialog *qq, RedirectDialog::SendMode mode )
      : q( qq ), mSendMode( mode )
    {
    }

    void slotUser1();
    void slotUser2();
    void slotAddressSelection();
    void slotAddressChanged( const QString& );

    RedirectDialog *q;
    QLabel *mLabelTo;
    MessageComposer::ComposerLineEdit *mEditTo;
    QPushButton *mBtnTo;

    QString mResentTo;
    RedirectDialog::SendMode mSendMode;
};

void RedirectDialog::Private::slotUser1()
{
  mSendMode = RedirectDialog::SendNow;
  q->accept();
}

void RedirectDialog::Private::slotUser2()
{
  mSendMode = RedirectDialog::SendLater;
  q->accept();
}

void RedirectDialog::Private::slotAddressSelection()
{
  MessageViewer::AutoQPointer<Akonadi::EmailAddressSelectionDialog> dlg( new Akonadi::EmailAddressSelectionDialog( q ) );
  dlg->view()->view()->setSelectionMode( QAbstractItemView::MultiSelection );

  mResentTo = mEditTo->text();

  if ( dlg->exec() != KDialog::Rejected && dlg ) {
    QStringList addresses;
    foreach ( const Akonadi::EmailAddressSelection &selection, dlg->selectedAddresses() )
      addresses << selection.quotedEmail();

    if ( !mResentTo.isEmpty() )
      addresses.prepend( mResentTo );

    mEditTo->setText( addresses.join( ", " ) );
    mEditTo->setModified( true );
  }
}

void RedirectDialog::Private::slotAddressChanged( const QString &text )
{
  q->enableButton( KDialog::User1, !text.isEmpty() );
  q->enableButton( KDialog::User2, !text.isEmpty() );
}


RedirectDialog::RedirectDialog( SendMode mode, QWidget *parent )
  : KDialog( parent ), d( new Private( this, mode ) )
{
  setCaption( i18n( "Redirect Message" ) );
  setButtons( User1 | User2 | Cancel );
  setDefaultButton( mode == SendNow ? User1 : User2 );

  QFrame *vbox = new KVBox( this );
  setMainWidget( vbox );
  d->mLabelTo = new QLabel( i18n( "Select the recipient &addresses "
                                  "to redirect to:" ), vbox );

  KHBox *hbox = new KHBox( vbox );
  hbox->setSpacing( 4 );
  d->mEditTo = new MessageComposer::ComposerLineEdit( true, hbox );
  d->mEditTo->setObjectName( "toLine" );
  d->mEditTo->setRecentAddressConfig( KernelIf->config().data() );
  d->mEditTo->setMinimumWidth( 300 );

  d->mBtnTo = new QPushButton( QString(), hbox );
  d->mBtnTo->setObjectName( "toBtn" );
  d->mBtnTo->setIcon( KIcon( "help-contents" ) );
  d->mBtnTo->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  d->mBtnTo->setMinimumSize( d->mBtnTo->sizeHint() * 1.2 );
  d->mBtnTo->setToolTip( i18n( "Use the Address-Selection Dialog" ) );
  d->mBtnTo->setWhatsThis( i18n( "This button opens a separate dialog "
                                 "where you can select recipients out "
                                 "of all available addresses." ) );

  connect( d->mBtnTo, SIGNAL( clicked() ), SLOT( slotAddressSelection() ) );

  connect( d->mEditTo, SIGNAL( textChanged ( const QString& ) ), SLOT( slotAddressChanged( const QString& ) ) );
  d->mLabelTo->setBuddy( d->mBtnTo );
  d->mEditTo->setFocus();

  setButtonGuiItem( User1, KGuiItem( i18n( "&Send Now" ), "mail-send" ) );
  setButtonGuiItem( User2, KGuiItem( i18n( "Send &Later" ), "mail-queue" ) );
  connect( this, SIGNAL( user1Clicked() ), this, SLOT( slotUser1() ) );
  connect( this, SIGNAL( user2Clicked() ), this, SLOT( slotUser2() ) );
  enableButton( User1, false );
  enableButton( User2, false );
}

RedirectDialog::~RedirectDialog()
{
  delete d;
}

QString RedirectDialog::to() const
{
  return d->mResentTo;
}

RedirectDialog::SendMode RedirectDialog::sendMode() const
{
  return d->mSendMode;
}

void RedirectDialog::accept()
{
  d->mResentTo = d->mEditTo->text();
  if ( d->mResentTo.isEmpty() ) {
    KMessageBox::sorry( this, i18n( "You cannot redirect the message without an address." ),
                              i18n( "Empty Redirection Address" ) );
  } else {
    done( Ok );
  }
}

#include "redirectdialog.moc"
