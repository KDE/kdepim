/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/resolverecipientspage.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

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

#include "resolverecipientspage.h"
#include "resolverecipientspage_p.h"

#include <dialogs/certificateselectiondialog.h>

#include <crypto/certificateresolver.h>

#include <models/keycache.h>

#include <utils/formatting.h>

#include <kmime/kmime_header_parsing.h>

#include <gpgme++/key.h>

#include <KLocalizedString>

#include <QButtonGroup>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QToolButton>
#include <QSignalMapper>
#include <QStringList>
#include <QVBoxLayout>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <cassert>

using namespace GpgME;
using namespace boost;
using namespace Kleo;
using namespace Kleo::Dialogs;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
using namespace KMime::Types;

ResolveRecipientsPage::ListWidget::ListWidget( QWidget* parent, Qt::WindowFlags flags ) : QWidget( parent, flags ), m_protocol( UnknownProtocol )
{
    m_listWidget = new QListWidget;
    m_listWidget->setSelectionMode( QAbstractItemView::MultiSelection );
    QVBoxLayout * const layout = new QVBoxLayout( this );
    layout->addWidget( m_listWidget );
    connect( m_listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectionChange()) );
}

ResolveRecipientsPage::ListWidget::~ListWidget()
{
}

void ResolveRecipientsPage::ListWidget::onSelectionChange()
{
    Q_FOREACH ( const QString& i, widgets.keys() ) { //krazy:exclude=foreach
        assert( items.contains( i ) );
        widgets[i]->setSelected( items[i]->isSelected() );
    }
    emit selectionChanged();
}

void ResolveRecipientsPage::ListWidget::addEntry( const Mailbox& mbox )
{
    addEntry( mbox.prettyAddress(), mbox.prettyAddress(), mbox );
}

void ResolveRecipientsPage::ListWidget::addEntry( const QString& id, const QString& name )
{
    addEntry( id, name, Mailbox() );
}

void ResolveRecipientsPage::ListWidget::addEntry( const QString& id, const QString& name, const Mailbox& mbox )
{
    assert( !widgets.contains( id ) && !items.contains( id ) );
    QListWidgetItem* item = new QListWidgetItem;
    item->setData( IdRole, id );
    ItemWidget* wid = new ItemWidget( id, name, mbox, this );
    connect( wid, SIGNAL(changed()), this, SIGNAL(completeChanged()) );
    wid->setProtocol( m_protocol );
    item->setSizeHint( wid->sizeHint() );
    m_listWidget->addItem( item );
    m_listWidget->setItemWidget( item, wid );
    widgets[id] = wid;
    items[id] = item;
}

Mailbox ResolveRecipientsPage::ListWidget::mailbox( const QString& id ) const
{
    return widgets.contains( id ) ? widgets[id]->mailbox() : Mailbox();
}

void ResolveRecipientsPage::ListWidget::setCertificates( const QString& id, const std::vector<Key>& pgp, const std::vector<Key>& cms )
{
    assert( widgets.contains( id ) );
    widgets[id]->setCertificates( pgp, cms );
}

Key ResolveRecipientsPage::ListWidget::selectedCertificate( const QString& id ) const
{
    return widgets.contains( id ) ? widgets[id]->selectedCertificate() : Key();
}


GpgME::Key ResolveRecipientsPage::ListWidget::selectedCertificate( const QString& id, GpgME::Protocol prot ) const
{
    return  widgets.contains( id ) ? widgets[id]->selectedCertificate( prot ) : Key();
}

QStringList ResolveRecipientsPage::ListWidget::identifiers() const
{
    return widgets.keys();
}

void ResolveRecipientsPage::ListWidget::setProtocol( GpgME::Protocol prot )
{
    if ( m_protocol == prot )
        return;
    m_protocol = prot;
    Q_FOREACH ( ItemWidget* i, widgets )
        i->setProtocol( prot );
}

void ResolveRecipientsPage::ListWidget::removeEntry( const QString& id )
{
    if ( !widgets.contains( id ) )
        return;
    delete items[id];
    items.remove( id );
    delete widgets[id];
    widgets.remove( id );
}

void ResolveRecipientsPage::ListWidget::showSelectionDialog( const QString& id )
{
    if ( !widgets.contains( id ) )
        return;
    widgets[id]->showSelectionDialog();
}

QStringList ResolveRecipientsPage::ListWidget::selectedEntries() const
{
    QStringList entries;
    const QList<QListWidgetItem*> items = m_listWidget->selectedItems();
    Q_FOREACH ( const QListWidgetItem* i, items )
    {
        entries.append( i->data( IdRole ).toString() );
    }
    return entries;
}

ResolveRecipientsPage::ItemWidget::ItemWidget( const QString& id, const QString& name, const Mailbox& mbox,
        QWidget* parent, Qt::WindowFlags flags ) : QWidget( parent, flags ), m_id( id ), m_mailbox( mbox ), m_protocol( UnknownProtocol ), m_selected( false )
{
    assert( !m_id.isEmpty() );
    setAutoFillBackground( true );
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setMargin( 0 );
    layout->addSpacing( 15 );
    m_nameLabel = new QLabel;
    m_nameLabel->setText( name );
    layout->addWidget( m_nameLabel );
    layout->addStretch();
    m_certLabel = new QLabel;
    m_certLabel->setText( i18n( "<i>No certificate selected</i>" ) );
    layout->addWidget( m_certLabel );
    m_certCombo = new QComboBox;
    connect( m_certCombo, SIGNAL(currentIndexChanged(int)),
             this, SIGNAL(changed()) );
    layout->addWidget( m_certCombo );
    m_selectButton = new QToolButton;
    m_selectButton->setText( i18n( "..." ) );
    connect( m_selectButton, SIGNAL(clicked()),
             this, SLOT(showSelectionDialog()) );
    layout->addWidget( m_selectButton );
    layout->addSpacing( 15 );
    setCertificates( std::vector<Key>(), std::vector<Key>() );
}

void ResolveRecipientsPage::ItemWidget::updateVisibility()
{
    m_certLabel->setVisible( m_certCombo->count() == 0 );
    m_certCombo->setVisible( m_certCombo->count() > 0 );
}

ResolveRecipientsPage::ItemWidget::~ItemWidget()
{
}

QString ResolveRecipientsPage::ItemWidget::id() const
{
    return m_id;
}

void ResolveRecipientsPage::ItemWidget::setSelected( bool selected )
{
    if ( m_selected == selected )
        return;
    m_selected = selected;
    setBackgroundRole( selected ? QPalette::Highlight : QPalette::Base );
    const QPalette::ColorRole foreground = selected ? QPalette::HighlightedText : QPalette::Text;
    setForegroundRole( foreground );
    m_nameLabel->setForegroundRole( foreground );
    m_certLabel->setForegroundRole( foreground );
}

bool ResolveRecipientsPage::ItemWidget::isSelected() const
{
    return m_selected;
}

static CertificateSelectionDialog::Option protocol2option( GpgME::Protocol proto ) {
    switch ( proto ) {
    case OpenPGP: return CertificateSelectionDialog::OpenPGPFormat;
    case CMS:     return CertificateSelectionDialog::CMSFormat;
    default:      return CertificateSelectionDialog::AnyFormat;
    }
}

static CertificateSelectionDialog * createCertificateSelectionDialog( QWidget* parent, GpgME::Protocol prot ) {
    CertificateSelectionDialog * const dlg = new CertificateSelectionDialog( parent );
    const CertificateSelectionDialog::Options options =
        CertificateSelectionDialog::SingleSelection |
        CertificateSelectionDialog::EncryptOnly |
        CertificateSelectionDialog::MultiSelection |
        protocol2option( prot );
    dlg->setOptions( options );
    return dlg;
}

void ResolveRecipientsPage::ItemWidget::showSelectionDialog()
{
    QPointer<CertificateSelectionDialog> dlg = createCertificateSelectionDialog( this, m_protocol );

    if ( dlg->exec() == QDialog::Accepted && dlg /* still with us? */ ) {
        const GpgME::Key cert = dlg->selectedCertificate();
        if ( !cert.isNull() ) {
            addCertificateToComboBox( cert );
            selectCertificateInComboBox( cert );
        }
    }
    delete dlg;
}

Mailbox ResolveRecipientsPage::ItemWidget::mailbox() const
{
    return m_mailbox;
}

void ResolveRecipientsPage::ItemWidget::selectCertificateInComboBox( const Key& key )
{
    m_certCombo->setCurrentIndex( m_certCombo->findData( QLatin1String(key.keyID()) ) );
}

void ResolveRecipientsPage::ItemWidget::addCertificateToComboBox( const GpgME::Key& key )
{
    m_certCombo->addItem( Formatting::formatForComboBox( key ), QByteArray( key.keyID() ) );
    if ( m_certCombo->count() == 1 )
        m_certCombo->setCurrentIndex( 0 );
    updateVisibility();
}

void ResolveRecipientsPage::ItemWidget::resetCertificates()
{
    std::vector<Key> certs;
    Key selected;
    switch ( m_protocol )
    {
        case OpenPGP:
            certs = m_pgp;
            break;
        case CMS:
            certs = m_cms;
            break;
        case UnknownProtocol:
            certs = m_cms;
            certs.insert( certs.end(), m_pgp.begin(), m_pgp.end() );
    }

    m_certCombo->clear();
    Q_FOREACH ( const Key& i, certs )
        addCertificateToComboBox( i );
    if ( !m_selectedCertificates[m_protocol].isNull() )
        selectCertificateInComboBox( m_selectedCertificates[m_protocol] );
    else if ( m_certCombo->count() > 0 )
        m_certCombo->setCurrentIndex( 0 );
    updateVisibility();
    emit changed();
}

void ResolveRecipientsPage::ItemWidget::setProtocol( Protocol prot )
{
    if ( m_protocol == prot )
        return;
    m_selectedCertificates[m_protocol] = selectedCertificate();
    if ( m_protocol != UnknownProtocol )
        ( m_protocol == OpenPGP ? m_pgp : m_cms ) = certificates();
    m_protocol = prot;
    resetCertificates();
}

void ResolveRecipientsPage::ItemWidget::setCertificates( const std::vector<Key>& pgp, const std::vector<Key>& cms )
{
    m_pgp = pgp;
    m_cms = cms;
    resetCertificates();
}

Key ResolveRecipientsPage::ItemWidget::selectedCertificate() const
{
#ifdef QT_STL
    return KeyCache::instance()->findByKeyIDOrFingerprint( m_certCombo->itemData( m_certCombo->currentIndex(), ListWidget::IdRole ).toString().toStdString() );
#else
    const QString tmpStr = m_certCombo->itemData( m_certCombo->currentIndex(), ListWidget::IdRole ).toString();
    const QByteArray asc = tmpStr.toLatin1();
    std::string tmpstdstring = std::string(asc.constData(), asc.length());
    return KeyCache::instance()->findByKeyIDOrFingerprint( tmpstdstring );
#endif
}


GpgME::Key ResolveRecipientsPage::ItemWidget::selectedCertificate( GpgME::Protocol prot ) const
{
    return prot == m_protocol ? selectedCertificate() : m_selectedCertificates.value( prot );
}

std::vector<Key> ResolveRecipientsPage::ItemWidget::certificates() const
{
    std::vector<Key> certs;
    for ( int i = 0; i < m_certCombo->count(); ++i ) {
#ifdef QT_STL
        certs.push_back( KeyCache::instance()->findByKeyIDOrFingerprint( m_certCombo->itemData( i, ListWidget::IdRole ).toString().toStdString() ) );
#else
        const QString tmpStr = m_certCombo->itemData( i, ListWidget::IdRole ).toString();
        const QByteArray asc = tmpStr.toLatin1();
        std::string tmpstdstring = std::string(asc.constData(), asc.length());
        certs.push_back( KeyCache::instance()->findByKeyIDOrFingerprint( tmpstdstring ) );
#endif
    }
    return certs;
}

class ResolveRecipientsPage::Private {
    friend class ::Kleo::Crypto::Gui::ResolveRecipientsPage;
    ResolveRecipientsPage * const q;
public:
    explicit Private( ResolveRecipientsPage * qq );
    ~Private();

    void setSelectedProtocol( Protocol protocol );
    void selectionChanged();
    void removeSelectedEntries();
    void addRecipient();
    void addRecipient( const Mailbox& mbox );
    void addRecipient( const QString& id, const QString& name );
    void updateProtocolRBVisibility();
    void protocolSelected( int prot );
    void writeSelectedCertificatesToPreferences();
    void completeChangedInternal();

private:
    ListWidget* m_listWidget;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    QRadioButton* m_pgpRB;
    QRadioButton* m_cmsRB;
    QLabel* m_additionalRecipientsLabel;
    Protocol m_presetProtocol;
    Protocol m_selectedProtocol;
    bool m_multipleProtocolsAllowed;
    boost::shared_ptr<RecipientPreferences> m_recipientPreferences;
};

ResolveRecipientsPage::Private::Private( ResolveRecipientsPage * qq )
    : q( qq ), m_presetProtocol( UnknownProtocol ), m_selectedProtocol( m_presetProtocol ), m_multipleProtocolsAllowed( false ), m_recipientPreferences()
{
    connect( q, SIGNAL(completeChanged()), q, SLOT(completeChangedInternal()) );
    q->setTitle( i18n( "<b>Recipients</b>" ) );
    QVBoxLayout* const layout = new QVBoxLayout( q );
    m_listWidget = new ListWidget;
    connect( m_listWidget, SIGNAL(selectionChanged()), q, SLOT(selectionChanged()) );
    connect( m_listWidget, SIGNAL(completeChanged()), q, SIGNAL(completeChanged()) );
    layout->addWidget( m_listWidget );
    m_additionalRecipientsLabel = new QLabel;
    m_additionalRecipientsLabel->setWordWrap( true );
    layout->addWidget( m_additionalRecipientsLabel );
    m_additionalRecipientsLabel->setVisible( false );
    QWidget* buttonWidget = new QWidget;
    QHBoxLayout* buttonLayout = new QHBoxLayout( buttonWidget );
    buttonLayout->setMargin( 0 );
    m_addButton = new QPushButton;
    connect( m_addButton, SIGNAL(clicked()), q, SLOT(addRecipient()) );
    m_addButton->setText( i18n( "Add Recipient..." ) );
    buttonLayout->addWidget( m_addButton );
    m_removeButton = new QPushButton;
    m_removeButton->setEnabled( false );
    m_removeButton->setText( i18n( "Remove Selected" ) );
    connect( m_removeButton, SIGNAL(clicked()),
             q, SLOT(removeSelectedEntries()) );
    buttonLayout->addWidget( m_removeButton );
    buttonLayout->addStretch();
    layout->addWidget( buttonWidget );
    QWidget* protocolWidget = new QWidget;
    QHBoxLayout* protocolLayout = new QHBoxLayout( protocolWidget );
    QButtonGroup* protocolGroup = new QButtonGroup( q );
    connect( protocolGroup, SIGNAL(buttonClicked(int)), q, SLOT(protocolSelected(int)) );
    m_pgpRB = new QRadioButton;
    m_pgpRB->setText( i18n( "OpenPGP" ) );
    protocolGroup->addButton( m_pgpRB, OpenPGP );
    protocolLayout->addWidget( m_pgpRB );
    m_cmsRB = new QRadioButton;
    m_cmsRB->setText( i18n( "S/MIME" ) );
    protocolGroup->addButton( m_cmsRB, CMS );
    protocolLayout->addWidget( m_cmsRB );
    protocolLayout->addStretch();
    layout->addWidget( protocolWidget );
}

ResolveRecipientsPage::Private::~Private() {}

void ResolveRecipientsPage::Private::completeChangedInternal()
{
    const bool isComplete = q->isComplete();
    const std::vector<Key> keys = q->resolvedCertificates();
    const bool haveSecret = std::find_if( keys.begin(), keys.end(), boost::bind( &Key::hasSecret, _1 ) ) != keys.end();
    if ( isComplete && !haveSecret )
        q->setExplanation( i18n( "<b>Warning:</b> None of the selected certificates seem to be your own. You will not be able to decrypt the encrypted data again." ) );
    else
        q->setExplanation( QString() );
}

void ResolveRecipientsPage::Private::updateProtocolRBVisibility()
{
    const bool visible = !m_multipleProtocolsAllowed && m_presetProtocol == UnknownProtocol;
    m_cmsRB->setVisible( visible );
    m_pgpRB->setVisible( visible );
    if ( visible )
    {
        if ( m_selectedProtocol == CMS )
            m_cmsRB->click();
        else
            m_pgpRB->click();
    }
}

bool ResolveRecipientsPage::isComplete() const
{
    const QStringList ids = d->m_listWidget->identifiers();
    if ( ids.isEmpty() )
        return false;

    Q_FOREACH ( const QString& i, ids )
    {
        if ( d->m_listWidget->selectedCertificate( i ).isNull() )
            return false;
    }

    return true;
}

ResolveRecipientsPage::ResolveRecipientsPage( QWidget * parent )
    : WizardPage( parent ), d( new Private( this ) )
{
}

ResolveRecipientsPage::~ResolveRecipientsPage() {}

Protocol ResolveRecipientsPage::selectedProtocol() const
{
    return d->m_selectedProtocol;
}

void ResolveRecipientsPage::Private::setSelectedProtocol( Protocol protocol )
{
    if ( m_selectedProtocol == protocol )
        return;
    m_selectedProtocol = protocol;
    m_listWidget->setProtocol( m_selectedProtocol );
    emit q->selectedProtocolChanged();
}

void ResolveRecipientsPage::Private::protocolSelected( int p )
{
    const Protocol protocol = static_cast<Protocol>( p );
    assert( protocol != UnknownProtocol );
    setSelectedProtocol( protocol );
}

void ResolveRecipientsPage::setPresetProtocol( Protocol prot )
{
    if ( d->m_presetProtocol == prot )
        return;
    d->m_presetProtocol = prot;
    d->setSelectedProtocol( prot );
    if ( prot != UnknownProtocol )
        d->m_multipleProtocolsAllowed = false;
    d->updateProtocolRBVisibility();
}

Protocol ResolveRecipientsPage::presetProtocol() const
{
    return d->m_presetProtocol;
}

bool ResolveRecipientsPage::multipleProtocolsAllowed() const
{
    return d->m_multipleProtocolsAllowed;
}

void ResolveRecipientsPage::setMultipleProtocolsAllowed( bool allowed )
{
    if ( d->m_multipleProtocolsAllowed == allowed )
        return;
    d->m_multipleProtocolsAllowed = allowed;
    if ( d->m_multipleProtocolsAllowed )
    {
        setPresetProtocol( UnknownProtocol );
        d->setSelectedProtocol( UnknownProtocol );
    }
    d->updateProtocolRBVisibility();
}


void ResolveRecipientsPage::Private::addRecipient( const QString& id, const QString& name )
{
    m_listWidget->addEntry( id, name );
}

void ResolveRecipientsPage::Private::addRecipient( const Mailbox& mbox )
{
    m_listWidget->addEntry( mbox );
}

void ResolveRecipientsPage::Private::addRecipient()
{
    QPointer<CertificateSelectionDialog> dlg = createCertificateSelectionDialog( q, q->selectedProtocol() );
    if ( dlg->exec() != QDialog::Accepted || !dlg /*q already deleted*/ )
        return;
    const std::vector<Key> keys = dlg->selectedCertificates();

    int i = 0;
    Q_FOREACH( const Key & key, keys ) {
        const QStringList existing = m_listWidget->identifiers();
        QString rec = i18n( "Recipient" );
        while ( existing.contains( rec ) )
            rec = i18nc( "%1 == number", "Recipient (%1)", ++i );
        addRecipient( rec, rec );
        const std::vector<Key> pgp = key.protocol() == OpenPGP ? std::vector<Key>( 1, key ) : std::vector<Key>();
        const std::vector<Key> cms = key.protocol() == CMS ? std::vector<Key>( 1, key ) : std::vector<Key>();
        m_listWidget->setCertificates( rec, pgp, cms );
    }
    emit q->completeChanged();
}

namespace {

    std::vector<Key> makeSuggestions( const boost::shared_ptr<RecipientPreferences>& prefs, const Mailbox& mb, GpgME::Protocol prot )
    {
        std::vector<Key> suggestions;
        const Key remembered = prefs ? prefs->preferredCertificate( mb, prot ) : Key();
         if ( !remembered.isNull() )
             suggestions.push_back( remembered );
         else
             suggestions = CertificateResolver::resolveRecipient( mb, prot );
         return suggestions;
    }
}

static QString listKeysForInfo( const std::vector<Key> & keys ) {
    QStringList list;
    std::transform( keys.begin(), keys.end(), list.begin(), &Formatting::formatKeyLink );
    return list.join( QLatin1String("<br/>") );
}

void ResolveRecipientsPage::setAdditionalRecipientsInfo( const std::vector<Key> & recipients ) {
    d->m_additionalRecipientsLabel->setVisible( !recipients.empty() );
    if ( recipients.empty() )
        return;
    d->m_additionalRecipientsLabel->setText(
        i18n( "<qt><p>Recipients predefined via GnuPG settings:</p>%1</qt>",
              listKeysForInfo( recipients ) ) );
}

void ResolveRecipientsPage::setRecipients( const std::vector<Mailbox>& recipients, const std::vector<Mailbox> & encryptToSelfRecipients )
{
    uint cmsCount = 0;
    uint pgpCount = 0;
    uint senders = 0;
    Q_FOREACH( const Mailbox & mb, encryptToSelfRecipients ) {
        const QString id = QLatin1String("sender-") + QString::number( ++senders );
        d->m_listWidget->addEntry( id, i18n("Sender"), mb );
        const std::vector<Key> pgp = makeSuggestions( d->m_recipientPreferences, mb, OpenPGP );
        const std::vector<Key> cms = makeSuggestions( d->m_recipientPreferences, mb, CMS );
        pgpCount += !pgp.empty();
        cmsCount += !cms.empty();
        d->m_listWidget->setCertificates( id, pgp, cms );
    }
    Q_FOREACH( const Mailbox& i, recipients )
    {
        //TODO:
        const QString address = i.prettyAddress();
        d->addRecipient( i );
        const std::vector<Key> pgp = makeSuggestions( d->m_recipientPreferences, i, OpenPGP );
        const std::vector<Key> cms = makeSuggestions( d->m_recipientPreferences, i, CMS );
        pgpCount += pgp.empty() ? 0 : 1;
        cmsCount += cms.empty() ? 0 : 1;
        d->m_listWidget->setCertificates( address, pgp, cms );
    }
    if ( d->m_presetProtocol == UnknownProtocol && !d->m_multipleProtocolsAllowed )
        ( cmsCount > pgpCount ? d->m_cmsRB : d->m_pgpRB )->click();
}

std::vector<Key> ResolveRecipientsPage::resolvedCertificates() const
{
    std::vector<Key> certs;
    Q_FOREACH( const QString& i, d->m_listWidget->identifiers() )
    {
        const GpgME::Key cert = d->m_listWidget->selectedCertificate( i );
        if ( !cert.isNull() )
            certs.push_back( cert );
    }
    return certs;
}

void ResolveRecipientsPage::Private::selectionChanged()
{
    m_removeButton->setEnabled( !m_listWidget->selectedEntries().isEmpty() );
}

void ResolveRecipientsPage::Private::removeSelectedEntries()
{
    Q_FOREACH ( const QString& i, m_listWidget->selectedEntries() )
        m_listWidget->removeEntry( i );
    emit q->completeChanged();
}

void ResolveRecipientsPage::setRecipientsUserMutable( bool isMutable )
{
    d->m_addButton->setVisible( isMutable );
    d->m_removeButton->setVisible( isMutable );
}

bool ResolveRecipientsPage::recipientsUserMutable() const
{
    return d->m_addButton->isVisible();
}


boost::shared_ptr<RecipientPreferences> ResolveRecipientsPage::recipientPreferences() const
{
    return d->m_recipientPreferences;
}

void ResolveRecipientsPage::setRecipientPreferences( const boost::shared_ptr<RecipientPreferences>& prefs )
{
    d->m_recipientPreferences = prefs;
}

void ResolveRecipientsPage::Private::writeSelectedCertificatesToPreferences()
{
    if ( !m_recipientPreferences )
        return;

    Q_FOREACH ( const QString& i, m_listWidget->identifiers() )
    {
        const Mailbox mbox = m_listWidget->mailbox( i );
        if ( !mbox.hasAddress() )
            continue;
        const Key pgp = m_listWidget->selectedCertificate( i, OpenPGP );
        if ( !pgp.isNull() )
            m_recipientPreferences->setPreferredCertificate( mbox, OpenPGP, pgp );
        const Key cms = m_listWidget->selectedCertificate( i, CMS );
        if ( !cms.isNull() )
            m_recipientPreferences->setPreferredCertificate( mbox, CMS, cms );
    }
}

void ResolveRecipientsPage::onNext() {
    d->writeSelectedCertificatesToPreferences();
}

#include "moc_resolverecipientspage_p.cpp"
#include "moc_resolverecipientspage.cpp"
