/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/newsignencryptfileswizard.cpp

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

#include "newsignencryptfileswizard.h"

#include "newresultpage.h"
#include "signingcertificateselectionwidget.h"

#include <crypto/certificateresolver.h>

#include <view/keytreeview.h>
#include <view/searchbar.h>

#include <models/keycache.h>
#include <models/predicates.h>
#include <models/keylistmodel.h>

#include <utils/archivedefinition.h>
#include <utils/path-helper.h>
#include <utils/gui-helper.h>

#include <kleo/stl_util.h>
#include <ui/filenamerequester.h>

#include <KLocalizedString>
#include <KIcon>
#include <KDebug>
#include <KMessageBox>

#include <QVBoxLayout>
#include <QLabel>
#include <QWizardPage>
#include <QRadioButton>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QComboBox>
#include <QTreeView>

#include <QPointer>

#include <gpgme++/key.h>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include <KGlobal>
#include <KSharedConfig>

using namespace GpgME;
using namespace boost;
using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;

enum Page {
    OperationPageId,
    RecipientsPageId,
    SignerPageId,
    ResultPageId,

    NumPages
};

static void enable_disable( QAbstractButton & button, const QAbstractItemView * view ) {
    const QItemSelectionModel * ism = view ? view->selectionModel() : 0 ;
    button.setEnabled( ism && ism->hasSelection() );
}

static void copy_selected_from_to( KeyTreeView & from, KeyTreeView & to ) {
    to.addKeysSelected( from.selectedKeys() );
    from.view()->selectionModel()->clearSelection();
}

static void move_selected_from_to( KeyTreeView & from, KeyTreeView & to ) {
    const std::vector<Key> keys = from.selectedKeys();
    from.removeKeys( keys );
    to.addKeysSelected( keys );
}

static void remove_all_keys_not_xyz( KeyTreeView & ktv, Protocol proto ) {
    const std::vector<Key> k
        = kdtools::copy_if< std::vector<Key> >( ktv.keys(), boost::bind( &Key::protocol, _1 ) != proto );
    ktv.removeKeys( k );
}

static QString join_max( const QStringList & sl, const int max, const QString & glue ) {
    return QStringList( sl.mid( 0, max ) ).join( glue );
}


namespace {

    // Simple dialog to show a list of files
    class ListDialog : public QDialog {
        Q_OBJECT
    public:
        explicit ListDialog( const QStringList & files, QWidget * parent=0 )
            : QDialog( parent ),
              listWidget( this ),
              buttonBox( QDialogButtonBox::Close, Qt::Vertical, this ),
              hlay( this )
        {
            setWindowTitle( i18nc("@title:window","Selected Files") );

            KDAB_SET_OBJECT_NAME( listWidget );
            KDAB_SET_OBJECT_NAME( buttonBox );
            KDAB_SET_OBJECT_NAME( hlay );

            listWidget.setSelectionMode( QListWidget::NoSelection );
            listWidget.addItems( files );

            hlay.addWidget( &listWidget );
            hlay.addWidget( &buttonBox );

            connect( &buttonBox, SIGNAL(rejected()), this, SLOT(reject()) );
        }

    private:
        QListWidget listWidget;
        QDialogButtonBox buttonBox;
        QHBoxLayout hlay;
    };

    //
    // This is a simple QLabel subclass, used mainly to have a widget
    // for the 'files' field of the wizard (though that one's unused as of now).
    //
    class ObjectsLabel : public QLabel {
        Q_OBJECT // ### PENDING(marc) deal with lots of files (->listbox)
        Q_PROPERTY( QStringList files READ files WRITE setFiles )
    public:
        static const int MaxLinesShownInline = 5;

        explicit ObjectsLabel( QWidget * parent=0, Qt::WindowFlags f=0 )
            : QLabel( parent, f ), m_dialog(), m_files( dummyFiles() )
        {
            connect( this, SIGNAL(linkActivated(QString)),
                     this, SLOT(slotLinkActivated()) );
            updateText();
            // updateGeometry() doesn't seem to reset the
            // minimumSizeHint(), using max-height dummy text here
            // does... Go figure
            m_files.clear();
        }
        explicit ObjectsLabel( const QStringList & files, QWidget * parent=0, Qt::WindowFlags f=0 )
            : QLabel( parent, f ), m_dialog(), m_files( files )
        {
            connect( this, SIGNAL(linkActivated(QString)),
                     this, SLOT(slotLinkActivated()) );
            updateText();
        }

        QStringList files() const { return m_files; }
        void setFiles( const QStringList & files ) {
            if ( m_files == files )
                return;
            m_files = files;
            updateText();
        }

    private Q_SLOTS:
        void slotLinkActivated() {
            if ( !m_dialog ) {
                m_dialog = new ListDialog( m_files, this );
                m_dialog->setAttribute( Qt::WA_DeleteOnClose );
            }
            if ( m_dialog->isVisible() )
                m_dialog->raise();
            else
                m_dialog->show();
        }

    private:
        static QStringList dummyFiles() {
            QStringList dummy;
            for ( int i = 0 ; i < MaxLinesShownInline+1 ; ++i )
                dummy.push_back( QString::number( i ) );
            return dummy;
        }
        QString makeText() const {
            if ( m_files.empty() )
                return QLatin1String("<p>") + i18n("No files selected.") + QLatin1String("</p>");
            QString html = QLatin1String("<p>") + i18np("Selected file:", "Selected files:", m_files.size() ) + QLatin1String("</p>")
                + QLatin1String("<ul><li>") + join_max( m_files, MaxLinesShownInline, QLatin1String("</li><li>") ) + QLatin1String("</li></ul>") ;
            if ( m_files.size() > MaxLinesShownInline )
                html += QLatin1String("<p><a href=\"link:/\">") + i18nc("@action","More...") + QLatin1String("</a></p>");
            return html;
        }
        void updateText() { setText( makeText() ); }

    private:
        QPointer<ListDialog> m_dialog;
        QStringList m_files;
    };

    //
    // Helper wrapper around Kleo::FileNameRequester that adjusts
    // extensions to an ArchiveDefinition
    //
    class ArchiveFileNameRequester : public Kleo::FileNameRequester {
        Q_OBJECT
    public:
        explicit ArchiveFileNameRequester( Protocol protocol, QWidget * parent=0 )
            : FileNameRequester( QDir::Files, parent ), m_protocol( protocol ), m_archiveDefinition()
        {
            setExistingOnly( false );
        }

    public Q_SLOTS:
        void setArchiveDefinition( const boost::shared_ptr<const Kleo::ArchiveDefinition> & ad ) {
            if ( ad == m_archiveDefinition )
                return;
            const QString oldExt = m_archiveDefinition ? m_archiveDefinition->extensions( m_protocol ).front() : QString() ;
            const QString newExt = ad                  ? ad->extensions( m_protocol ).front()                  : QString() ;
            QString fn = fileName();
            if ( fn.endsWith( oldExt ) )
                fn.chop( oldExt.size() + 1 );
            m_archiveDefinition = ad;
            if ( ad )
                fn += QLatin1Char('.') + newExt;
            FileNameRequester::setFileName( fn );
        }
        void setFileName( const QString & fn ) {
            const QString ext = m_archiveDefinition ? m_archiveDefinition->extensions( m_protocol ).front() : QString() ;
            if ( m_archiveDefinition && !fn.endsWith( ext ) )
                FileNameRequester::setFileName( fn + QLatin1Char('.') + ext );
            else
                FileNameRequester::setFileName( fn );
        }
    private:
        const Protocol m_protocol;
        shared_ptr<const ArchiveDefinition> m_archiveDefinition;
    };



    class WizardPage : public QWizardPage {
        Q_OBJECT
    public:
        explicit WizardPage( QWidget * parent=0 )
            : QWizardPage( parent ),
              m_presetProtocol( UnknownProtocol )
        {

        }

        bool isArchiveRequested() const {
            return field(QLatin1String("archive")).toBool();
        }

        QString archiveName( Protocol p ) const {
            return field( p == OpenPGP ? QLatin1String("archive-name-pgp") : QLatin1String("archive-name-cms" )).toString();
        }

        bool isRemoveUnencryptedFilesEnabled() const {
            return field(QLatin1String("remove")).toBool();
        }

        bool isSignOnlySelected() const {
            return field(QLatin1String("sign")).toBool();
        }

        bool isEncryptOnlySelected() const {
            return field(QLatin1String("encrypt")).toBool();
        }

        bool isSignEncryptSelected() const {
            return field(QLatin1String("signencrypt")).toBool() ;
        }

        bool isSigningSelected() const {
            return isSignOnlySelected() || isSignEncryptSelected() ;
        }

        bool isEncryptionSelected() const {
            return isEncryptOnlySelected() || isSignEncryptSelected() ;
        }

        Protocol protocol() const { return m_presetProtocol; }
        Protocol effectiveProtocol() const {
            if ( isSignEncryptSelected() ) {
                assert( m_presetProtocol == OpenPGP || m_presetProtocol == UnknownProtocol );
                return OpenPGP;
            } else {
                return m_presetProtocol;
            }
        }

        void setPresetProtocol( Protocol proto ) {
            if ( proto == m_presetProtocol )
                return;
            m_presetProtocol = proto;
            doSetPresetProtocol();
        }

    private:
        virtual void doSetPresetProtocol() {}

    private:
        Protocol m_presetProtocol;
    };


    class OperationPage : public WizardPage {
        Q_OBJECT
        Q_PROPERTY( QStringList files READ files WRITE setFiles )
        Q_PROPERTY( bool signingPreset READ isSigningPreset WRITE setSigningPreset )
        Q_PROPERTY( bool signingUserMutable READ isSigningUserMutable WRITE setSigningUserMutable )
        Q_PROPERTY( bool encryptionPreset READ isEncryptionPreset WRITE setEncryptionPreset )
        Q_PROPERTY( bool encryptionUserMutable READ isEncryptionUserMutable WRITE setEncryptionUserMutable )
        Q_PROPERTY( bool archiveUserMutable READ isArchiveUserMutable WRITE setArchiveUserMutable )
    public:
        explicit OperationPage( QWidget * parent=0 )
            : WizardPage( parent ),
              m_objectsLabel( this ),
              m_archiveCB( i18n("Archive files with:"), this ),
              m_archive( this ),
              m_archiveNamePgpLB( i18n("Archive name (OpenPGP):"), this ),
              m_archiveNamePgp( OpenPGP, this ),
              m_archiveNameCmsLB( i18n("Archive name (S/MIME):"), this ),
              m_archiveNameCms( CMS, this ),
              m_signencrypt( i18n("Sign and Encrypt (OpenPGP only)"), this ),
              m_encrypt( i18n("Encrypt"), this ),
              m_sign( i18n("Sign"), this ),
              m_armor( i18n("Text output (ASCII armor)"), this ),
              m_removeSource( i18n("Remove unencrypted original file when done"), this ),
              m_signingUserMutable( true ),
              m_encryptionUserMutable( true ),
              m_archiveUserMutable( true ),
              m_signingPreset( true ),
              m_encryptionPreset( true ),
              m_archiveDefinitions( ArchiveDefinition::getArchiveDefinitions() )
        {
            setTitle( i18nc("@title","What do you want to do?") );
            setSubTitle( i18nc("@title",
                               "Please select here whether you want to sign or encrypt files.") );
            KDAB_SET_OBJECT_NAME( m_objectsLabel );
            KDAB_SET_OBJECT_NAME( m_signencrypt );
            KDAB_SET_OBJECT_NAME( m_encrypt );
            KDAB_SET_OBJECT_NAME( m_sign );
            KDAB_SET_OBJECT_NAME( m_archiveCB );
            KDAB_SET_OBJECT_NAME( m_archive );
            KDAB_SET_OBJECT_NAME( m_archiveNamePgpLB );
            KDAB_SET_OBJECT_NAME( m_archiveNamePgp );
            KDAB_SET_OBJECT_NAME( m_archiveNameCmsLB );
            KDAB_SET_OBJECT_NAME( m_archiveNameCms );
            KDAB_SET_OBJECT_NAME( m_armor );
            KDAB_SET_OBJECT_NAME( m_removeSource );

            QGridLayout * glay = new QGridLayout;
            glay->addWidget( &m_archiveCB,        0, 0 );
            glay->addWidget( &m_archive,          0, 1 );
            glay->addWidget( &m_archiveNamePgpLB, 1, 0 );
            glay->addWidget( &m_archiveNamePgp,   1, 1 );
            glay->addWidget( &m_archiveNameCmsLB, 2, 0 );
            glay->addWidget( &m_archiveNameCms,   2, 1 );

            QVBoxLayout * vlay = new QVBoxLayout( this );
            vlay->addWidget( &m_objectsLabel );
            vlay->addLayout( glay );
            vlay->addWidget( &m_signencrypt );
            vlay->addWidget( &m_encrypt );
            vlay->addWidget( &m_sign );
            vlay->addStretch( 1 );
            vlay->addWidget( &m_armor );
            vlay->addWidget( &m_removeSource );

            m_archiveNamePgpLB.setAlignment( Qt::AlignRight );
            m_archiveNamePgpLB.setBuddy( &m_archiveNamePgp );
            m_archiveNameCmsLB.setAlignment( Qt::AlignRight );
            m_archiveNameCmsLB.setBuddy( &m_archiveNameCms );

            m_armor.setChecked( false );
            m_archive.setEnabled( false );
            m_archiveNamePgpLB.setEnabled( false );
            m_archiveNamePgp.setEnabled( false );
            m_archiveNameCmsLB.setEnabled( false );
            m_archiveNameCms.setEnabled( false );

            Q_FOREACH( const shared_ptr<ArchiveDefinition> & ad, m_archiveDefinitions )
                m_archive.addItem( ad->label(), qVariantFromValue( ad ) );

            registerField( QLatin1String("files"), this, "files" );

            registerField( QLatin1String("signing-preset"), this, "signingPreset" );
            registerField( QLatin1String("encryption-preset"), this, "encryptionPreset" );

            registerField( QLatin1String("signencrypt"), &m_signencrypt );
            registerField( QLatin1String("encrypt"), &m_encrypt );
            registerField( QLatin1String("sign"), &m_sign );

            registerField( QLatin1String("armor"), &m_armor );
            registerField( QLatin1String("remove"), &m_removeSource );

            registerField( QLatin1String("archive"), &m_archiveCB );
            registerField( QLatin1String("archive-id"), &m_archive );
            registerField( QLatin1String("archive-name-pgp"), &m_archiveNamePgp, "fileName" );
            registerField( QLatin1String("archive-name-cms"), &m_archiveNameCms, "fileName" );

            registerField( QLatin1String("signing-user-mutable"), this, "signingUserMutable" );
            registerField( QLatin1String("encryption-user-mutable"), this, "encryptionUserMutable" );
            registerField( QLatin1String("archive-user-mutable"), this, "archiveUserMutable" );

            connect( &m_archive, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(slotArchiveDefinitionChanged()) );

            connect( &m_signencrypt, SIGNAL(clicked()), this, SIGNAL(completeChanged()) );
            connect( &m_encrypt,     SIGNAL(clicked()), this, SIGNAL(completeChanged()) );
            connect( &m_sign,        SIGNAL(clicked()), this, SIGNAL(completeChanged()) );
            connect( &m_archiveCB,   SIGNAL(clicked()), this, SIGNAL(completeChanged()) );
            connect( &m_archiveNamePgp, SIGNAL(fileNameChanged(QString)), this, SIGNAL(completeChanged()) );
            connect( &m_archiveNameCms, SIGNAL(fileNameChanged(QString)), this, SIGNAL(completeChanged()) );

            connect( &m_sign, SIGNAL(toggled(bool)),
                     &m_removeSource, SLOT(setDisabled(bool)) );
            connect( &m_archiveCB, SIGNAL(toggled(bool)),
                     &m_archive, SLOT(setEnabled(bool)) );
            connect( &m_archiveCB, SIGNAL(toggled(bool)),
                     &m_archiveNamePgpLB, SLOT(setEnabled(bool)) );
            connect( &m_archiveCB, SIGNAL(toggled(bool)),
                     &m_archiveNamePgp, SLOT(setEnabled(bool)) );
            connect( &m_archiveCB, SIGNAL(toggled(bool)),
                     &m_archiveNameCmsLB, SLOT(setEnabled(bool)) );
            connect( &m_archiveCB, SIGNAL(toggled(bool)),
                     &m_archiveNameCms, SLOT(setEnabled(bool)) );

            const shared_ptr<ArchiveDefinition> ad = archiveDefinition();
            m_archiveNamePgp.setArchiveDefinition( ad );
            m_archiveNameCms.setArchiveDefinition( ad );
        }

        QStringList files() const { return m_objectsLabel.files(); }
        void setFiles( const QStringList & files ) {
            m_objectsLabel.setFiles( files );
            const QString archiveName =
                files.size() == 1 ? files.front() :
                /* else */          QDir( heuristicBaseDirectory( files ) )
                                           .absoluteFilePath( i18nc("base name of an archive file, e.g. archive.zip or archive.tar.gz", "archive") );
            m_archiveNamePgp.setFileName( archiveName );
            m_archiveNameCms.setFileName( archiveName );
        }

        bool isSigningPreset() const { return m_signingPreset; }
        void setSigningPreset( bool preset ) {
            if ( m_signingPreset == preset )
                return;
            m_signingPreset = preset;
            updateSignEncryptArchiveWidgetStates();
        }

        bool isSigningUserMutable() const { return m_signingUserMutable; }
        void setSigningUserMutable( bool mut ) {
            if ( m_signingUserMutable == mut )
                return;
            m_signingUserMutable = mut;
            updateSignEncryptArchiveWidgetStates();
        }

        bool isEncryptionPreset() const { return m_encryptionPreset; }
        void setEncryptionPreset( bool preset ) {
            if ( m_encryptionPreset == preset )
                return;
            m_encryptionPreset = preset;
            updateSignEncryptArchiveWidgetStates();
        }

        bool isEncryptionUserMutable() const { return m_encryptionUserMutable; }
        void setEncryptionUserMutable( bool mut ) {
            if ( m_encryptionUserMutable == mut )
                return;
            m_encryptionUserMutable = mut;
            updateSignEncryptArchiveWidgetStates();
        }

        bool isArchiveUserMutable() const { return m_archiveUserMutable; }
        void setArchiveUserMutable( bool mut ) {
            if ( m_archiveUserMutable == mut )
                return;
            m_archiveUserMutable = mut;
            updateSignEncryptArchiveWidgetStates();
        }

        /* reimp */ bool isComplete() const {
            return ( !isArchiveRequested() || !archiveName( OpenPGP ).isEmpty() && !archiveName( CMS ).isEmpty() ) // ### make more permissive
                && ( isSigningSelected() || isEncryptionSelected() ) ;
        }

        /* reimp */ bool validatePage() {
            if ( isSignOnlySelected() && isArchiveRequested() )
                return KMessageBox::warningContinueCancel( this,
                                                           i18nc("@info",
                                                                 "<para>Archiving in combination with sign-only currently requires what are known as opaque signatures - "
                                                                 "unlike detached ones, these embed the content in the signature.</para>"
                                                                 "<para>This format is rather unusual. You might want to archive the files separately, "
                                                                 "and then sign the archive as one file with Kleopatra.</para>"
                                                                 "<para>Future versions of Kleopatra are expected to also support detached signatures in this case.</para>" ),                                                    
                                                           i18nc("@title:window", "Unusual Signature Warning"),
                                                           KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                                           QLatin1String("signencryptfileswizard-archive+sign-only-warning") )
                    == KMessageBox::Continue ;
            else
                return true;
        }

        /* reimp */ int nextId() const {
            return isEncryptionSelected() ? RecipientsPageId : SignerPageId ;
        }
        /* reimp */ void doSetPresetProtocol() {
            updateSignEncryptArchiveWidgetStates();
        }

        shared_ptr<ArchiveDefinition> archiveDefinition() const {
            return m_archive.itemData( m_archive.currentIndex() ).value< shared_ptr<ArchiveDefinition> >();
        }

        void setArchiveDefinition( const shared_ptr<ArchiveDefinition> & ad ) {
            m_archive.setCurrentIndex( m_archive.findData( qVariantFromValue( ad ) ) );
        }

        void setArchiveDefinition( const QString & adName ) {
            const std::vector< shared_ptr<ArchiveDefinition> >::const_iterator
                it = kdtools::find_if( m_archiveDefinitions, boost::bind( &ArchiveDefinition::id, _1 ) == adName );
            if ( it != m_archiveDefinitions.end() )
                m_archive.setCurrentIndex( it - m_archiveDefinitions.begin() );
        }

    private Q_SLOTS:
        void slotArchiveDefinitionChanged() {
            const shared_ptr<ArchiveDefinition> ad = archiveDefinition();
            m_archiveNamePgp.setArchiveDefinition( ad );
            m_archiveNameCms.setArchiveDefinition( ad );
        }

    private:
        void updateSignEncryptArchiveWidgetStates() {
            m_archiveCB.setEnabled( m_archiveUserMutable );

            const bool mustEncrypt = m_encryptionPreset && !m_encryptionUserMutable ;
            const bool mustSign    = m_signingPreset    && !m_signingUserMutable    ;

            const bool mayEncrypt  = m_encryptionPreset || m_encryptionUserMutable  ;
            const bool maySign     = m_signingPreset    || m_signingUserMutable     ;

            const bool canSignEncrypt = protocol() != CMS && mayEncrypt && maySign ;
            const bool canSignOnly    = maySign && !mustEncrypt ;
            const bool canEncryptOnly = mayEncrypt && !mustSign ;

            m_signencrypt.setEnabled( canSignEncrypt );
            m_encrypt.setEnabled( canEncryptOnly );
            m_sign.setEnabled( canSignOnly );

            really_check( m_signencrypt, canSignEncrypt &&  m_signingPreset &&  m_encryptionPreset );
            really_check( m_encrypt,     canEncryptOnly && !m_signingPreset &&  m_encryptionPreset );
            really_check( m_sign,        canSignOnly    &&  m_signingPreset && !m_encryptionPreset );

            m_signencrypt.setToolTip( protocol() == CMS
                                      ? i18n("This operation is not available for S/MIME")
                                      : QString() );
        }
    private:
        ObjectsLabel m_objectsLabel;
        QCheckBox m_archiveCB;
        QComboBox m_archive;
        QLabel m_archiveNamePgpLB;
        ArchiveFileNameRequester m_archiveNamePgp;
        QLabel m_archiveNameCmsLB;
        ArchiveFileNameRequester m_archiveNameCms;
        QRadioButton m_signencrypt, m_encrypt, m_sign;
        QCheckBox m_armor, m_removeSource;
        bool m_signingUserMutable, m_encryptionUserMutable, m_archiveUserMutable;
        bool m_signingPreset, m_encryptionPreset;
        const std::vector< shared_ptr<ArchiveDefinition> > m_archiveDefinitions;
    };


    class RecipientsPage : public WizardPage {
        Q_OBJECT
    public:
        explicit RecipientsPage( QWidget * parent=0 )
            : WizardPage( parent ),
              m_lastEffectiveProtocol( static_cast<Protocol>(-1) ), // dummy start
              m_searchbar( this ),
              m_unselectedKTV( this ),
              m_selectPB( i18n("Add"), this ),
              m_unselectPB( i18n("Remove"), this ),
              m_selectedKTV( this )
        {
            setTitle( i18nc("@title","For whom do you want to encrypt?") );
            setSubTitle( i18nc("@title",
                               "Please select for whom you want the files to be encrypted. "
                               "Do not forget to pick one of your own certificates.") );
            // the button may be there or not, the _text_ is static, though:
            setButtonText( QWizard::CommitButton, i18nc("@action","Encrypt") );

            KDAB_SET_OBJECT_NAME( m_searchbar );
            KDAB_SET_OBJECT_NAME( m_unselectedKTV );
            KDAB_SET_OBJECT_NAME( m_selectPB );
            KDAB_SET_OBJECT_NAME( m_unselectPB );
            KDAB_SET_OBJECT_NAME( m_selectedKTV );

            m_selectPB.setIcon( KIcon( QLatin1String("arrow-down") ) );
            m_unselectPB.setIcon( KIcon( QLatin1String("arrow-up") ) );

            m_selectPB.setEnabled( false );
            m_unselectPB.setEnabled( false );

            m_unselectedKTV.setHierarchicalModel( AbstractKeyListModel::createHierarchicalKeyListModel( &m_unselectedKTV ) );
            m_unselectedKTV.setHierarchicalView( true );
            m_selectedKTV.setFlatModel( AbstractKeyListModel::createFlatKeyListModel( &m_selectedKTV ) );
            m_selectedKTV.setHierarchicalView( false );

            QVBoxLayout * vlay = new QVBoxLayout( this );
            vlay->addWidget( &m_searchbar );
            vlay->addWidget( &m_unselectedKTV, 1 );

            QHBoxLayout * hlay = new QHBoxLayout;
            hlay->addStretch( 1 );
            hlay->addWidget( &m_selectPB );
            hlay->addWidget( &m_unselectPB );
            hlay->addStretch( 1 );

            vlay->addLayout( hlay );
            vlay->addWidget( &m_selectedKTV, 1 );

            xconnect( &m_searchbar, SIGNAL(stringFilterChanged(QString)),
                      &m_unselectedKTV, SLOT(setStringFilter(QString)) );
            xconnect( &m_searchbar, SIGNAL(keyFilterChanged(boost::shared_ptr<Kleo::KeyFilter>)),
                      &m_unselectedKTV, SLOT(setKeyFilter(boost::shared_ptr<Kleo::KeyFilter>)) );

            connect( m_unselectedKTV.view()->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                     this, SLOT(slotUnselectedSelectionChanged()) );
            connect( m_selectedKTV.view()->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                     this, SLOT(slotSelectedSelectionChanged()) );

            connect( &m_selectPB, SIGNAL(clicked()), this, SLOT(select()) );
            connect( &m_unselectPB, SIGNAL(clicked()), this, SLOT(unselect()) );
        }

        /* reimp */ bool isComplete() const {
            return !m_selectedKTV.keys().empty();
        }

        /* reimp */ int nextId() const {
            if ( isSigningSelected() )
                return SignerPageId;
            else
                return ResultPageId;
        }

        static bool need_reload( Protocol now, Protocol then ) {
            if ( then == UnknownProtocol )
                return false;
            if ( now == UnknownProtocol )
                return true;
            return now != then;
        }

        static bool need_grep( Protocol now, Protocol then ) {
            return now != UnknownProtocol && then == UnknownProtocol ;
        }

        /* reimp */ void initializePage() {

            setCommitPage( !isSigningSelected() );

            const Protocol currentEffectiveProtocol = effectiveProtocol();

            if ( need_reload( currentEffectiveProtocol, m_lastEffectiveProtocol ) ) {
                std::vector<Key> keys = KeyCache::instance()->keys();
                _detail::grep_can_encrypt( keys );
                if ( currentEffectiveProtocol != UnknownProtocol )
                    _detail::grep_protocol( keys, currentEffectiveProtocol );
                m_unselectedKTV.setKeys( keys );
            } else if ( need_grep( currentEffectiveProtocol, m_lastEffectiveProtocol ) ) {
                remove_all_keys_not_xyz( m_unselectedKTV, currentEffectiveProtocol );
                remove_all_keys_not_xyz( m_selectedKTV,   currentEffectiveProtocol );
            }

            m_lastEffectiveProtocol = currentEffectiveProtocol;
        }

        /* reimp */ bool validatePage() {
            const std::vector<Key> & r = keys();
            if ( _detail::none_of_secret( r ) ) {
                if ( KMessageBox::warningContinueCancel( this,
                                                         i18nc("@info",
                                                               "<para>None of the recipients you are encrypting to seems to be your own.</para>"
                                                               "<para>This means that you will not be able to decrypt the data anymore, once encrypted.</para>"
                                                               "<para>Do you want to continue, or cancel to change the recipient selection?</para>"),
                                                         i18nc("@title:window","Encrypt-To-Self Warning"),
                                                         KStandardGuiItem::cont(),
                                                         KStandardGuiItem::cancel(),
                                                         QLatin1String("warn-encrypt-to-non-self"), KMessageBox::Notify|KMessageBox::Dangerous )
                     == KMessageBox::Cancel )
                    return false;
                else if ( isRemoveUnencryptedFilesEnabled() )
                    if ( KMessageBox::warningContinueCancel( this,
                                                             i18nc("@info",
                                                                   "<para>You have requested the unencrypted data to be removed after encryption.</para>"
                                                                   "<para>Are you really sure you do not need to access the data anymore in decrypted form?</para>"),
                                                             i18nc("@title:window","Encrypt-To-Self Warning"),
                                                             KStandardGuiItem::cont(),
                                                             KStandardGuiItem::cancel(),
                                                             QLatin1String("warn-encrypt-to-non-self-destructive"), KMessageBox::Notify|KMessageBox::Dangerous )
                         == KMessageBox::Cancel )
                        return false;
            }
            return true;
        }

        const std::vector<Key> & keys() const {
            return m_selectedKTV.keys();
        }

    private Q_SLOTS:
        void slotUnselectedSelectionChanged() {
            enable_disable( m_selectPB, m_unselectedKTV.view() );
        }
        void slotSelectedSelectionChanged() {
            enable_disable( m_unselectPB, m_selectedKTV.view() );
        }
        void select() {
            copy_selected_from_to( m_unselectedKTV, m_selectedKTV );
            emit completeChanged();
        }
        void unselect() {
            move_selected_from_to( m_selectedKTV, m_unselectedKTV );
            emit completeChanged();
        }

    private:
        Protocol m_lastEffectiveProtocol;

        SearchBar m_searchbar;
        KeyTreeView m_unselectedKTV;
        QPushButton m_selectPB, m_unselectPB;
        KeyTreeView m_selectedKTV;
    };


    class SignerPage : public WizardPage {
        Q_OBJECT
    public:
        explicit SignerPage( QWidget * parent=0 )
            : WizardPage( parent ),
              signPref(),
              pgpCB( i18n("Sign with OpenPGP"), this ),
              cmsCB( i18n("Sign with S/MIME"), this ),
              widget( this )
        {
            setTitle( i18nc("@title","Who do you want to sign as?") );
            setSubTitle( i18nc("@title",
                               "Please choose an identity with which to sign the data." ) );
            // this one is always a commit page
            setCommitPage( true );

            QVBoxLayout * vlay = new QVBoxLayout( this );

            KDAB_SET_OBJECT_NAME( pgpCB );
            KDAB_SET_OBJECT_NAME( cmsCB );
            KDAB_SET_OBJECT_NAME( widget );
            KDAB_SET_OBJECT_NAME( vlay );

            vlay->addWidget( &pgpCB );
            vlay->addWidget( &cmsCB );

            widget.layout()->setMargin( 0 );
            vlay->addWidget( &widget );

            // ### connect something to completeChanged()
            // ### deal with widget.rememberAsDefault()

            connect( &pgpCB, SIGNAL(toggled(bool)), this, SLOT(slotSignProtocolToggled()) );
            connect( &cmsCB, SIGNAL(toggled(bool)), this, SLOT(slotSignProtocolToggled()) );
        }

        std::vector<Key> keys() const {
            const QMap<Protocol,Key> keys = widget.selectedCertificates();
            const bool pgp = pgpCB.isChecked();
            const bool cms = cmsCB.isChecked();

            std::vector<Key> result;
            result.reserve( pgp + cms );

            if ( pgp )
                result.push_back( keys[OpenPGP] );
            if ( cms )
                result.push_back( keys[CMS] );

            // remove empty keys, for good measure...
            result.erase( std::remove_if( result.begin(), result.end(), mem_fn( &Key::isNull ) ),
                          result.end() );
            return result;
        }

        /* reimp */ bool isComplete() const {
            return !keys().empty();
        }

        /* reimp */ int nextId() const {
            return ResultPageId ;
        }

        /* reimp */ void initializePage() {

            if ( QWizard * wiz = wizard() ) {
                // need to do this here, since wizard() == 0 in the ctor
                const NewSignEncryptFilesWizard *filesWizard = qobject_cast<NewSignEncryptFilesWizard*>( wiz );
                disconnect( filesWizard, SIGNAL(operationPrepared()), this, SLOT(slotCommitSigningPreferences()) );
                connect( filesWizard, SIGNAL(operationPrepared()), this, SLOT(slotCommitSigningPreferences()) );
            }

            bool pgp = effectiveProtocol() == OpenPGP;
            bool cms = effectiveProtocol() == CMS;

            if ( effectiveProtocol() == UnknownProtocol )
                pgp = cms = true;

            assert( pgp || cms );

            if ( isSignOnlySelected() ) {
                // free choice of OpenPGP and/or CMS
                // (except when protocol() requires otherwise):
                pgpCB.setEnabled( pgp );
                cmsCB.setEnabled( cms );
                pgpCB.setChecked( pgp );
                cmsCB.setChecked( cms );

                setButtonText( QWizard::CommitButton, i18nc("@action","Sign") );
            } else {
                // we need signing keys for each of protocols that
                // make up the recipients:

                const std::vector<Key> & recipients = resolvedRecipients();
                if ( _detail::none_of_protocol( recipients, OpenPGP ) )
                    pgp = false;
                if ( _detail::none_of_protocol( recipients, CMS ) )
                    cms = false;

                pgpCB.setEnabled( false );
                cmsCB.setEnabled( false );
                pgpCB.setChecked( pgp );
                cmsCB.setChecked( cms );

                setButtonText( QWizard::CommitButton, i18nc("@action","Sign && Encrypt") );
            }

            if ( !signPref ) {
                signPref.reset( new KConfigBasedSigningPreferences( KSharedConfig::openConfig() ) );
                widget.setSelectedCertificates( signPref->preferredCertificate( OpenPGP ),
                                                signPref->preferredCertificate( CMS ) );
            }
        }

    private:
        const std::vector<Key> & resolvedRecipients() const {
            assert( wizard() );
            assert( qobject_cast<NewSignEncryptFilesWizard*>( wizard() ) == static_cast<NewSignEncryptFilesWizard*>( wizard() ) );
            return static_cast<NewSignEncryptFilesWizard*>( wizard() )->resolvedRecipients();
        }

    private Q_SLOTS:
        void slotSignProtocolToggled() {
            widget.setAllowedProtocols( pgpCB.isChecked(), cmsCB.isChecked() );
            emit completeChanged();
        }

        void slotCommitSigningPreferences() {
            if ( widget.rememberAsDefault() )
                Q_FOREACH( const GpgME::Key & key, keys() )
                    if ( !key.isNull() )
                        signPref->setPreferredCertificate( key.protocol(), key );
        }

    private:
        shared_ptr<SigningPreferences> signPref;
        QCheckBox pgpCB, cmsCB;
        SigningCertificateSelectionWidget widget;
    };


    class ResultPage : public NewResultPage {
        Q_OBJECT
    public:
        explicit ResultPage( QWidget * parent=0 )
            : NewResultPage( parent )
        {
            setTitle( i18nc("@title","Results") );
            setSubTitle( i18nc("@title",
                               "Status and progress of the crypto operations is shown here." ) );
        }

    };

}

class NewSignEncryptFilesWizard::Private {
    friend class ::Kleo::Crypto::Gui::NewSignEncryptFilesWizard;
    NewSignEncryptFilesWizard * const q;
public:
    explicit Private( NewSignEncryptFilesWizard * qq )
        : q( qq ),
          operationPage( new OperationPage( q ) ),
          recipientsPage( new RecipientsPage( q ) ),
          signerPage( new SignerPage( q ) ),
          resultPage( new ResultPage( q ) ),
          createArchivePreset( false ),
          createArchiveUserMutable( true ),
          signingPreset( true ),
          signingUserMutable( true ),
          encryptionPreset( true ),
          encryptionUserMutable( true )
    {
        KDAB_SET_OBJECT_NAME( operationPage );
        KDAB_SET_OBJECT_NAME( recipientsPage );
        KDAB_SET_OBJECT_NAME( signerPage );
        KDAB_SET_OBJECT_NAME( resultPage );

        q->setPage( OperationPageId, operationPage );
        q->setPage( RecipientsPageId, recipientsPage );
        q->setPage( SignerPageId, signerPage );
        q->setPage( ResultPageId, resultPage );

        connect( q, SIGNAL(currentIdChanged(int)), q, SLOT(slotCurrentIdChanged(int)) );
    }

private:
    void slotCurrentIdChanged( int id ) {
        if ( id == ResultPageId )
            emit q->operationPrepared();
    }

private:
    int startId() const {
        if ( !createArchivePreset && !createArchiveUserMutable ) {
            if ( signingPreset && !encryptionPreset && !encryptionUserMutable )
                return SignerPageId;
            if ( encryptionPreset && !signingPreset && !signingUserMutable ||
                 signingPreset && !signingUserMutable && encryptionPreset && !encryptionUserMutable )
                return RecipientsPageId;
        }
        if ( signingUserMutable || encryptionUserMutable || createArchivePreset || createArchiveUserMutable )
            return OperationPageId;
        else
            if ( encryptionPreset )
                return RecipientsPageId;
            else
                return SignerPageId;
    }
    void updateStartId() { q->setStartId( startId() ); }

private:
    OperationPage  * operationPage;
    RecipientsPage * recipientsPage;
    SignerPage     * signerPage;
    ResultPage     * resultPage;

    bool createArchivePreset      : 1;
    bool createArchiveUserMutable : 1;
    bool signingPreset         : 1;
    bool signingUserMutable    : 1;
    bool encryptionPreset      : 1;
    bool encryptionUserMutable : 1;
};


NewSignEncryptFilesWizard::NewSignEncryptFilesWizard( QWidget * parent, Qt::WindowFlags f )
    : QWizard( parent, f ), d( new Private( this ) )
{

}

NewSignEncryptFilesWizard::~NewSignEncryptFilesWizard() { qDebug(); }

void NewSignEncryptFilesWizard::setPresetProtocol( Protocol proto ) {
    d->operationPage->setPresetProtocol( proto );
    d->recipientsPage->setPresetProtocol( proto );
    d->signerPage->setPresetProtocol( proto );
}

void NewSignEncryptFilesWizard::setCreateArchivePreset( bool preset ) {
    if ( preset == d->createArchivePreset && preset == isCreateArchiveSelected() )
        return;
    d->createArchivePreset = preset;
    setField( QLatin1String("archive"), preset );
    d->updateStartId();
}

void NewSignEncryptFilesWizard::setCreateArchiveUserMutable( bool mut ) {
    if ( mut == d->createArchiveUserMutable )
        return;
    d->createArchiveUserMutable = mut;
    setField( QLatin1String("archive-user-mutable"), mut );
    d->updateStartId();
}

void NewSignEncryptFilesWizard::setArchiveDefinitionId( const QString & id ) {
    d->operationPage->setArchiveDefinition( id );
}

void NewSignEncryptFilesWizard::setSigningPreset( bool preset ) {
    if ( preset == d->signingPreset )
        return;
    d->signingPreset = preset;
    setField( QLatin1String("signing-preset"), preset );
    d->updateStartId();
}

void NewSignEncryptFilesWizard::setSigningUserMutable( bool mut ) {
    if ( mut == d->signingUserMutable )
        return;
    d->signingUserMutable = mut;
    setField( QLatin1String("signing-user-mutable"), mut );
    d->updateStartId();
}
    
void NewSignEncryptFilesWizard::setEncryptionPreset( bool preset ) {
    if ( preset == d->encryptionPreset )
        return;
    d->encryptionPreset = preset;
    setField( QLatin1String("encryption-preset"), preset );
    d->updateStartId();
}

void NewSignEncryptFilesWizard::setEncryptionUserMutable( bool mut ) {
    if ( mut == d->encryptionUserMutable )
        return;
    d->encryptionUserMutable = mut;
    setField( QLatin1String("encryption-user-mutable"), mut );
    d->updateStartId();
}

void NewSignEncryptFilesWizard::setFiles( const QStringList & files ) {
    setField( QLatin1String("files"), files );
}


bool NewSignEncryptFilesWizard::isSigningSelected() const {
    return field(QLatin1String("sign")).toBool() || field(QLatin1String("signencrypt")).toBool() ;
}

bool NewSignEncryptFilesWizard::isEncryptionSelected() const {
    return field(QLatin1String("encrypt")).toBool() || field(QLatin1String("signencrypt")).toBool() ;
}

bool NewSignEncryptFilesWizard::isAsciiArmorEnabled() const {
    return field(QLatin1String("armor")).toBool();
}

bool NewSignEncryptFilesWizard::isRemoveUnencryptedFilesEnabled() const {
    return isEncryptionSelected() && field(QLatin1String("remove")).toBool();
}

bool NewSignEncryptFilesWizard::isCreateArchiveSelected() const {
    return field(QLatin1String("archive")).toBool();
}

shared_ptr<ArchiveDefinition> NewSignEncryptFilesWizard::selectedArchiveDefinition() const {
    return d->operationPage->archiveDefinition();
}

QString NewSignEncryptFilesWizard::archiveFileName( Protocol p ) const {
    return d->/*any page*/operationPage->archiveName( p );
}

const std::vector<Key> & NewSignEncryptFilesWizard::resolvedRecipients() const {
    return d->recipientsPage->keys();
}

std::vector<Key> NewSignEncryptFilesWizard::resolvedSigners() const {
    return d->signerPage->keys();
}
    

void NewSignEncryptFilesWizard::setTaskCollection( const shared_ptr<TaskCollection> & coll ) {
    d->resultPage->setTaskCollection( coll );
}

#include "moc_newsignencryptfileswizard.cpp"
#include "newsignencryptfileswizard.moc"
