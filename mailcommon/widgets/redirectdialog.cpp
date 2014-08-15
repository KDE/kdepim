/*
  Copyright (c) 2003 Andreas Gungl <a.gungl@gmx.de>
  Copyright (c) 2014 Laurent Montel <montel@kde.org>

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
#include <KPIMIdentities/Identity>
#include <KPIMIdentities/IdentityCombo>
#include <KPIMIdentities/IdentityManager>

#include <MailTransport/Transport>
#include <MailTransport/TransportComboBox>
#include <MailTransport/TransportManager>


#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <QIcon>

#include <QLabel>
#include <QPushButton>
#include <QTreeView>
#include <QHBoxLayout>
#include <QFormLayout>

using namespace MailCommon;

RedirectWidget::RedirectWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setSpacing( 0 );
    hbox->setMargin( 0 );
    hbox->setAlignment(Qt::AlignRight);
    setLayout(hbox);

    mEdit = new MessageComposer::ComposerLineEdit( true );
    mEdit->setRecentAddressConfig( KernelIf->config().data() );
    mEdit->setMinimumWidth( 300 );
    mEdit->setClearButtonEnabled( true );
    hbox->addWidget(mEdit);

    QPushButton *BtnTo = new QPushButton( QString() );
    BtnTo->setIcon( QIcon::fromTheme( QLatin1String("help-contents") ) );
    BtnTo->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    BtnTo->setMinimumSize( BtnTo->sizeHint() * 1.2 );
    BtnTo->setToolTip( i18n( "Use the Address-Selection Dialog" ) );
    BtnTo->setWhatsThis( i18n( "This button opens a separate dialog "
                               "where you can select recipients out "
                               "of all available addresses." ) );
    hbox->addWidget(BtnTo);
    connect( BtnTo, SIGNAL(clicked()), SLOT(slotAddressSelection()) );

    connect( mEdit, SIGNAL(textChanged(QString)), SIGNAL(addressChanged(QString)) );
}

RedirectWidget::~RedirectWidget()
{

}

QString RedirectWidget::resend()
{
    mResendStr = mEdit->text();
    return mResendStr;
}

void RedirectWidget::setFocus()
{
    mEdit->setFocus();
}

void RedirectWidget::slotAddressSelection()
{
    MessageViewer::AutoQPointer<Akonadi::EmailAddressSelectionDialog> dlg(
                new Akonadi::EmailAddressSelectionDialog( this ) );

    dlg->view()->view()->setSelectionMode( QAbstractItemView::MultiSelection );

    mResendStr = mEdit->text();

    if ( dlg->exec() != KDialog::Rejected && dlg ) {
        QStringList addresses;
        foreach ( const Akonadi::EmailAddressSelection &selection, dlg->selectedAddresses() ) {
            addresses << selection.quotedEmail();
        }

        if ( !mResendStr.isEmpty() ) {
            addresses.prepend( mResendStr );
        }

        mEdit->setText( addresses.join( QLatin1String(", ") ) );
        mEdit->setModified( true );
    }
}


class RedirectDialog::Private
{
public:
    Private( RedirectDialog *qq, RedirectDialog::SendMode mode )
        : q( qq ),
          mEditTo( 0 ),
          mEditCc( 0 ),
          mEditBcc( 0 ),
          mSendMode( mode ),
          mComboboxIdentity( 0 ),
          mTransportCombobox( 0 )
    {
    }
    enum TypeAddress {
        ResendTo,
        ResendCc,
        ResendBcc
    };

    void slotUser1();
    void slotUser2();
    void slotAddressChanged( const QString & );
    QString redirectLabelType(TypeAddress type) const;
    RedirectDialog *q;
    RedirectWidget *mEditTo;
    RedirectWidget *mEditCc;
    RedirectWidget *mEditBcc;

    RedirectDialog::SendMode mSendMode;
    KPIMIdentities::IdentityCombo *mComboboxIdentity;
    MailTransport::TransportComboBox *mTransportCombobox;
};

QString RedirectDialog::Private::redirectLabelType(TypeAddress type) const
{
    QString label;
    switch(type) {
    case ResendTo:
        label = i18n("Resend-To:");
        break;
    case ResendCc:
        label = i18n("Resend-Cc:");
        break;
    case ResendBcc:
        label = i18n("Resend-Bcc:");
        break;
    }
    return label;
}

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

void RedirectDialog::Private::slotAddressChanged( const QString &text )
{
    const bool textIsNotEmpty(!text.isEmpty());
    q->enableButton( KDialog::User1, textIsNotEmpty );
    q->enableButton( KDialog::User2, textIsNotEmpty );
}

RedirectDialog::RedirectDialog( SendMode mode, QWidget *parent )
    : KDialog( parent ), d( new Private( this, mode ) )
{
    setCaption( i18n( "Redirect Message" ) );
    setButtons( User1 | User2 | Cancel );
    setDefaultButton( mode == SendNow ? User1 : User2 );

    QWidget *mainWidget = new QWidget;
    setMainWidget(mainWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainWidget->setLayout(mainLayout);
    mainLayout->setSpacing(0);
    QLabel *LabelTo = new QLabel( i18n( "Select the recipient addresses to redirect:" ));
    mainLayout->addWidget(LabelTo);

    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    d->mEditTo = new RedirectWidget;
    mainLayout->addWidget(d->mEditTo);
    formLayout->addRow(d->redirectLabelType(RedirectDialog::Private::ResendTo), d->mEditTo);

    connect( d->mEditTo, SIGNAL(addressChanged(QString)), SLOT(slotAddressChanged(QString)) );

    d->mEditCc = new RedirectWidget;
    formLayout->addRow(d->redirectLabelType(RedirectDialog::Private::ResendCc), d->mEditCc);
    d->mEditBcc = new RedirectWidget;
    formLayout->addRow(d->redirectLabelType(RedirectDialog::Private::ResendBcc), d->mEditBcc);
    d->mEditTo->setFocus();

    QHBoxLayout *hbox = new QHBoxLayout;
    mainLayout->addLayout(hbox);
    QLabel *lab = new QLabel(i18n("Identity:"));
    hbox->addWidget(lab);
    d->mComboboxIdentity = new KPIMIdentities::IdentityCombo(KernelIf->identityManager());
    hbox->addWidget(d->mComboboxIdentity);
    lab->setBuddy(d->mComboboxIdentity);

    hbox = new QHBoxLayout;
    mainLayout->addLayout(hbox);
    lab = new QLabel(i18n("Transport:"));
    hbox->addWidget(lab);
    d->mTransportCombobox = new MailTransport::TransportComboBox;
    hbox->addWidget(d->mTransportCombobox);
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
    return d->mEditTo->resend();
}

QString RedirectDialog::cc() const
{
    return d->mEditCc->resend();
}

QString RedirectDialog::bcc() const
{
    return d->mEditBcc->resend();
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
    const QString editTo = d->mEditTo->resend();
    if ( editTo.isEmpty() ) {
        KMessageBox::sorry(
                    this,
                    i18n( "You cannot redirect the message without an address." ),
                    i18n( "Empty Redirection Address" ) );
    } else {
        done( Ok );
    }
}

#include "moc_redirectdialog.cpp"
