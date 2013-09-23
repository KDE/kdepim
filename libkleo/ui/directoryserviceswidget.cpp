/*
    directoryserviceswidget.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klar�vdalens Datakonsult AB

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

#include "directoryserviceswidget.h"

#include "ui_directoryserviceswidget.h"

#include <KIcon>
#include <KDebug>

#include <QItemDelegate>
#include <QAbstractTableModel>
#include <QSpinBox>
#include <QComboBox>
#include <QHeaderView>
#include <QMenu>
#include <QAction>

#include <boost/bind.hpp>

#include <vector>

#include <climits>
#include <cassert>
#include <algorithm>
#include <functional>

using namespace Kleo;
using namespace boost;

namespace {

    static KUrl defaultX509Service() {
        KUrl url;
        url.setScheme( QLatin1String("ldap") );
        url.setHost( i18nc("default server name, keep it a valid domain name, ie. no spaces", "server") );
        return url;
    }
    static KUrl defaultOpenPGPService() {
        KUrl url;
        url.setScheme( QLatin1String("hkp") );
        url.setHost( QLatin1String("keys.gnupg.net") );
        return url;
    }

    static bool is_ldap_scheme( const KUrl & url ) {
        const QString scheme = url.protocol();
        return QString::compare( scheme, QLatin1String( "ldap" ),  Qt::CaseInsensitive ) == 0
            || QString::compare( scheme, QLatin1String( "ldaps" ), Qt::CaseInsensitive ) == 0;
    }

    static const struct {
        const char label[6];
        unsigned short port;
        DirectoryServicesWidget::Scheme base;
    } protocols[] = {
        { I18N_NOOP("hkp"), 11371, DirectoryServicesWidget::HKP  },
        { I18N_NOOP("http"),   80, DirectoryServicesWidget::HTTP },
        { I18N_NOOP("https"), 443, DirectoryServicesWidget::HTTP },
        { I18N_NOOP("ftp"),    21, DirectoryServicesWidget::FTP  },
        { I18N_NOOP("ftps"),  990, DirectoryServicesWidget::FTP  },
        { I18N_NOOP("ldap"),  389, DirectoryServicesWidget::LDAP },
        { I18N_NOOP("ldaps"), 636, DirectoryServicesWidget::LDAP },
    };
    static const unsigned int numProtocols = sizeof protocols / sizeof *protocols;

    static unsigned short default_port( const QString & scheme ) {
        for ( unsigned int i = 0 ; i < numProtocols ; ++i )
            if ( QString::compare( scheme, QLatin1String( protocols[i].label ), Qt::CaseInsensitive ) == 0 )
                return protocols[i].port;
        return 0;
    }

    static QString display_scheme( const KUrl & url ) {
        if ( url.scheme().isEmpty() )
            return QLatin1String( "hkp" );
        else
            return url.scheme();
    }

    static QString display_host( const KUrl & url ) {
        // work around "subkeys.pgp.net" being interpreted as a path, not host
        if ( url.host().isEmpty() )
            return url.path();
        else
            return url.host();
    }

    static unsigned short display_port( const KUrl & url ) {
        if ( url.port() > 0 )
            return url.port();
        else
            return default_port( display_scheme( url ) );
    }

    static bool is_default_port( const KUrl & url ) {
        return display_port( url ) == default_port( display_scheme( url ) ) ;
    }

    static QRect calculate_geometry( const QRect & cell, const QSize & sizeHint ) {
        const int height = qMax( cell.height(), sizeHint.height() );
        return QRect( cell.left(), cell.top() - ( height - cell.height() ) / 2,
                      cell.width(), height );
    }

    struct KUrl_compare : std::binary_function<KUrl,KUrl,bool> {
        bool operator()( const KUrl & lhs, const KUrl & rhs ) const {
            return QString::compare( display_scheme( lhs ), display_scheme( rhs ), Qt::CaseInsensitive ) == 0
                && QString::compare( display_host( lhs ), display_host( rhs ), Qt::CaseInsensitive ) == 0
                && lhs.port() == rhs.port()
                && lhs.user() == rhs.user()
                // ... ignore password...
                && ( !is_ldap_scheme( lhs )
                     || KUrl::fromPercentEncoding( lhs.query().mid( 1 ).toLatin1() )
                     == KUrl::fromPercentEncoding( rhs.query().mid( 1 ).toLatin1() ) ) ;
        }
    };

    class Model : public QAbstractTableModel {
        Q_OBJECT
    public:
        explicit Model( QObject * parent=0 )
            : QAbstractTableModel( parent ),
              m_items(),
              m_openPGPReadOnly( false ),
              m_x509ReadOnly( false ),
              m_schemes( DirectoryServicesWidget::AllSchemes )
        {

        }

        void setOpenPGPReadOnly( bool ro ) {
            if ( ro == m_openPGPReadOnly )
                return;
            m_openPGPReadOnly = ro;
            for ( unsigned int row = 0, end = rowCount() ; row != end ; ++row )
                if ( isOpenPGPService( row ) )
                    emit dataChanged( index( row, 0 ), index( row, NumColumns ) );
        }

        void setX509ReadOnly( bool ro ) {
            if ( ro == m_x509ReadOnly )
                return;
            m_x509ReadOnly = ro;
            for ( unsigned int row = 0, end = rowCount() ; row != end ; ++row )
                if ( isX509Service( row ) )
                    emit dataChanged( index( row, 0 ), index( row, NumColumns ) );
        }

        QModelIndex addOpenPGPService( const KUrl & url, bool force=false ) {
            return addService( url, false, true, force );
        }
        QModelIndex addX509Service( const KUrl & url, bool force=false ) {
            return addService( url, true, false, force );
        }
        QModelIndex addService( const KUrl & url, bool x509, bool pgp, bool force ) {
            const std::vector<Item>::iterator it = force ? m_items.end() : findExistingUrl( url ) ;
            unsigned int row;
            if ( it != m_items.end() ) {
                // existing item:
                it->x509 |= x509;
                it->pgp  |= pgp;
                row = it - m_items.begin() ;
                emit dataChanged( index( row, std::min( X509, OpenPGP ) ), index( row, std::max( X509, OpenPGP ) ) );
            } else {
                // append new item
                const Item item = { url, x509, pgp };
                row = m_items.size();
                beginInsertRows( QModelIndex(), row, row );
                m_items.push_back( item );
                endInsertRows();
            }
            return index( row, firstEditableColumn( row ) );
        }

        unsigned int numServices() const { return m_items.size(); }
        bool isOpenPGPService( unsigned int row ) const { return row < m_items.size() && m_items[row].pgp;  }
        bool    isX509Service( unsigned int row ) const { return row < m_items.size() && m_items[row].x509 && isLdapRow( row ) ; }
        KUrl          service( unsigned int row ) const { return row < m_items.size() ?  m_items[row].url : KUrl() ; }

        bool isReadOnlyRow( unsigned int row ) const {
            return ( isX509Service( row ) && m_x509ReadOnly )
                || ( isOpenPGPService( row ) && m_openPGPReadOnly );
        }

        enum Columns {
            Scheme,
            Host,
            Port,
            BaseDN,
            UserName,
            Password,
            X509,
            OpenPGP,

            NumColumns
        };

        QModelIndex duplicateRow( unsigned int row ) {
            if ( row >= m_items.size() )
                return QModelIndex();

            beginInsertRows( QModelIndex(), row+1, row+1 );
            m_items.insert( m_items.begin() + row + 1, m_items[row] );
            if ( m_items[row].pgp )
                m_items[row+1].pgp = false; // enforce pgp exclusivitiy
            endInsertRows();
            return index( row+1, 0 );
        }

        void deleteRow( unsigned int row ) {
            if ( row >= m_items.size() )
                return;

            beginRemoveRows( QModelIndex(), row, row );
            m_items.erase( m_items.begin() + row );
            endInsertRows();
        }

        void clear() {
            if ( m_items.empty() )
                return;
            beginRemoveRows( QModelIndex(), 0, m_items.size()-1 );
            m_items.clear();
            endRemoveRows();
        }

        /* reimp */ int columnCount( const QModelIndex & =QModelIndex() ) const { return NumColumns; }
        /* reimp */ int rowCount( const QModelIndex & =QModelIndex() ) const { return m_items.size(); }

        /* reimp */ QVariant data( const QModelIndex & idx, int role ) const;
        /* reimp */ QVariant headerData( int section, Qt::Orientation o, int role ) const;

        /* reimp */ Qt::ItemFlags flags( const QModelIndex & idx ) const;
        /* reimp */ bool setData( const QModelIndex & idx, const QVariant & value, int role );

    private:
        bool doSetData( unsigned int row, unsigned int column, const QVariant & value, int role );
        void setExclusivePgpFlag( unsigned int row );

        static QString toolTipForColumn( int column );
        bool isLdapRow( unsigned int row ) const;
        int firstEditableColumn( unsigned int ) const {
            return Host;
        }

    private:
        struct Item {
            KUrl url;
            bool x509 : 1;
            bool pgp  : 1;
        };
        std::vector<Item> m_items;
        bool m_openPGPReadOnly : 1;
        bool m_x509ReadOnly    : 1;
        DirectoryServicesWidget::Schemes m_schemes;

    private:
        std::vector<Item>::iterator findExistingUrl( const KUrl & url ) {
            return std::find_if( m_items.begin(), m_items.end(),
                                 boost::bind( KUrl_compare(), url, boost::bind( &Item::url, _1 ) ) );
        }
    };

    class Delegate : public QItemDelegate {
        Q_OBJECT
    public:
        explicit Delegate( QObject * parent=0 )
            : QItemDelegate( parent ),
              m_schemes( DirectoryServicesWidget::AllSchemes )
        {

        }

        void setAllowedSchemes( const DirectoryServicesWidget::Schemes schemes ) {
            m_schemes = schemes;
        };
        DirectoryServicesWidget::Schemes allowedSchemes() const { return m_schemes; }

        /* reimp */
        QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & idx ) const {
            switch ( idx.column() ) {
            case Model::Scheme:
                return createSchemeWidget( parent );
            case Model::Port:
                return createPortWidget( parent );
            }
            return QItemDelegate::createEditor( parent, option, idx );
        }

        /* reimp */
        void setEditorData( QWidget * editor, const QModelIndex & idx ) const {
            switch ( idx.column() ) {
            case Model::Scheme:
                setSchemeEditorData( qobject_cast<QComboBox*>( editor ), idx.data( Qt::EditRole ).toString() );
                break;
            case Model::Port:
                setPortEditorData( qobject_cast<QSpinBox*>( editor ), idx.data( Qt::EditRole ).toInt() );
                break;
            default:
                QItemDelegate::setEditorData( editor, idx );
                break;
            }
        }

        /* reimp */
        void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & idx ) const {
            switch ( idx.column() ) {
            case Model::Scheme:
                setSchemeModelData( qobject_cast<QComboBox*>( editor ), model, idx );
                break;
            case Model::Port:
                setPortModelData( qobject_cast<QSpinBox*>( editor ), model, idx );
                break;
            default:
                QItemDelegate::setModelData( editor, model, idx );
                break;
            }
        }

        /* reimp */
        void updateEditorGeometry( QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
            if ( index.column() == Model::Scheme || index.column() == Model::Port )
                editor->setGeometry( calculate_geometry( option.rect, editor->sizeHint() ) );
            else
                QItemDelegate::updateEditorGeometry( editor, option, index );
        }

    private:
        QWidget * createSchemeWidget( QWidget * parent ) const {
            if ( !m_schemes )
                return 0;
            QComboBox * cb = new QComboBox( parent );
            for ( unsigned int i = 0 ; i < numProtocols ; ++i )
                if ( m_schemes & protocols[i].base )
                    cb->addItem( i18n( protocols[i].label ), QLatin1String(protocols[i].label) );
            assert( cb->count() > 0 );
            return cb;
        }
        void setSchemeEditorData( QComboBox * cb, const QString & scheme ) const {
            assert( cb );
            cb->setCurrentIndex( cb->findData( scheme, Qt::UserRole, Qt::MatchFixedString ) );
        }
        void setSchemeModelData( const QComboBox * cb, QAbstractItemModel * model, const QModelIndex & idx ) const {
            assert( cb );
            assert( model );
            model->setData( idx, cb->itemData( cb->currentIndex() ) );
        }

        QWidget * createPortWidget( QWidget * parent ) const {
            QSpinBox * sb = new QSpinBox( parent );
            sb->setRange( 1, USHRT_MAX ); // valid port numbers
            return sb;
        }
        void setPortEditorData( QSpinBox * sb, unsigned short port ) const {
            assert( sb );
            sb->setValue( port );
        }
        void setPortModelData( const QSpinBox * sb, QAbstractItemModel * model, const QModelIndex & idx ) const {
            assert( sb );
            assert( model );
            model->setData( idx, sb->value() );
        }

    private:
        DirectoryServicesWidget::Schemes m_schemes;
    };

}

class DirectoryServicesWidget::Private {
    friend class ::Kleo::DirectoryServicesWidget;
    DirectoryServicesWidget * const q;
public:
    explicit Private( DirectoryServicesWidget * qq )
        : q( qq ),
          protocols( AllProtocols ),
          readOnlyProtocols( NoProtocol ),
          model(),
          delegate(),
          ui( q )
    {
        ui.treeView->setModel( &model );
        ui.treeView->setItemDelegate( &delegate );

        connect( &model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                 q, SIGNAL(changed()) );
        connect( &model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                 q, SIGNAL(changed()) );
        connect( &model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                 q, SIGNAL(changed()) );
        connect( ui.treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                 q, SLOT(slotSelectionChanged()) );

        slotShowUserAndPasswordToggled( false );
    }

private:
    void slotNewClicked() {
        int row = selectedRow();
        if ( row < 0 )
            row = currentRow();
        if ( row < 0 || model.isReadOnlyRow( row ) )
            if ( protocols & OpenPGPProtocol )
                slotNewOpenPGPClicked();
            else if ( protocols & X509Protocol )
                slotNewX509Clicked();
            else
                assert( !"This should not happen.");
        else
            edit( model.duplicateRow( row ) );
    }
    void edit( const QModelIndex & index ) {
        if ( index.isValid() ) {
            ui.treeView->clearSelection();
            ui.treeView->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
            ui.treeView->edit( index );
        }
    }
    void slotNewX509Clicked() {
        edit( model.addX509Service( defaultX509Service(), true ) );
    }
    void slotNewOpenPGPClicked() {
        edit( model.addOpenPGPService( defaultOpenPGPService(), true ) );
    }
    void slotDeleteClicked() {
        model.deleteRow( selectedRow() );
    }
    void slotSelectionChanged() {
        enableDisableActions();
    }
    void slotShowUserAndPasswordToggled( bool on ) {
        QHeaderView * const hv = ui.treeView->header();
        assert( hv );
        hv->setSectionHidden( Model::UserName, !on );
        hv->setSectionHidden( Model::Password, !on );
    }

    int selectedRow() const {
        const QModelIndexList mil = ui.treeView->selectionModel()->selectedRows();
        return mil.empty() ? -1 : mil.front().row();
    }
    int currentRow() const {
        const QModelIndex idx = ui.treeView->selectionModel()->currentIndex();
        return idx.isValid() ? idx.row() : -1 ;
    }

    void showHideColumns();

    void enableDisableActions() {
        const bool x509 = ( protocols & X509Protocol    ) && !( readOnlyProtocols & X509Protocol    ) ;
        const bool pgp  = ( protocols & OpenPGPProtocol ) && !( readOnlyProtocols & OpenPGPProtocol ) ;
        ui.newX509Action.setEnabled( x509 );
        ui.newOpenPGPAction.setEnabled( pgp );
        if ( x509 && pgp ) {
            ui.newTB->setMenu( &ui.newMenu );
            ui.newTB->setPopupMode( QToolButton::MenuButtonPopup );
        } else {
            ui.newTB->setMenu( 0 );
            ui.newTB->setPopupMode( QToolButton::DelayedPopup );
            ui.newTB->setEnabled( x509 || pgp );
        }
        const int row = selectedRow();
        ui.deleteTB->setEnabled( row >= 0 && !model.isReadOnlyRow( row ) );
    }

private:
    Protocols protocols;
    Protocols readOnlyProtocols;
    Model model;
    Delegate delegate;
    struct UI : Ui_DirectoryServicesWidget {
        QAction newX509Action;
        QAction newOpenPGPAction;
        QMenu newMenu;

        explicit UI( DirectoryServicesWidget * q )
            : Ui_DirectoryServicesWidget(),
              newX509Action( i18nc("New X.509 Directory Server", "X.509"), q ),
              newOpenPGPAction( i18nc("New OpenPGP Directory Server", "OpenPGP"), q ),
              newMenu( q )
        {
            newX509Action.setObjectName( QLatin1String("newX509Action") );
            newOpenPGPAction.setObjectName( QLatin1String("newOpenPGPAction") );
            newMenu.setObjectName( QLatin1String("newMenu") );

            setupUi( q );

            connect( &newX509Action, SIGNAL(triggered()), q, SLOT(slotNewX509Clicked()) );
            connect( &newOpenPGPAction, SIGNAL(triggered()), q, SLOT(slotNewOpenPGPClicked()) );

            newMenu.addAction( &newX509Action );
            newMenu.addAction( &newOpenPGPAction );
            
            newTB->setMenu( &newMenu );
        }

    } ui;
};

DirectoryServicesWidget::DirectoryServicesWidget( QWidget * p, Qt::WindowFlags f )
    : QWidget( p, f ), d( new Private( this ) )
{

}


DirectoryServicesWidget::~DirectoryServicesWidget() {
    delete d;
}

void DirectoryServicesWidget::setAllowedSchemes( Schemes schemes ) {
    d->delegate.setAllowedSchemes( schemes );
    d->showHideColumns();
}

DirectoryServicesWidget::Schemes DirectoryServicesWidget::allowedSchemes() const {
    return d->delegate.allowedSchemes();
}

void DirectoryServicesWidget::setAllowedProtocols( Protocols protocols ) {
    if ( d->protocols == protocols )
        return;
    d->protocols = protocols;
    d->showHideColumns();
    d->enableDisableActions();
}

DirectoryServicesWidget::Protocols DirectoryServicesWidget::allowedProtocols() const {
    return d->protocols;
}

void DirectoryServicesWidget::setReadOnlyProtocols( Protocols protocols ) {
    if ( d->readOnlyProtocols == protocols )
        return;
    d->readOnlyProtocols = protocols;
    d->model.setOpenPGPReadOnly( protocols & OpenPGPProtocol );
    d->model.setX509ReadOnly( protocols & X509Protocol );
    d->enableDisableActions();
}

DirectoryServicesWidget::Protocols DirectoryServicesWidget::readOnlyProtocols() const {
    return d->readOnlyProtocols;
}

void DirectoryServicesWidget::addOpenPGPServices( const KUrl::List & urls ) {
    Q_FOREACH( const KUrl & url, urls )
        d->model.addOpenPGPService( url );
}

KUrl::List DirectoryServicesWidget::openPGPServices() const {
    KUrl::List result;
    for ( unsigned int i = 0, end = d->model.numServices() ; i != end ; ++i )
        if ( d->model.isOpenPGPService( i ) )
            result.push_back( d->model.service( i ) );
    return result;
}

void DirectoryServicesWidget::addX509Services( const KUrl::List & urls ) {
    Q_FOREACH( const KUrl & url, urls )
        d->model.addX509Service( url );
}

KUrl::List DirectoryServicesWidget::x509Services() const {
    KUrl::List result;
    for ( unsigned int i = 0, end = d->model.numServices() ; i != end ; ++i )
        if ( d->model.isX509Service( i ) )
            result.push_back( d->model.service( i ) );
    return result;
}

void DirectoryServicesWidget::clear() {
    if ( !d->model.numServices() )
        return;
    d->model.clear();
    emit changed();
}

void DirectoryServicesWidget::Private::showHideColumns() {
    QHeaderView * const hv = ui.treeView->header();
    assert( hv );
    // don't show 'scheme' column when only accepting X509Protocol (###?)
    hv->setSectionHidden( Model::Scheme,  protocols == X509Protocol );
    // hide the protocol selection columns for if only one protocol is allowed anyway:
    hv->setSectionHidden( Model::X509,    protocols != AllProtocols );
    hv->setSectionHidden( Model::OpenPGP, protocols != AllProtocols );
}

//
// Model
//

QVariant Model::headerData( int section, Qt::Orientation orientation, int role ) const {
    if ( orientation == Qt::Horizontal )
        if ( role == Qt::ToolTipRole )
            return toolTipForColumn( section );
        else if ( role == Qt::DisplayRole )
            switch ( section ) {
            case Scheme:   return i18n("Scheme");
            case Host:     return i18n("Server Name");
            case Port:     return i18n("Server Port");
            case BaseDN:   return i18n("Base DN");
            case UserName: return i18n("User Name");
            case Password: return i18n("Password");
            case X509:     return i18n("X.509");
            case OpenPGP:  return i18n("OpenPGP");
            default:       return QVariant();
            }
        else
            return QVariant();
    else
        return QAbstractTableModel::headerData( section, orientation, role );
}

QVariant Model::data( const QModelIndex & index, int role ) const {
    const unsigned int row = index.row();
    if ( index.isValid() && row < m_items.size() )
        switch ( role ) {
        case Qt::ToolTipRole: {
            const QString tt = toolTipForColumn( index.column() );
            if ( !isReadOnlyRow( index.row() ) )
                return tt;
            else
                return tt.isEmpty()
                    ? i18n("(read-only)")
                    : i18nc("amended tooltip; %1: original tooltip",
                            "%1 (read-only)", tt );
        }
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch ( index.column() ) {
            case Scheme:
                return display_scheme( m_items[row].url );
            case Host:
                return display_host( m_items[row].url );
            case Port:
                return display_port( m_items[row].url );
            case BaseDN:
                if ( isLdapRow( row ) )
                    return KUrl::fromPercentEncoding( m_items[row].url.query().mid( 1 ).toLatin1() );  // decode query and skip leading '?'
                else
                    return QVariant();
            case UserName:
                return m_items[row].url.user();
            case Password:
                return m_items[row].url.pass();
            case X509:
            case OpenPGP:
            default:
                return QVariant();
            }
        case Qt::CheckStateRole:
            switch ( index.column() ) {
            case X509:
                return m_items[row].x509 && isLdapRow( row ) ? Qt::Checked : Qt::Unchecked ;
            case OpenPGP:
                return m_items[row].pgp  ? Qt::Checked : Qt::Unchecked ;
            default:
                return QVariant();
            }
        }
    return QVariant();
}

bool Model::isLdapRow( unsigned int row ) const {
    if ( row >= m_items.size() )
        return false;
    return is_ldap_scheme( m_items[row].url );
}

Qt::ItemFlags Model::flags( const QModelIndex & index ) const {
    const unsigned int row = index.row();
    Qt::ItemFlags flags = QAbstractTableModel::flags( index );
    if ( isReadOnlyRow( row ) )
        flags &= ~Qt::ItemIsSelectable ;
    if ( index.isValid() && row < m_items.size() )
        switch ( index.column() ) {
        case Scheme:
            switch ( m_schemes ) {
            default:
                if ( !isReadOnlyRow( row ) )
                    return flags | Qt::ItemIsEditable ;
                // else fall through
            case DirectoryServicesWidget::HKP:
            case DirectoryServicesWidget::HTTP:
            case DirectoryServicesWidget::FTP:
            case DirectoryServicesWidget::LDAP:
                // only one scheme allowed -> no editing possible
                return flags & ~(Qt::ItemIsEditable|Qt::ItemIsEnabled) ;
            }
        case Host:
        case Port:
            if ( isReadOnlyRow( row ) )
                return flags & ~(Qt::ItemIsEditable|Qt::ItemIsEnabled) ;
            else
                return flags | Qt::ItemIsEditable ;
        case BaseDN:
            if ( isLdapRow( row ) && !isReadOnlyRow( row ) )
                return flags | Qt::ItemIsEditable ;
            else
                return flags & ~(Qt::ItemIsEditable|Qt::ItemIsEnabled) ;
        case UserName:
        case Password:
            if ( isReadOnlyRow( row ) )
                return flags & ~(Qt::ItemIsEditable|Qt::ItemIsEnabled) ;
            else
                return flags | Qt::ItemIsEditable ;
        case X509:
            if ( !isLdapRow( row ) )
                return flags & ~(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled) ;
            // fall through
        case OpenPGP:
            if ( isReadOnlyRow( row ) )
                return flags & ~(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled) ;
            else
                return flags | Qt::ItemIsUserCheckable ;
        }
    return flags;
}

bool Model::setData( const QModelIndex & idx, const QVariant & value, int role ) {
    const unsigned int row = idx.row();
    if ( !idx.isValid() || row >= m_items.size() )
        return false;
    if ( isReadOnlyRow( row ) )
        return false;
    if ( !doSetData( row, idx.column(), value, role ) )
        return false;
    emit dataChanged( idx, idx );
    return true;
}

bool Model::doSetData( unsigned int row, unsigned int column, const QVariant & value, int role ) {
    if ( role == Qt::EditRole )
        switch ( column ) {
        case Scheme:
            if ( is_default_port( m_items[row].url ) ) {
                // drag the port along with scheme changes
                m_items[row].url.setPort( -1 );
                const QModelIndex changed = index( row, Port );
                emit dataChanged( changed, changed );
            }
            m_items[row].url.setProtocol( value.toString() );
            return true;
        case Host:
            if ( display_host( m_items[row].url ) != m_items[row].url.host() ) {
                m_items[row].url.setProtocol( display_scheme( m_items[row].url ) );
                m_items[row].url.setPath( QLatin1String("/") );
            }
            m_items[row].url.setHost( value.toString() );
            return true;
        case Port:
            if ( value.toUInt() == default_port( display_scheme( m_items[row].url ) ) )
                m_items[row].url.setPort( -1 );
            else
                m_items[row].url.setPort( value.toUInt() );
            return true;
        case BaseDN:
            if ( value.toString().isEmpty() ) {
                m_items[row].url.setPath( QString() );
                m_items[row].url.setQuery( QString() );
            } else {
                m_items[row].url.setPath( QLatin1String("/") ); // workaround KUrl parsing bug
                m_items[row].url.setQuery( value.toString() );
            }
            return true;
        case UserName:
            m_items[row].url.setUser( value.toString() );
            return true;
        case Password:
            m_items[row].url.setPass( value.toString() );
            return true;
        }
    if ( role == Qt::CheckStateRole )
        switch ( column ) {
        case X509:
            m_items[row].x509 = value.toInt() == Qt::Checked ;
            return true;
        case OpenPGP:
            {
                const bool on = value.toInt() == Qt::Checked ;
                if ( on )
                    setExclusivePgpFlag( row );
                else
                    m_items[row].pgp = false;
            }
            return true;
        }
    return false;
}

void Model::setExclusivePgpFlag( unsigned int row ) {
    if ( row >= m_items.size() || m_items[row].pgp )
        return;
    m_items[row].pgp = true; // dataChanged() for this one is supposed to be emitted by the caller
    for ( unsigned int i = 0, end = m_items.size() ; i < end ; ++i )
        if ( i != row )
            if ( m_items[i].pgp ) {
                m_items[i].pgp = false;
                const QModelIndex changed = index( i, OpenPGP );
                emit dataChanged( changed, changed );
                break;
            }
}

// static
QString Model::toolTipForColumn( int column ) {
    switch ( column ) {
    case Scheme:   return i18n("Select the access protocol (scheme) that the "
                               "directory service is available through.");
    case Host:     return i18n("Enter the name or IP address of the server "
                               "hosting the directory service.");
    case Port:     return i18n("<b>(Optional, the default is fine in most cases)</b> "
                               "Pick the port number the directory service is "
                               "listening on.");
    case BaseDN:   return i18n("<b>(Only for LDAP)</b> "
                               "Enter the base DN for this LDAP server to "
                               "limit searches to only that subtree of the directory.");
    case UserName: return i18n("<b>(Optional)</b> "
                               "Enter your user name here, if needed.");
    case Password: return i18n("<b>(Optional, not recommended)</b> "
                               "Enter your password here, if needed. "
                               "Note that the password will be saved in the clear "
                               "in a config file in your home directory.");
    case X509:     return i18n("Check this column if this directory service is "
                               "providing S/MIME (X.509) certificates.");
    case OpenPGP:  return i18n("Check this column if this directory service is "
                               "providing OpenPGP certificates.");
    default:
        return QString();
    }
}

#include "directoryserviceswidget.moc"
#include "moc_directoryserviceswidget.cpp"
