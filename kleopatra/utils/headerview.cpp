#include "headerview.h"

#include <algorithm>
#include <numeric>
#include <cassert>

#include <KDebug>

//#define ENABLE_HEADERVIEW_DEBUG

#ifdef ENABLE_HEADERVIEW_DEBUG
# define hvDebug kDebug
#else
# define hvDebug if ( true ) {} else kDebug
#endif

using namespace Kleo;

class HeaderView::Private {
public:
    Private()
        : mousePressed( false ),
          modes(),
          sizes()
    {

    }

    void ensureNumSections( unsigned int num ) {
        if ( num > modes.size() )
            modes.resize( num, QHeaderView::Fixed );
    }

    bool mousePressed : 1;
    std::vector<QHeaderView::ResizeMode> modes;
    std::vector<int> sizes;
};

HeaderView::HeaderView( Qt::Orientation o, QWidget * p )
    : QHeaderView( o, p ), d( new Private )
{

}

HeaderView::~HeaderView() {}

static std::vector<int> section_sizes( const QHeaderView * view ) {
    assert( view );
    std::vector<int> result;
    result.reserve( view->count() );
    for ( int i = 0, end = view->count() ; i != end ; ++i )
        result.push_back( view->sectionSize( i ) );
    return result;
}

static void apply_section_sizes( QHeaderView * view, const std::vector<int> & newSizes ) {
    assert( view );
    for ( unsigned int i = 0, end = newSizes.size() ; i != end ; ++i )
        view->resizeSection( i, newSizes[i] );
}

namespace {

    template <typename T_container>
    inline typename T_container::value_type lookup( const T_container & c, unsigned int i, const typename T_container::value_type & defaultValue ) {
        return i < c.size() ? c[i] : defaultValue ;
    }

}

template <typename T, typename A>
QDebug operator<<( QDebug debug, const std::vector<T,A> & v ) {
    debug.nospace() << "std::vector(";
    for ( typename std::vector<T,A>::size_type i = 0; i < v.size(); ++i ) {
        if (i)
            debug << ", ";
        debug << v[i];
    }
    debug << ")";
    return debug.space();
}



static std::vector<int> calculate_section_sizes( const std::vector<int> & oldSizes, int newLength, const std::vector<QHeaderView::ResizeMode> & modes, int minSize ) {

    if ( oldSizes.empty() ) {
        hvDebug() << "no existing sizes";
        return std::vector<int>();
    }

    int oldLength = 0, fixedLength = 0, stretchLength = 0;
    int numStretchSections = 0;
    for ( unsigned int i = 0, end = oldSizes.size() ; i != end ; ++i ) {
        oldLength += oldSizes[i];
        if ( lookup( modes, i, QHeaderView::Fixed ) == QHeaderView::Stretch ) {
            stretchLength += oldSizes[i];
            ++numStretchSections;
        } else {
            fixedLength += oldSizes[i];
        }
    }

    if ( oldLength <= 0 ) {
        hvDebug() << "no existing lengths - returning equidistant sizes";
        return std::vector<int>( oldSizes.size(), newLength / oldSizes.size() );
    }

    const int stretchableSpace = std::max( newLength - fixedLength, 0 );

    std::vector<int> newSizes;
    newSizes.reserve( oldSizes.size() );
    for ( unsigned int i = 0, end = oldSizes.size() ; i != end ; ++i )
        newSizes.push_back( std::max( minSize,
                                      lookup( modes, i, QHeaderView::Fixed ) == QHeaderView::Stretch
                                      ? stretchLength ? stretchableSpace * oldSizes[i] / stretchLength : stretchableSpace / numStretchSections
                                      : oldSizes[i] ) );

    hvDebug() << "\noldSizes = " << oldSizes << "/" << oldLength
             << "\nnewSizes = " << newSizes << "/" << newLength;

    return newSizes;
}


void HeaderView::setSectionSizes( const std::vector<int> & sizes ) {
    hvDebug() << sizes;
    d->ensureNumSections( sizes.size() );
    d->sizes = sizes;
    apply_section_sizes( this, sizes );
    hvDebug() << "->" << sectionSizes();
}

std::vector<int> HeaderView::sectionSizes() const {
    return section_sizes( this );
}

void HeaderView::setSectionResizeMode( unsigned int section, ResizeMode mode ) {
    d->ensureNumSections( section+1 );
    d->modes[section] = mode;
}

void HeaderView::setModel( QAbstractItemModel * model ) {

    hvDebug() << "before" << section_sizes( this );

    QHeaderView::setModel( model );

    hvDebug() << "after " << section_sizes( this );

}

void HeaderView::setRootIndex( const QModelIndex & idx ) {
    hvDebug() << "before" << section_sizes( this );
    QHeaderView::setRootIndex( idx );
    hvDebug() << "after " << section_sizes( this );
}

void HeaderView::mousePressEvent( QMouseEvent * e ) {
    d->mousePressed = true;
    QHeaderView::mousePressEvent( e );
}

void HeaderView::mouseReleaseEvent( QMouseEvent * e ) {
    d->mousePressed = false;
    QHeaderView::mouseReleaseEvent( e );
}

void HeaderView::updateGeometries() {

    const std::vector<int> oldSizes = d->mousePressed ? section_sizes( this ) : d->sizes ;

    hvDebug() << "before" << section_sizes( this ) << '(' << d->sizes << ')';

    QHeaderView::updateGeometries();

    hvDebug() << "after " << section_sizes( this );

    const std::vector<int> newSizes = calculate_section_sizes( oldSizes, width(), d->modes, minimumSectionSize() );
    d->sizes = newSizes;

    apply_section_sizes( this, newSizes );
}

#include "moc_headerview.cpp"
