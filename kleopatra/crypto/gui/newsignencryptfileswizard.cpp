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

#include <view/keytreeview.h>
#include <view/searchbar.h>

#include <models/keycache.h>
#include <models/predicates.h>
#include <models/keylistmodel.h>

#include <utils/stl_util.h>

#include <KLocale>
#include <KIcon>
#include <KDebug>

#include <QLabel>
#include <QWizardPage>
#include <QRadioButton>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QTreeView>
#include <QListWidget>
#include <QLayout>

#include <QVariant>
#include <QPointer>

#include <gpgme++/key.h>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

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

static void really_check( QAbstractButton & b, bool on ) {
    const bool excl = b.autoExclusive();
    b.setAutoExclusive( false );
    b.setChecked( on );
    b.setAutoExclusive( excl );
}

static bool xconnect( const QObject * a, const char * signal,
                      const QObject * b, const char * slot )
{
    return
        QObject::connect( a, signal, b, slot ) &&
        QObject::connect( b, signal, a, slot ) ;
}

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

static void remove_all_cms_keys( KeyTreeView & ktv ) {
    const std::vector<Key> cms
        = kdtools::copy_if< std::vector<Key> >( ktv.keys(), bind( &Key::protocol, _1 ) == CMS );
    ktv.removeKeys( cms );
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
                return "<p>" + i18n("No files selected.") + "</p>";
            QString html = "<p>" + i18np("Selected file:", "Selected files:", m_files.size() ) + "</p>"
                + "<ul><li>" + join_max( m_files, MaxLinesShownInline, "</li><li>" ) + "</li></ul>" ;
            if ( m_files.size() > MaxLinesShownInline )
                html += "<p><a href=\"link:/\">" + i18nc("@action","More...") + "</a></p>";
            return html;
        }
        void updateText() { setText( makeText() ); }

    private:
        QPointer<ListDialog> m_dialog;
        QStringList m_files;
    };


    class WizardPage : public QWizardPage {
        Q_OBJECT
    public:
        explicit WizardPage( QWidget * parent=0 )
            : QWizardPage( parent ),
              m_presetProtocol( UnknownProtocol )
        {

        }

        bool isSigningSelected() const {
            return field("sign").toBool() || isSignEncryptSelected() ;
        }

        bool isEncryptionSelected() const {
            return field("encrypt").toBool() || isSignEncryptSelected() ;
        }

        bool isSignEncryptSelected() const {
            return field("signencrypt").toBool() ;
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
    public:
        explicit OperationPage( QWidget * parent=0 )
            : WizardPage( parent ),
              m_objectsLabel( this ),
              m_signencrypt( i18n("Sign and Encrypt (OpenPGP only)"), this ),
              m_encrypt( i18n("Encrypt"), this ),
              m_sign( i18n("Sign"), this ),
              m_armor( i18n("Text output (ASCII armor)"), this ),
              m_removeSource( i18n("Remove unencrypted original file when done"), this )
        {
            setTitle( i18nc("@title","What do you want to do?") );
            setSubTitle( i18nc("@title",
                               "Please select here whether you want to sign or encrypt files.") );
            KDAB_SET_OBJECT_NAME( m_objectsLabel );
            KDAB_SET_OBJECT_NAME( m_signencrypt );
            KDAB_SET_OBJECT_NAME( m_encrypt );
            KDAB_SET_OBJECT_NAME( m_sign );
            KDAB_SET_OBJECT_NAME( m_armor );
            KDAB_SET_OBJECT_NAME( m_removeSource );

            QVBoxLayout * vlay = new QVBoxLayout( this );
            vlay->addWidget( &m_objectsLabel );
            vlay->addWidget( &m_signencrypt );
            vlay->addWidget( &m_encrypt );
            vlay->addWidget( &m_sign );
            vlay->addStretch( 1 );
            vlay->addWidget( &m_armor );
            vlay->addWidget( &m_removeSource );

            m_armor.setChecked( true );

            registerField( "files", &m_objectsLabel, "files" );

            registerField( "signencrypt", &m_signencrypt );
            registerField( "encrypt", &m_encrypt );
            registerField( "sign", &m_sign );

            registerField( "armor", &m_armor );
            registerField( "remove", &m_removeSource );

            connect( &m_signencrypt, SIGNAL(clicked()), this, SIGNAL(completeChanged()) );
            connect( &m_encrypt,     SIGNAL(clicked()), this, SIGNAL(completeChanged()) );
            connect( &m_sign,        SIGNAL(clicked()), this, SIGNAL(completeChanged()) );

            connect( &m_sign, SIGNAL(toggled(bool)),
                     &m_removeSource, SLOT(setDisabled(bool)) );
        }

        /* reimp */ bool isComplete() const {
            return m_signencrypt.isChecked() || m_encrypt.isChecked() || m_sign.isChecked() ;
        }
        /* reimp */ int nextId() const {
            return isEncryptionSelected() ? RecipientsPageId : SignerPageId ;
        }
        /* reimp */ void doSetPresetProtocol() {
            const bool canSignEncrypt = protocol() != CMS ;
            m_signencrypt.setEnabled( canSignEncrypt );
            m_signencrypt.setToolTip( canSignEncrypt ? QString() :
                                      i18n("This operation is not available for S/MIME") );
            if ( !canSignEncrypt )
                really_check( m_signencrypt, false );
        }

    private:
        ObjectsLabel m_objectsLabel;
        QRadioButton m_signencrypt, m_encrypt, m_sign;
        QCheckBox m_armor, m_removeSource;
    };


    class RecipientsPage : public WizardPage {
        Q_OBJECT
    public:
        explicit RecipientsPage( QWidget * parent=0 )
            : WizardPage( parent ),
              m_keysLoaded( false ),
              m_cmsKeysLoaded( false ),
              m_searchbar( this ),
              m_unselectedKTV( this ),
              m_selectPB( i18n("Add"), this ),
              m_unselectPB( i18n("Remove"), this ),
              m_selectedKTV( this )
        {
            setTitle( i18nc("@title","Whom do you want to encrypt to?") );
            setSubTitle( i18nc("@title",
                               "Please select whom you want the files to be encrypted to. "
                               "Do not forget to pick one of your own certificates.") );
            // the button may be there or not, the _text_ is static, though:
            setButtonText( QWizard::CommitButton, i18nc("@action","Encrypt") );

            KDAB_SET_OBJECT_NAME( m_searchbar );
            KDAB_SET_OBJECT_NAME( m_unselectedKTV );
            KDAB_SET_OBJECT_NAME( m_selectPB );
            KDAB_SET_OBJECT_NAME( m_unselectPB );
            KDAB_SET_OBJECT_NAME( m_selectedKTV );

            m_selectPB.setIcon( KIcon( "list-add" ) );
            m_unselectPB.setIcon( KIcon( "list-remove" ) );

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

        /* reimp */ void initializePage() {

            setCommitPage( !isSigningSelected() );

            const bool signEncrypt = isSignEncryptSelected();
            kDebug() << "m_keysLoaded" << m_keysLoaded
                     << "m_cmsKeysLoaded" << m_cmsKeysLoaded
                     << "signEncrypt" << signEncrypt;
            if ( !m_keysLoaded ) {
                std::vector<Key> keys = KeyCache::instance()->keys();
                kDebug() << "keys.size" << keys.size();
                if ( signEncrypt )
                    // only OpenPGP can do sign+encrypt in one operation
                    _detail::grep_protocol( keys, OpenPGP );
                m_unselectedKTV.setKeys( keys );
                m_keysLoaded = true;
                m_cmsKeysLoaded = !signEncrypt;
            } else if ( m_cmsKeysLoaded && signEncrypt ) {
                remove_all_cms_keys( m_unselectedKTV );
                remove_all_cms_keys( m_selectedKTV );
                m_cmsKeysLoaded = false;
            }
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
        bool m_keysLoaded    : 1;
        bool m_cmsKeysLoaded : 1;

        SearchBar m_searchbar;
        KeyTreeView m_unselectedKTV;
        QPushButton m_selectPB, m_unselectPB;
        KeyTreeView m_selectedKTV;
    };


    static QVector<Protocol> make_allowed_protocols( Protocol proto ) {
        QVector<Protocol> result;
        if ( proto == UnknownProtocol )
            result << OpenPGP << CMS ;
        else
            result << proto;
        return result;
    }

    class SignerPage : public WizardPage {
        Q_OBJECT
    public:
        explicit SignerPage( QWidget * parent=0 )
            : WizardPage( parent ),
              widget( this )
        {
            setTitle( i18nc("@title","Who do you want to sign as?") );
            setSubTitle( i18nc("@title",
                               "Please choose an identity with which to sign the data." ) );
            // this one is always a commit page
            setCommitPage( true );

            QVBoxLayout * vlay = new QVBoxLayout( this );

            KDAB_SET_OBJECT_NAME( widget );
            KDAB_SET_OBJECT_NAME( vlay );

            vlay->setMargin( 0 );
            vlay->addWidget( &widget );

            // ### connect something to completeChanged()
            // ### deal with widget.rememberAsDefault()
        }

        std::vector<Key> keys() const {
            const QMap<Protocol,Key> keys = widget.selectedCertificates();
            std::vector<Key> result;
            if ( effectiveProtocol() == UnknownProtocol )
                result = kdtools::copy< std::vector<Key> >( keys.values() );
            else
                result.push_back( keys[effectiveProtocol()] );
            // remove empty keys, for good measure...
            result.erase( std::remove_if( result.begin(), result.end(), mem_fn( &Key::isNull ) ),
                          result.end() );
            return result;
        }

        /* reimp */ bool isCompleted() const {
            return !keys().empty();
        }

        /* reimp */ int nextId() const {
            return ResultPageId ;
        }

        /* reimp */ void initializePage() {
            if ( isEncryptionSelected() )
                setButtonText( QWizard::CommitButton, i18nc("@action","Sign && Encrypt") );
            else
                setButtonText( QWizard::CommitButton, i18nc("@action","Sign") );
            widget.setAllowedProtocols( make_allowed_protocols( effectiveProtocol() ) );
        }
    private:
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
        if ( signingUserMutable || encryptionUserMutable )
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

    bool signingPreset         : 1;
    bool signingUserMutable    : 1;
    bool encryptionPreset      : 1;
    bool encryptionUserMutable : 1;
};


NewSignEncryptFilesWizard::NewSignEncryptFilesWizard( QWidget * parent, Qt::WindowFlags f )
    : QWizard( parent, f ), d( new Private( this ) )
{

}

NewSignEncryptFilesWizard::~NewSignEncryptFilesWizard() { kDebug(); }

void NewSignEncryptFilesWizard::setPresetProtocol( Protocol proto ) {
    d->operationPage->setPresetProtocol( proto );
    d->recipientsPage->setPresetProtocol( proto );
    d->signerPage->setPresetProtocol( proto );
}

void NewSignEncryptFilesWizard::setSigningPreset( bool preset ) {
    if ( preset == d->signingPreset && preset == isSigningSelected() )
        return;
    d->signingPreset = preset;
    d->updateStartId();
    if ( isEncryptionSelected() )
        setField( "signencrypt", true );
    else
        setField( "sign", true );
}

void NewSignEncryptFilesWizard::setSigningUserMutable( bool mut ) {
    if ( mut == d->signingUserMutable )
        return;
    d->signingUserMutable = mut;
    d->updateStartId();
}
    
void NewSignEncryptFilesWizard::setEncryptionPreset( bool preset ) {
    if ( preset == d->encryptionPreset && preset == isEncryptionSelected() )
        return;
    d->encryptionPreset = preset;
    d->updateStartId();
    if ( isSigningSelected() )
        setField( "signencrypt", true );
    else
        setField( "encrypt", true );
}

void NewSignEncryptFilesWizard::setEncryptionUserMutable( bool mut ) {
    if ( mut == d->encryptionUserMutable )
        return;
    d->encryptionUserMutable = mut;
    d->updateStartId();
}

void NewSignEncryptFilesWizard::setFiles( const QStringList & files ) {
    setField( "files", files );
}


bool NewSignEncryptFilesWizard::isSigningSelected() const {
    return field("sign").toBool() || field("signencrypt").toBool() ;
}

bool NewSignEncryptFilesWizard::isEncryptionSelected() const {
    return field("encrypt").toBool() || field("signencrypt").toBool() ;
}

bool NewSignEncryptFilesWizard::isAsciiArmorEnabled() const {
    return field("armor").toBool();
}

bool NewSignEncryptFilesWizard::isRemoveUnencryptedFilesEnabled() const {
    return isEncryptionSelected() && field("remove").toBool();
}

std::vector<Key> NewSignEncryptFilesWizard::resolvedRecipients() const {
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
