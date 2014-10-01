/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/signencryptemailconflictdialog.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
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

#include <config-kleopatra.h>

#include "signencryptemailconflictdialog.h"

#include <crypto/sender.h>
#include <crypto/recipient.h>

#include <dialogs/certificateselectiondialog.h>

#include <models/predicates.h>

#include <utils/gui-helper.h>
#include <utils/formatting.h>
#include <utils/kleo_assert.h>
#include <utils/kdsignalblocker.h>

#include <kleo/stl_util.h>

#include <gpgme++/key.h>

#include <kmime/kmime_header_parsing.h>

#include <KLocalizedString>

#include <QLabel>
#include <QComboBox>
#include <QLayout>
#include <QStackedWidget>
#include <QToolButton>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QStylePainter>
#include <QStyle>
#include <QPointer>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include <iterator>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
using namespace Kleo::Dialogs;
using namespace GpgME;
using namespace boost;

Q_DECLARE_METATYPE( GpgME::Key )
Q_DECLARE_METATYPE( GpgME::UserID )

namespace {

    // A QComboBox with an initial text (as known from web browsers)
    //
    // only works with read-only QComboBoxen, doesn't affect sizeHint
    // as it should...
    //
    class ComboBox : public QComboBox {
        Q_OBJECT
        Q_PROPERTY( QString initialText READ initialText WRITE setInitialText )
        Q_PROPERTY( QIcon initialIcon READ initialIcon WRITE setInitialIcon )
    public:
        explicit ComboBox( QWidget * parent=0 )
            : QComboBox( parent ),
              m_initialText(),
              m_initialIcon()
        {

        }

        explicit ComboBox( const QString & initialText, QWidget * parent=0 )
            : QComboBox( parent ),
              m_initialText( initialText ),
              m_initialIcon()
        {

        }

        explicit ComboBox( const QIcon & initialIcon, const QString & initialText, QWidget * parent=0 )
            : QComboBox( parent ),
              m_initialText( initialText ),
              m_initialIcon( initialIcon )
        {

        }

        QString initialText() const { return m_initialText; }
        QIcon initialIcon() const { return m_initialIcon; }

    public Q_SLOTS:
        void setInitialText( const QString & txt ) {
            if ( txt == m_initialText )
                return;
            m_initialText = txt;
            if ( currentIndex() == -1 )
                update();
        }
        void setInitialIcon( const QIcon & icon ) {
            if ( icon.cacheKey() == m_initialIcon.cacheKey() )
                return;
            m_initialIcon = icon;
            if ( currentIndex() == -1 )
                update();
        }

    protected:
        void paintEvent( QPaintEvent * ) {
            QStylePainter p( this );
            p.setPen( palette().color( QPalette::Text ) );
            QStyleOptionComboBox opt;
            initStyleOption( &opt );
            p.drawComplexControl( QStyle::CC_ComboBox, opt );

            if ( currentIndex() == -1 ) {
                opt.currentText = m_initialText;
                opt.currentIcon = m_initialIcon;
            }
            p.drawControl( QStyle::CE_ComboBoxLabel, opt );
        }

    private:
        QString m_initialText;
        QIcon m_initialIcon;
    };

    static QString make_initial_text( const std::vector<Key> & keys ) {
        if ( keys.empty() )
            return i18n("(no matching certificates found)");
        else
            return i18n("Please select a certificate");
    }

    class KeysComboBox : public ComboBox {
        Q_OBJECT
    public:
        explicit KeysComboBox( QWidget * parent=0 )
            : ComboBox( parent ) {}
        explicit KeysComboBox( const QString & initialText, QWidget * parent=0 )
            : ComboBox( initialText, parent ) {}
        explicit KeysComboBox( const std::vector<Key> & keys, QWidget * parent=0 )
            : ComboBox( make_initial_text( keys ), parent ) { setKeys( keys ); }

        void setKeys( const std::vector<Key> & keys ) {
            clear();
            Q_FOREACH( const Key & key, keys )
                addItem( Formatting::formatForComboBox( key ), qVariantFromValue( key ) );
        }

        std::vector<Key> keys() const {
            std::vector<Key> result;
            result.reserve( count() );
            for ( int i = 0, end = count() ; i != end ; ++i )
                result.push_back( qvariant_cast<Key>( itemData(i) ) );
            return result;;
        }

        int findOrAdd( const Key & key ) {
            for ( int i = 0, end = count() ; i != end ; ++i )
                if ( _detail::ByFingerprint<std::equal_to>()( key, qvariant_cast<Key>( itemData(i) ) ) )
                    return i;
            insertItem( 0, Formatting::formatForComboBox( key ), qVariantFromValue( key ) );
            return 0;
        }

        void addAndSelectCertificate( const Key & key ) {
            setCurrentIndex( findOrAdd( key ) );
        }

        Key currentKey() const {
            return qvariant_cast<Key>( itemData( currentIndex() ) );
        }

    };
        
    class Line {
    public:
        static const unsigned int NumColumns = 4;

        Line( const QString & toFrom, const QString & mailbox, const std::vector<Key> & pgp, bool pgpAmbig, const std::vector<Key> & cms, bool cmsAmbig, QWidget * q, QGridLayout & glay )
            : pgpAmbiguous( pgpAmbig ),
              cmsAmbiguous( cmsAmbig ),
              toFromLB( new QLabel( toFrom, q ) ),
              mailboxLB( new QLabel( mailbox, q ) ),
              sbox( new QStackedWidget( q ) ),
              pgpCB( new KeysComboBox( pgp, sbox ) ),
              cmsCB( new KeysComboBox( cms, sbox ) ),
              noProtocolCB( new KeysComboBox( i18n("(please choose between OpenPGP and S/MIME first)"), sbox ) ),
              toolTB( new QToolButton( q ) )
        {
            KDAB_SET_OBJECT_NAME( toFromLB );
            KDAB_SET_OBJECT_NAME( mailboxLB );
            KDAB_SET_OBJECT_NAME( noProtocolCB );
            KDAB_SET_OBJECT_NAME( pgpCB );
            KDAB_SET_OBJECT_NAME( cmsCB );
            KDAB_SET_OBJECT_NAME( sbox );
            KDAB_SET_OBJECT_NAME( toolTB );

            QFont bold;
            bold.setBold( true );
            toFromLB->setFont( bold );

            mailboxLB->setTextFormat( Qt::PlainText );
            toolTB->setText( i18n("...") );

            pgpCB->setEnabled( !pgp.empty() );
            cmsCB->setEnabled( !cms.empty() );
            noProtocolCB->setEnabled( false );

            pgpCB->setKeys( pgp );
            if ( pgpAmbiguous )
                pgpCB->setCurrentIndex( -1 );

            cmsCB->setKeys( cms );
            if ( cmsAmbiguous )
                cmsCB->setCurrentIndex( -1 );

            sbox->addWidget( pgpCB );
            sbox->addWidget( cmsCB );
            sbox->addWidget( noProtocolCB );
            sbox->setCurrentWidget( noProtocolCB );

            const int row = glay.rowCount();
            unsigned int col = 0;
            glay.addWidget( toFromLB,  row, col++ );
            glay.addWidget( mailboxLB, row, col++ );
            glay.addWidget( sbox,      row, col++ );
            glay.addWidget( toolTB,    row, col++ );
            assert( col == NumColumns );

            q->connect( pgpCB, SIGNAL(currentIndexChanged(int)), SLOT(slotCompleteChanged()) );
            q->connect( cmsCB, SIGNAL(currentIndexChanged(int)), SLOT(slotCompleteChanged()) );
            q->connect( toolTB, SIGNAL(clicked()), SLOT(slotCertificateSelectionDialogRequested()) );
        }

        KeysComboBox * comboBox( Protocol proto ) const {
            if ( proto == OpenPGP )
                return pgpCB;
            if ( proto == CMS )
                return cmsCB;
            return 0;
        }

        QString mailboxText() const {
            return mailboxLB->text();
        }

        void addAndSelectCertificate( const Key & key ) const {
            if ( KeysComboBox * const cb = comboBox( key.protocol() ) ) {
                cb->addAndSelectCertificate( key );
                cb->setEnabled( true );
            }
        }

        void showHide( Protocol proto, bool & first, bool showAll, bool op ) const {
            if ( op && ( showAll || wasInitiallyAmbiguous( proto ) ) ) {

                toFromLB->setVisible( first );
                first = false;

                QFont font = mailboxLB->font();
                font.setBold( wasInitiallyAmbiguous( proto ) );
                mailboxLB->setFont( font );

                sbox->setCurrentIndex( proto );

                mailboxLB->show();
                sbox->show();
                toolTB->show();
            } else {
                toFromLB->hide();
                mailboxLB->hide();
                sbox->hide();
                toolTB->hide();
            }
            
        }

        bool wasInitiallyAmbiguous( Protocol proto ) const {
            return proto == OpenPGP && pgpAmbiguous
                || proto == CMS     && cmsAmbiguous ;
        }

        bool isStillAmbiguous( Protocol proto ) const {
            kleo_assert( proto == OpenPGP || proto == CMS );
            const KeysComboBox * const cb = comboBox( proto );
            return cb->currentIndex() == -1 ;
        }

        Key key( Protocol proto ) const {
            kleo_assert( proto == OpenPGP || proto == CMS );
            const KeysComboBox * const cb = comboBox( proto );
            return cb->currentKey();
        }

        const QToolButton * toolButton() const { return toolTB; }

        void kill() {
            delete toFromLB;
            delete mailboxLB;
            delete sbox;
            delete toolTB;
        }

    private:
        bool pgpAmbiguous : 1;
        bool cmsAmbiguous : 1;

        QLabel * toFromLB;
        QLabel * mailboxLB;
        QStackedWidget * sbox;
        KeysComboBox * pgpCB;
        KeysComboBox * cmsCB;
        KeysComboBox * noProtocolCB;
        QToolButton * toolTB;

    };

}

static CertificateSelectionDialog *
create_certificate_selection_dialog( QWidget * parent, Protocol proto ) {
    CertificateSelectionDialog * const dlg = new CertificateSelectionDialog( parent );    
    dlg->setOptions( proto == OpenPGP ? CertificateSelectionDialog::OpenPGPFormat :
                     proto == CMS     ? CertificateSelectionDialog::CMSFormat : CertificateSelectionDialog::AnyFormat );
    return dlg;
}

static CertificateSelectionDialog *
create_encryption_certificate_selection_dialog( QWidget * parent, Protocol proto, const QString & mailbox ) {
    CertificateSelectionDialog * const dlg = create_certificate_selection_dialog( parent, proto );
    dlg->setCustomLabelText( i18n("Please select an encryption certificate for recipient \"%1\"", mailbox ) );
    dlg->setOptions( CertificateSelectionDialog::SingleSelection |
                     CertificateSelectionDialog::EncryptOnly |
                     dlg->options() );
    return dlg;
}

static CertificateSelectionDialog *
create_signing_certificate_selection_dialog( QWidget * parent, Protocol proto, const QString & mailbox ) {
    CertificateSelectionDialog * const dlg = create_certificate_selection_dialog( parent, proto );
    dlg->setCustomLabelText( i18n("Please select a signing certificate for sender \"%1\"", mailbox ) );
    dlg->setOptions( CertificateSelectionDialog::SingleSelection |
                     CertificateSelectionDialog::SignOnly |
                     CertificateSelectionDialog::SecretKeys |
                     dlg->options() );
    return dlg;
}

static QString make_top_label_conflict_text( bool sign, bool enc ) {
    return 
        sign && enc ? i18n("Kleopatra cannot unambiguously determine matching certificates "
                           "for all recipients/senders of the message.\n"
                           "Please select the correct certificates for each recipient:") :
        sign        ? i18n("Kleopatra cannot unambiguously determine matching certificates "
                           "for the sender of the message.\n"
                           "Please select the correct certificates for the sender:") :
        enc         ? i18n("Kleopatra cannot unambiguously determine matching certificates "
                           "for all recipients of the message.\n"
                           "Please select the correct certificates for each recipient:" ) :
        /* else */    (kleo_assert_fail( sign || enc ),QString()) ;
}

static QString make_top_label_quickmode_text( bool sign, bool enc ) {
    return
        enc    ? i18n("Please verify that correct certificates have been selected for each recipient:") :
        sign   ? i18n("Please verify that the correct certificate has been selected for the sender:") :
        /*else*/ (kleo_assert_fail( sign || enc ),QString()) ;
}

class SignEncryptEMailConflictDialog::Private {
    friend class ::Kleo::Crypto::Gui::SignEncryptEMailConflictDialog;
    SignEncryptEMailConflictDialog * const q;
public:
    explicit Private( SignEncryptEMailConflictDialog * qq )
        : q( qq ),
          senders(),
          recipients(),
          sign( true ),
          encrypt( true ),
          presetProtocol( UnknownProtocol ),
          ui( q )
    {

    }

private:
    void updateTopLabelText() {
        ui.conflictTopLB.setText( make_top_label_conflict_text( sign, encrypt ) );
        ui.quickModeTopLB.setText( make_top_label_quickmode_text( sign, encrypt ) );
    }

    void showHideWidgets() {
        const Protocol proto = q->selectedProtocol();
        const bool quickMode = q->isQuickMode();

        const bool needProtocolSelection = presetProtocol == UnknownProtocol ;

        const bool needShowAllRecipientsCB =
            quickMode             ? false :
            needProtocolSelection ? needShowAllRecipients( OpenPGP ) || needShowAllRecipients( CMS ) :
            /* else */              needShowAllRecipients( proto )
            ;

        ui.showAllRecipientsCB.setVisible( needShowAllRecipientsCB );

        ui.pgpRB.setVisible( needProtocolSelection );
        ui.cmsRB.setVisible( needProtocolSelection );

        const bool showAll = !needShowAllRecipientsCB || ui.showAllRecipientsCB.isChecked();

        bool first;
        first = true;
        Q_FOREACH( const Line & line, ui.signers )
            line.showHide( proto, first, showAll, sign );
        ui.selectSigningCertificatesGB.setVisible( sign && ( showAll || !first ) );

        first = true;
        Q_FOREACH( const Line & line, ui.recipients )
            line.showHide( proto, first, showAll, encrypt );
        ui.selectEncryptionCertificatesGB.setVisible( encrypt && ( showAll || !first ) );
    }

    bool needShowAllRecipients( Protocol proto ) const {
        if ( sign )
            if ( const unsigned int num = kdtools::count_if( ui.signers, boost::bind( &Line::wasInitiallyAmbiguous, _1, proto ) ) )
                if ( num != ui.signers.size() )
                    return true;
        if ( encrypt )
            if ( const unsigned int num = kdtools::count_if( ui.recipients, boost::bind( &Line::wasInitiallyAmbiguous, _1, proto ) ) )
                if ( num != ui.recipients.size() )
                    return true;
        return false;
    }

    void createSendersAndRecipients() {
        ui.clearSendersAndRecipients();

        ui.addSelectSigningCertificatesGB();
        Q_FOREACH( const Sender & s, senders )
            addSigner( s );

        ui.addSelectEncryptionCertificatesGB();
        Q_FOREACH( const Sender & s, senders )
            addRecipient( s );
        Q_FOREACH( const Recipient & r, recipients )
            addRecipient( r );
    }

    void addSigner( const Sender & s ) {
        ui.addSigner( s.mailbox().prettyAddress(),
                      s.signingCertificateCandidates( OpenPGP ),
                      s.isSigningAmbiguous( OpenPGP ),
                      s.signingCertificateCandidates( CMS ),
                      s.isSigningAmbiguous( CMS ),
                      q );
    }

    void addRecipient( const Sender & s ) {
        ui.addRecipient( s.mailbox().prettyAddress(),
                         s.encryptToSelfCertificateCandidates( OpenPGP ),
                         s.isEncryptionAmbiguous( OpenPGP ),
                         s.encryptToSelfCertificateCandidates( CMS ),
                         s.isEncryptionAmbiguous( CMS ),
                         q );
    }

    void addRecipient( const Recipient & r ) {
        ui.addRecipient( r.mailbox().prettyAddress(),
                         r.encryptionCertificateCandidates( OpenPGP ),
                         r.isEncryptionAmbiguous( OpenPGP ),
                         r.encryptionCertificateCandidates( CMS ),
                         r.isEncryptionAmbiguous( CMS ),
                         q );
    }

    bool isComplete( Protocol proto ) const;

private:
    void enableDisableOkButton() {
        ui.setOkButtonEnabled( q->isComplete() );
    }
    void slotCompleteChanged() {
        enableDisableOkButton();
    }
    void slotShowAllRecipientsToggled( bool ) {
        showHideWidgets();
    }
    void slotProtocolChanged() {
        showHideWidgets();
        enableDisableOkButton();
    }
    void slotCertificateSelectionDialogRequested() {
        const QObject * const s = q->sender();
        const Protocol proto = q->selectedProtocol();
        QPointer<CertificateSelectionDialog> dlg;
        Q_FOREACH( const Line & l, ui.signers )
            if ( s == l.toolButton() ) {
                dlg = create_signing_certificate_selection_dialog( q, proto, l.mailboxText() );
                if ( dlg->exec() )
                    l.addAndSelectCertificate( dlg->selectedCertificate() );
                // ### switch to key.protocol(), in case proto == UnknownProtocol
                break;
            }
        Q_FOREACH( const Line & l, ui.recipients )
            if ( s == l.toolButton() ) {
                dlg = create_encryption_certificate_selection_dialog( q, proto, l.mailboxText() );
                if ( dlg->exec() )
                    l.addAndSelectCertificate( dlg->selectedCertificate() );
                // ### switch to key.protocol(), in case proto == UnknownProtocol
                break;
            }
        delete dlg;
    }

private:
    std::vector<Sender> senders;
    std::vector<Recipient> recipients;

    bool sign : 1;
    bool encrypt : 1;
    Protocol presetProtocol;

private:
    struct Ui {
        QLabel conflictTopLB, quickModeTopLB;
        QCheckBox showAllRecipientsCB;
        QRadioButton pgpRB, cmsRB;
        QGroupBox selectSigningCertificatesGB;
        QGroupBox selectEncryptionCertificatesGB;
        QCheckBox quickModeCB;
        QDialogButtonBox buttonBox;
        QVBoxLayout vlay;
        QHBoxLayout  hlay;
        QGridLayout  glay;
        std::vector<Line> signers, recipients;

        void setOkButtonEnabled( bool enable ) {
            return buttonBox.button( QDialogButtonBox::Ok )->setEnabled( enable );
        }

        explicit Ui( SignEncryptEMailConflictDialog * q )
            : conflictTopLB( make_top_label_conflict_text( true, true ), q ),
              quickModeTopLB( make_top_label_quickmode_text( true, true ), q ),
              showAllRecipientsCB( i18n("Show all recipients"), q ),
              pgpRB( i18n("OpenPGP"), q ),
              cmsRB( i18n("S/MIME"), q ),
              selectSigningCertificatesGB( i18n("Select Signing Certificate"), q ),
              selectEncryptionCertificatesGB( i18n("Select Encryption Certificate"), q ),
              quickModeCB( i18n("Only show this dialog in case of conflicts (experimental)"), q ),
              buttonBox( QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, q ),
              vlay( q ),
              hlay(),
              glay(),
              signers(),
              recipients()
        {
            KDAB_SET_OBJECT_NAME( conflictTopLB );
            KDAB_SET_OBJECT_NAME( quickModeTopLB );
            KDAB_SET_OBJECT_NAME( showAllRecipientsCB );
            KDAB_SET_OBJECT_NAME( pgpRB );
            KDAB_SET_OBJECT_NAME( cmsRB );
            KDAB_SET_OBJECT_NAME( selectSigningCertificatesGB );
            KDAB_SET_OBJECT_NAME( selectEncryptionCertificatesGB );
            KDAB_SET_OBJECT_NAME( quickModeCB );
            KDAB_SET_OBJECT_NAME( buttonBox );
            KDAB_SET_OBJECT_NAME( hlay );
            KDAB_SET_OBJECT_NAME( glay );
            KDAB_SET_OBJECT_NAME( vlay );

            q->setWindowTitle( i18n("Select Certificates For Message") );

            conflictTopLB.hide();

            selectSigningCertificatesGB.setFlat( true );
            selectEncryptionCertificatesGB.setFlat( true );
            selectSigningCertificatesGB.setAlignment( Qt::AlignCenter );
            selectEncryptionCertificatesGB.setAlignment( Qt::AlignCenter );

            glay.setColumnStretch( 2, 1 );
            glay.setColumnStretch( 3, 1 );

            vlay.setSizeConstraint( QLayout::SetMinimumSize );

            vlay.addWidget( &conflictTopLB );
            vlay.addWidget( &quickModeTopLB );

            hlay.addWidget( &showAllRecipientsCB );
            hlay.addStretch( 1 );
            hlay.addWidget( &pgpRB );
            hlay.addWidget( &cmsRB );
            vlay.addLayout( &hlay );

            addSelectSigningCertificatesGB();
            addSelectEncryptionCertificatesGB();
            vlay.addLayout( &glay );

            vlay.addStretch( 1 );

            vlay.addWidget( &quickModeCB, 0, Qt::AlignCenter );
            vlay.addWidget( &buttonBox );

            connect(&buttonBox, &QDialogButtonBox::accepted, q, &SignEncryptEMailConflictDialog::accept);
            connect(&buttonBox, &QDialogButtonBox::rejected, q, &SignEncryptEMailConflictDialog::reject);

            connect( &showAllRecipientsCB, SIGNAL(toggled(bool)), q, SLOT(slotShowAllRecipientsToggled(bool)) );
            connect( &pgpRB, SIGNAL(toggled(bool)),
                     q, SLOT(slotProtocolChanged()) );
            connect( &cmsRB, SIGNAL(toggled(bool)),
                     q, SLOT(slotProtocolChanged()) );
        }

        void clearSendersAndRecipients() {
            std::vector<Line> sig, enc;
            sig.swap( signers );
            enc.swap( recipients );
            kdtools::for_each( sig, mem_fn( &Line::kill ) );
            kdtools::for_each( enc, mem_fn( &Line::kill ) );
            glay.removeWidget( &selectSigningCertificatesGB );
            glay.removeWidget( &selectEncryptionCertificatesGB );
        }

        void addSelectSigningCertificatesGB() {
            glay.addWidget( &selectSigningCertificatesGB,    glay.rowCount(), 0, 1, Line::NumColumns );
        }
        void addSelectEncryptionCertificatesGB() {
            glay.addWidget( &selectEncryptionCertificatesGB, glay.rowCount(), 0, 1, Line::NumColumns );
        }

        void addSigner( const QString & mailbox,
                        const std::vector<Key> & pgp, bool pgpAmbiguous,
                        const std::vector<Key> & cms, bool cmsAmbiguous, QWidget * q )
        {
            Line line( i18n("From:"), mailbox, pgp, pgpAmbiguous, cms, cmsAmbiguous, q, glay );
            signers.push_back( line );
        }

        void addRecipient( const QString & mailbox,
                           const std::vector<Key> & pgp, bool pgpAmbiguous,
                           const std::vector<Key> & cms, bool cmsAmbiguous, QWidget * q )
        {
            Line line( i18n("To:"), mailbox, pgp, pgpAmbiguous, cms, cmsAmbiguous, q, glay );
            recipients.push_back( line );
        }

    } ui;
};

SignEncryptEMailConflictDialog::SignEncryptEMailConflictDialog( QWidget * parent, Qt::WindowFlags f )
    : QDialog( parent, f ), d( new Private( this ) )
{

}

SignEncryptEMailConflictDialog::~SignEncryptEMailConflictDialog() {}

void SignEncryptEMailConflictDialog::setPresetProtocol( Protocol p ) {
    if ( p == d->presetProtocol )
        return;
    const KDSignalBlocker pgpBlocker( d->ui.pgpRB );
    const KDSignalBlocker cmsBlocker( d->ui.cmsRB );
    really_check( d->ui.pgpRB, p == OpenPGP );
    really_check( d->ui.cmsRB, p == CMS );
    d->presetProtocol = p;
    d->showHideWidgets();
    d->enableDisableOkButton();
}

Protocol SignEncryptEMailConflictDialog::selectedProtocol() const {
    if ( d->presetProtocol != UnknownProtocol )
        return d->presetProtocol;
    if ( d->ui.pgpRB.isChecked() )
        return OpenPGP;
    if ( d->ui.cmsRB.isChecked() )
        return CMS;
    return UnknownProtocol;
}

void SignEncryptEMailConflictDialog::setSubject( const QString & subject ) {
    setWindowTitle( i18n("Select Certificates For Message \"%1\"", subject ) );
}

void SignEncryptEMailConflictDialog::setSign( bool sign ) {
    if ( sign == d->sign )
        return;
    d->sign = sign;
    d->updateTopLabelText();
    d->showHideWidgets();
    d->enableDisableOkButton();
}

void SignEncryptEMailConflictDialog::setEncrypt( bool encrypt ) {
    if ( encrypt == d->encrypt )
        return;
    d->encrypt = encrypt;
    d->updateTopLabelText();
    d->showHideWidgets();
    d->enableDisableOkButton();
}

void SignEncryptEMailConflictDialog::setSenders( const std::vector<Sender> & senders ) {
    if ( senders == d->senders )
        return;
    d->senders = senders;
    d->createSendersAndRecipients();
    d->showHideWidgets();
    d->enableDisableOkButton();
}

void SignEncryptEMailConflictDialog::setRecipients( const std::vector<Recipient> & recipients ) {
    if ( d->recipients == recipients )
        return;
    d->recipients = recipients;
    d->createSendersAndRecipients();
    d->showHideWidgets();
    d->enableDisableOkButton();
}

void SignEncryptEMailConflictDialog::pickProtocol() {

    if ( selectedProtocol() != UnknownProtocol )
        return; // already picked

    const bool pgp = d->isComplete( OpenPGP );
    const bool cms = d->isComplete( CMS );

    if ( pgp && !cms )
        d->ui.pgpRB.setChecked( true );
    else if ( cms && !pgp )
        d->ui.cmsRB.setChecked( true );
}

bool SignEncryptEMailConflictDialog::isComplete() const {
    const Protocol proto = selectedProtocol();
    return proto != UnknownProtocol && d->isComplete( proto ) ;
}

bool SignEncryptEMailConflictDialog::Private::isComplete( Protocol proto ) const {
    return ( !sign    || kdtools::none_of( ui.signers,    boost::bind( &Line::isStillAmbiguous, _1, proto ) ) )
        && ( !encrypt || kdtools::none_of( ui.recipients, boost::bind( &Line::isStillAmbiguous, _1, proto ) ) )
        ;
}

static std::vector<Key> get_keys( const std::vector<Line> & lines, Protocol proto ) {
    if ( proto == UnknownProtocol )
        return std::vector<Key>();
    assert( proto == OpenPGP || proto == CMS );

    std::vector<Key> keys;
    keys.reserve( lines.size() );
    kdtools::transform( lines, std::back_inserter( keys ),
                        boost::bind( &Line::key, _1, proto ) ); 
    kleo_assert( kdtools::none_of( keys, mem_fn( &Key::isNull ) ) );
    return keys;
}

std::vector<Key> SignEncryptEMailConflictDialog::resolvedSigningKeys() const {
    return d->sign    ? get_keys( d->ui.signers,    selectedProtocol() ) : std::vector<Key>() ;
}

std::vector<Key> SignEncryptEMailConflictDialog::resolvedEncryptionKeys() const {
    return d->encrypt ? get_keys( d->ui.recipients, selectedProtocol() ) : std::vector<Key>() ;
}

void SignEncryptEMailConflictDialog::setQuickMode( bool on ) {
    d->ui.quickModeCB.setChecked( on );
}

bool SignEncryptEMailConflictDialog::isQuickMode() const {
    return d->ui.quickModeCB.isChecked();
}

void SignEncryptEMailConflictDialog::setConflict( bool conflict ) {
    d->ui.conflictTopLB.setVisible( conflict );
    d->ui.quickModeTopLB.setVisible( !conflict );
}

#include "moc_signencryptemailconflictdialog.cpp"
#include "signencryptemailconflictdialog.moc"
