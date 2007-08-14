#include "keylistsortfilterproxymodel.h"

#include "keylistmodel.h"

#include <kleo/keyfilter.h>

#include <gpgme++/key.h>

#include <utils/stl_util.h>
#include <boost/bind.hpp>

#include <cassert>

using namespace Kleo;
using namespace boost;
using namespace GpgME;

class KeyListSortFilterProxyModel::Private {
    friend class ::Kleo::KeyListSortFilterProxyModel;
public:
    explicit Private()
        : keyFilters(), mode( AllFiltersMatch ) {}
    ~Private() {}

private:
    std::vector< shared_ptr<const KeyFilter> > keyFilters;
    MatchMode mode;
};


KeyListSortFilterProxyModel::KeyListSortFilterProxyModel( QObject * p )
    : QSortFilterProxyModel( p ), d( new Private )
{
    setDynamicSortFilter( true );
}

KeyListSortFilterProxyModel::~KeyListSortFilterProxyModel() {}

std::vector< shared_ptr<const KeyFilter> > KeyListSortFilterProxyModel::keyFilters() const {
    return d->keyFilters;
}

void KeyListSortFilterProxyModel::setKeyFilters( const std::vector< shared_ptr<const KeyFilter> > & kf ) {
    if ( kf == d->keyFilters )
        return;
    d->keyFilters = kf;
    invalidateFilter();
}

KeyListSortFilterProxyModel::MatchMode KeyListSortFilterProxyModel::keyFilterMatchMode() const {
    return d->mode;
}

void KeyListSortFilterProxyModel::setKeyFilterMatchMode( MatchMode mode ) {
    if ( mode == d->mode )
        return;
    d->mode = mode;
    invalidateFilter();
}


bool KeyListSortFilterProxyModel::filterAcceptsRow( int source_row, const QModelIndex & source_parent ) const {

    //
    // 1. Check that name or email matches filterRegExp
    //
    const QModelIndex nameIndex = sourceModel()->index( source_row, AbstractKeyListModel::PrettyName, source_parent );
    const QModelIndex emailIndex = sourceModel()->index( source_row, AbstractKeyListModel::PrettyEMail, source_parent );

    const QString name = nameIndex.data().toString();
    const QString email = emailIndex.data().toString();

    const QRegExp rx = filterRegExp();
    if ( !name.contains( rx ) && !email.contains( rx ) )
        return false;

    //
    // 2. Check that key filters match (if any are defined)
    //
    if ( !d->keyFilters.empty() ) { // avoid artifacts when no filters are defined

        assert( qobject_cast<AbstractKeyListModel*>( sourceModel() ) );
        const AbstractKeyListModel * const klm = static_cast<AbstractKeyListModel*>( sourceModel() );

        const Key key = klm->key( nameIndex );

        switch ( keyFilterMatchMode() ) {
        case AllFiltersMatch:
            return kdtools::all( d->keyFilters.begin(), d->keyFilters.end(),
                                 bind( &KeyFilter::matches, _1, key ) );
        case AnyFilterMatches:
            return kdtools::any( d->keyFilters.begin(), d->keyFilters.end(),
                                 bind( &KeyFilter::matches, _1, key ) );
        };
    }

    // 3. match by default:
    return true;
}

#include "moc_keylistsortfilterproxymodel.cpp"
