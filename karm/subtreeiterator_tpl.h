/**
 * \file subtreeiterator_tpl.h
 * Implementation of the iterator through a complete subtree of a QListView.
 *
 * \version $Header$
 */

#include <qlistview.h>

// #include "subtreeiterator.h"

template <typename VIEW, typename ITEM>
SubtreeIterator<VIEW, ITEM>::SubtreeIterator()
    : m_item( 0 ),
    m_childStack()
{
}

template <typename VIEW, typename ITEM>
SubtreeIterator<VIEW, ITEM>::SubtreeIterator( const SubtreeIterator<VIEW, ITEM>& a_subtree )
    : m_item( a_subtree.m_item ),
    m_childStack( a_subtree.m_childStack )
{
}

template <typename VIEW, typename ITEM>
SubtreeIterator<VIEW, ITEM>::SubtreeIterator( ITEM* a_item )
    : m_item( a_item ),
    m_childStack()
{
}

template <typename VIEW, typename ITEM>
SubtreeIterator<VIEW, ITEM>::SubtreeIterator( VIEW* a_view )
    : m_item( a_view ? dynamic_cast<ITEM*>( a_view->firstChild() ) : 0 ),
    m_childStack()
{
}

template <typename VIEW, typename ITEM>
SubtreeIterator<VIEW, ITEM>& SubtreeIterator<VIEW, ITEM>::operator=( const SubtreeIterator<VIEW, ITEM>& a_subtree )
{
    m_item = a_subtree.m_item;
    m_childStack = a_subtree.m_childStack;

    return *this;
}

template <typename VIEW, typename ITEM>
SubtreeIterator<VIEW, ITEM>& SubtreeIterator<VIEW, ITEM>::operator++()
{
    increment();
    return *this;
}

template <typename VIEW, typename ITEM>
SubtreeIterator<VIEW, ITEM> SubtreeIterator<VIEW, ITEM>::operator++(int)
{
    SubtreeIterator<VIEW, ITEM> s( m_item );
    increment();
    return s;
}

template <typename VIEW, typename ITEM>
ITEM* SubtreeIterator<VIEW, ITEM>::operator*() 
{
    return m_item;
}

template <typename VIEW, typename ITEM>
bool SubtreeIterator<VIEW, ITEM>::operator==( const SubtreeIterator<VIEW, ITEM>& a_subtree )
{
    bool pointer = m_item == a_subtree.m_item;
    bool stack = m_childStack == a_subtree.m_childStack;
    // if ( pointer ) // Not sure about this.
        // assert( stack );
    return pointer && stack;
}

template <typename VIEW, typename ITEM>
bool SubtreeIterator<VIEW, ITEM>::operator!=( const SubtreeIterator<VIEW, ITEM>& a_subtree )
{
    return !operator==( a_subtree );
}

template <typename VIEW, typename ITEM>
void SubtreeIterator<VIEW, ITEM>::increment()
{
#if 0
    if ( m_item ) {
        qDebug( QString( "  Subtree: Incrementing from item %1" ).arg( m_item->text( 0 ) ) );
    }
    else {
        qDebug( "  Subtree: Incrementing with no current item." );
    }
#endif

    ITEM* child = m_item ? dynamic_cast<ITEM*>( m_item->firstChild() ) : 0;
    if ( child ) {
        m_childStack.push_back( child );
        m_item = child;
        return;
    }

    // If the tree has just one level, don't want to take the sibling of the root.
    ITEM* sibling = m_item ? dynamic_cast<ITEM*>( m_item->nextSibling() ) : 0;
    if ( sibling && m_childStack.size() >= 1 ) {
        m_childStack.pop_back();
        m_childStack.push_back( sibling );
        m_item = sibling;
        return;
    }

    // Greater than 2 because the first is the parent and the second is the current level.
    while ( m_childStack.size() >= 2 ) {
        m_childStack.pop_back();
        ITEM* uplevel = m_childStack.back();
        sibling = dynamic_cast<ITEM*>( uplevel->nextSibling() );
        if ( sibling ) {
            m_childStack.pop_back();
            m_childStack.push_back( sibling );
            m_item = sibling;
            return;
        }
    }

    m_childStack.clear();
    m_item = 0;

    return;
}
