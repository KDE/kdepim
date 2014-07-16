/*
  Copyright (c) 2003 Andreas Gungl <a.gungl@gmx.de>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
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
#include "kernel/mailkernel.h"

#include <messagecomposer/composer/composerlineedit.h>

#include <messageviewer/utils/autoqpointer.h>

#include <Akonadi/Contact/EmailAddressSelectionDialog>

#include <KPIMUtils/Email>
#include <KDE/KPIMIdentities/Identity>
#include <KDE/KPIMIdentities/IdentityCombo>
#include <KDE/KPIMIdentities/IdentityManager>

#include <KDE/Mailtransport/Transport>
#include <KDE/Mailtransport/TransportComboBox>
#include <KDE/Mailtransport/TransportManager>


#include <KIconLoader>
#include <KLocale>
#include <KMessageBox>
#include <KVBox>

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QTreeView>

using namespace MailCommon;

class RedirectDialog::Private
{
public:
    Private( RedirectDialog *qq, RedirectDialog::SendMode mode )
        : q( qq ), mEditTo( 0 ), mSendMode( mode ), mComboboxIdentity( 0 ), mTransportCombobox( 0 )
    {
    }

    void slotUser1();
    void slotUser2();
    void slotAddressSelection();
    void slotAddressChanged( const QString & );

    RedirectDialog *q;
    MessageComposer::ComposerLineEdit *mEditTo;

    QString mResentTo;
    RedirectDialog::SendMode mSendMode;
    KPIMIdentities::IdentityCombo *mComboboxIdentity;
    MailTransport::TransportComboBox *mTransportCombobox;
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
    MessageViewer::AutoQPointer<Akonadi::EmailAddressSelectionDialog> dlg(
                new Akonadi::EmailAddressSelectionDialog( q ) );

    dlg->view()->view()->setSelectionMode( QAbstractItemView::MultiSelection );

    mResentTo = mEditTo->text();

    if ( dlg->exec() != KDialog::Rejected && dlg ) {
        QStringList addresses;
        foreach ( const Akonadi::EmailAddressSelection &selection, dlg->selectedAddresses() ) {
            addresses << selection.quotedEmail();
        }

        if ( !mResentTo.isEmpty() ) {
            addresses.prepend( mResentTo );
        }

        mEditTo->setText( addresses.join( QLatin1String(", ") ) );
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
    QLabel *LabelTo = new QLabel( i18n( "Select the recipient &addresses "
                                        "to redirect to:" ), vbox );

    KHBox *hbox = new KHBox( vbox );
    hbox->setSpacing( 4 );
    d->mEditTo = new MessageComposer::ComposerLineEdit( true, hbox );
    d->mEditTo->setRecentAddressConfig( KernelIf->config().data() );
    d->mEditTo->setMinimumWidth( 300 );
    d->mEditTo->setClearButtonShown( true );

    QPushButton *BtnTo = new QPushButton( QString(), hbox );
    BtnTo->setIcon( KIcon( QLatin1String("help-contents") ) );
    BtnTo->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    BtnTo->setMinimumSize( BtnTo->sizeHint() * 1.2 );
    BtnTo->setToolTip( i18n( "Use the Address-Selection Dialog" ) );
    BtnTo->setWhatsThis( i18n( "This button opens a separate dialog "
                               "where you can select recipients out "
                               "of all available addresses." ) );

    connect( BtnTo, SIGNAL(clicked()), SLOT(slotAddressSelection()) );

    connect( d->mEditTo, SIGNAL(textChanged(QString)), SLOT(slotAddressChanged(QString)) );
    LabelTo->setBuddy( BtnTo );
    d->mEditTo->setFocus();

    hbox = new KHBox( vbox );
    QLabel *lab = new QLabel(i18n("Identity:"),hbox);
    d->mComboboxIdentity = new KPIMIdentities::IdentityCombo(KernelIf->identityManager(),hbox);
    lab->setBuddy(d->mComboboxIdentity);

    hbox = new KHBox(vbox);
    lab = new QLabel(i18n("Transport:"),hbox);
    d->mTransportCombobox = new MailTransport::TransportComboBox( hbox );
    lab->setBuddy(d->mTransportCombobox);

    setButtonGuiItem( User1, KGuiItem( i18n( "&Send Now" ), QLatin1String("mail-send") ) );
    setButtonGuiItem( User2, KGuiItem( i18n( "Send &Later" ), QLatin1String("mail-queue") ) );
    connect( this, SIGNAL(user1Clicked()), this, SLOT(slotUser1()) );
    connect( this, SIGNAL(user2Clicked()), this, SLOT(slotUser2()) );
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

int RedirectDialog::transportId() const
{
    return d->mTransportCombobox->currentTransportId();
}

int RedirectDialog::identity() const
{
    return static_cast<int>(d->mComboboxIdentity->currentIdentity());
}

void RedirectDialog::accept()
{
    d->mResentTo = d->mEditTo->text();
    if ( d->mResentTo.isEmpty() ) {
        KMessageBox::sorry(
                    this,
                    i18n( "You cannot redirect the message without an address." ),
                    i18n( "Empty Redirection Address" ) );
    } else {
        done( Ok );
    }
}

#include "moc_redirectdialog.cpp"
