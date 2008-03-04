#include "certificateselectiondialog.h"

#include <view/searchbar.h>
#include <view/tabwidget.h>

#include <models/keylistmodel.h>
#include <models/keycache.h>

#include <commands/refreshkeyscommand.h>

#include <gpgme++/key.h>

#include <KLocale>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KDebug>

#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLayout>
#include <QItemSelectionModel>
#include <QAbstractItemView>
#include <QPointer>

#include <boost/bind.hpp>

#include <algorithm>

using namespace Kleo;
using namespace Kleo::Dialogs;
using namespace boost;
using namespace GpgME;

class CertificateSelectionDialog::Private {
    friend class ::Kleo::Dialogs::CertificateSelectionDialog;
    CertificateSelectionDialog * const q;
public:
    explicit Private( CertificateSelectionDialog * qq );


private:
    void refresh();
    void slotRefreshed();
    void slotCurrentViewChanged( QAbstractItemView * newView );
    void slotSelectionChanged();
    void slotDoubleClicked( const QModelIndex & idx );

private:
    bool acceptable( const std::vector<Key> & keys ) {
        return !keys.empty();
    }
    void filterAllowedKeys( std::vector<Key> & keys );
    void updateLabelText() {
        ui.label.setText( !customLabelText.isEmpty() ? customLabelText :
                          (options & MultiSelection)
                          ? i18n( "Please select one or more of the following certificates:" )
                          : i18n( "Please select one of the following certificates:" ) );
    }

private:
    QPointer<QAbstractItemView> lastView;
    QString customLabelText;
    Options options;

    struct UI {
        QLabel label;
        SearchBar searchBar;
        TabWidget tabWidget;
        QDialogButtonBox buttonBox;
        QVBoxLayout vlay;

        explicit UI( CertificateSelectionDialog * q )
            : label( q ),
              searchBar( q ),
              tabWidget( q ),
              buttonBox( q ),
              vlay( q )
        {
            KDAB_SET_OBJECT_NAME( label );
            KDAB_SET_OBJECT_NAME( searchBar );
            KDAB_SET_OBJECT_NAME( tabWidget );
            KDAB_SET_OBJECT_NAME( buttonBox );
            KDAB_SET_OBJECT_NAME( vlay );

            vlay.addWidget( &label );
            vlay.addWidget( &searchBar );
            vlay.addWidget( &tabWidget, 1 );
            vlay.addWidget( &buttonBox );

            QPushButton * const ok = buttonBox.addButton( QDialogButtonBox::Ok );
            ok->setEnabled( false );
            QPushButton * const cancel = buttonBox.addButton( QDialogButtonBox::Close );
            QPushButton * const refresh = buttonBox.addButton( i18n("&Refresh Certificates"), QDialogButtonBox::ActionRole );

            connect( &buttonBox, SIGNAL(accepted()), q, SLOT(accept()) );
            connect( &buttonBox, SIGNAL(rejected()), q, SLOT(reject()) );
            connect( refresh,    SIGNAL(clicked()),  q, SLOT(refresh()) );
        }
    } ui;
};


CertificateSelectionDialog::Private::Private( CertificateSelectionDialog * qq )
    : q( qq ),
      ui( q )
{
    ui.tabWidget.setFlatModel( AbstractKeyListModel::createFlatKeyListModel() );
    ui.tabWidget.setHierarchicalModel( AbstractKeyListModel::createHierarchicalKeyListModel() );
    ui.tabWidget.connectSearchBar( &ui.searchBar );

    connect( &ui.tabWidget, SIGNAL(currentViewChanged(QAbstractItemView*)),
             q, SLOT(slotCurrentViewChanged(QAbstractItemView*)) );

    updateLabelText();
    q->setWindowTitle( i18n( "Certificate Selection" ) );
}

CertificateSelectionDialog::CertificateSelectionDialog( QWidget * parent, Qt::WindowFlags f )
    : QDialog( parent, f ), d( new Private( this ) )
{
    const KSharedConfig::Ptr config = KSharedConfig::openConfig( "kleopatracertificateselectiondialogrc" );
    d->ui.tabWidget.loadViews( config.data() );
    const KConfigGroup geometry( config, "Geometry" );
    resize( geometry.readEntry( "size", size() ) );
    d->slotRefreshed();
}

CertificateSelectionDialog::~CertificateSelectionDialog() {}

void CertificateSelectionDialog::setCustomLabelText( const QString & txt ) {
    if ( txt == d->customLabelText )
        return;
    d->customLabelText = txt;
    d->updateLabelText();
}

QString CertificateSelectionDialog::customLabelText() const {
    return d->customLabelText;
}

void CertificateSelectionDialog::setOptions( Options options ) {
    if ( d->options == options )
        return;
    d->options = options;

    d->ui.tabWidget.setMultiSelection( options & MultiSelection );

    d->slotRefreshed();
}

CertificateSelectionDialog::Options CertificateSelectionDialog::options() const {
    return d->options;
}

void CertificateSelectionDialog::setStringFilter( const QString & filter ) {
    d->ui.tabWidget.setStringFilter( filter );
}

void CertificateSelectionDialog::setKeyFilter( const shared_ptr<KeyFilter> & filter ) {
    d->ui.tabWidget.setKeyFilter( filter );
}

void CertificateSelectionDialog::selectCertificates( const std::vector<Key> & keys ) {
    const QAbstractItemView * const view = d->ui.tabWidget.currentView();
    if ( !view )
        return;
    const KeyListModelInterface * const model = dynamic_cast<KeyListModelInterface*>( view->model() );
    assert( model );
    QItemSelectionModel * const sm = view->selectionModel();
    assert( sm );

    Q_FOREACH( const QModelIndex & idx, model->indexes( keys ) )
        if ( idx.isValid() )
            sm->select( idx, QItemSelectionModel::Select | QItemSelectionModel::Rows );
}

void CertificateSelectionDialog::selectCertificate( const Key & key ) {
    selectCertificates( std::vector<Key>( 1, key ) );
}

std::vector<Key> CertificateSelectionDialog::selectedCertificates() const {
    const QAbstractItemView * const view = d->ui.tabWidget.currentView();
    if ( !view )
        return std::vector<Key>();
    const KeyListModelInterface * const model = dynamic_cast<KeyListModelInterface*>( view->model() );
    assert( model );
    const QItemSelectionModel * const sm = view->selectionModel();
    assert( sm );
    return model->keys( sm->selectedIndexes() );
}

Key CertificateSelectionDialog::selectedCertificate() const {
    const std::vector<Key> keys = selectedCertificates();
    return keys.empty() ? Key() : keys.front() ;
}

void CertificateSelectionDialog::hideEvent( QHideEvent * e ) {
    KSharedConfig::Ptr config = KSharedConfig::openConfig( "kleopatracertificateselectiondialogrc" );
    d->ui.tabWidget.saveViews( config.data() );
    KConfigGroup geometry( config, "Geometry" );
    geometry.writeEntry( "size", size() );
    QDialog::hideEvent( e );
}

void CertificateSelectionDialog::Private::refresh() {
    Command * const cmd = new RefreshKeysCommand( 0 );
    connect( cmd, SIGNAL(finsihed()), q, SLOT(slotRefreshed()) );
    cmd->start();
}

void CertificateSelectionDialog::Private::slotRefreshed() {
    q->setEnabled( true );
    std::vector<Key> keys = (options & SecretKeys) ? SecretKeyCache::instance()->keys() : PublicKeyCache::instance()->keys() ;
    filterAllowedKeys( keys );
    const std::vector<Key> selected = q->selectedCertificates();
    if ( AbstractKeyListModel * const model = ui.tabWidget.flatModel() )
        model->addKeys( keys );
    if ( AbstractKeyListModel * const model = ui.tabWidget.hierarchicalModel() )
        model->addKeys( keys );
    q->selectCertificates( selected );
}

void CertificateSelectionDialog::Private::filterAllowedKeys( std::vector<Key> & keys ) {
    std::vector<Key>::iterator end = keys.end();

    switch ( options & AnyFormat ) {
    case OpenPGPFormat:
        end = std::remove_if( keys.begin(), end, bind( &Key::protocol, _1 ) != GpgME::OpenPGP );
        break;
    case CMSFormat:
        end = std::remove_if( keys.begin(), end, bind( &Key::protocol, _1 ) != GpgME::CMS );
        break;
    default:
    case AnyFormat:
        ;
    }

    switch ( options & AnyCertificate ) {
    case SignOnly:
        end = std::remove_if( keys.begin(), end, !bind( &Key::canSign, _1 ) );
        break;
    case EncryptOnly:
        end = std::remove_if( keys.begin(), end, !bind( &Key::canEncrypt, _1 ) );
        break;
    default:
    case AnyCertificate:
        ;
    }

    if ( options & SecretKeys )
        end = std::remove_if( keys.begin(), end, !bind( &Key::hasSecret, _1 ) );

    keys.erase( end, keys.end() );
}

void CertificateSelectionDialog::Private::slotCurrentViewChanged( QAbstractItemView * newView ) {
    if ( lastView ) {
        disconnect( lastView, SIGNAL(doubleClicked(QModelIndex)),
                    q, SLOT(slotDoubleClicked(QModelIndex)) );
        assert( lastView->selectionModel() );
        disconnect( lastView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                    q, SLOT(slotSelectionChanged()) );
    }
    lastView = newView;
    if ( newView ) {
        connect( newView, SIGNAL(doubleClicked(QModelIndex)),
                 q, SLOT(slotDoubleClicked(QModelIndex)) );
        assert( newView->selectionModel() );
        connect( newView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                 q, SLOT(slotSelectionChanged()) );
    }
    slotSelectionChanged();
}

void CertificateSelectionDialog::Private::slotSelectionChanged() {
    if ( QPushButton * const pb = ui.buttonBox.button( QDialogButtonBox::Ok ) )
        pb->setEnabled( acceptable( q->selectedCertificates() ) );
}

void CertificateSelectionDialog::Private::slotDoubleClicked( const QModelIndex & idx ) {
    QAbstractItemView * const view = ui.tabWidget.currentView();
    assert( view );
    const KeyListModelInterface * const model = dynamic_cast<KeyListModelInterface*>( view->model() );
    assert( model );
    QItemSelectionModel * const sm = view->selectionModel();
    assert( sm );
    sm->select( idx, QItemSelectionModel::ClearAndSelect );
    QMetaObject::invokeMethod( q, "accept", Qt::QueuedConnection );
}

void CertificateSelectionDialog::accept() {
    if ( d->acceptable( selectedCertificates() ) )
        QDialog::accept();
}

#include "moc_certificateselectiondialog.cpp"
