/**
 * \file listviewiterator_tpl.h
 * Implementation of an iteration through an entire QListView.
 *
 * \version $Header$
 */

#include <qlistview.h>

template <typename VIEW, typename ITEM>
ListViewIterator<VIEW, ITEM>::ListViewIterator()
    : m_item( 0 )
{
}

template <typename VIEW, typename ITEM>
ListViewIterator<VIEW, ITEM>::ListViewIterator( const ListViewIterator<VIEW, ITEM>& a_list )
    : m_item( a_list.m_item )
{
}

template <typename VIEW, typename ITEM>
ListViewIterator<VIEW, ITEM>::ListViewIterator( ITEM* a_item )
    : m_item( a_item ? new QListViewItemIterator( a_item ) : 0 )
{
}

template <typename VIEW, typename ITEM>
ListViewIterator<VIEW, ITEM>::ListViewIterator( VIEW* a_view )
    : m_item( a_view ? new QListViewItemIterator( a_view ) : 0 )
{
}

template <typename VIEW, typename ITEM>
ListViewIterator<VIEW, ITEM>::~ListViewIterator()
{
    delete m_item;
}

template <typename VIEW, typename ITEM>
ListViewIterator<VIEW, ITEM>& ListViewIterator<VIEW, ITEM>::operator=( const ListViewIterator<VIEW, ITEM>& a_list )
{
    m_item = a_list.m_item;
    return *this;
}

template <typename VIEW, typename ITEM>
ListViewIterator<VIEW, ITEM>& ListViewIterator<VIEW, ITEM>::operator++()
{
    if ( !m_item )
        return *this;
    ++(*m_item);
    return *this;
}

template <typename VIEW, typename ITEM>
ListViewIterator<VIEW, ITEM> ListViewIterator<VIEW, ITEM>::operator++(int)
{
    if ( !m_item )
        return *this;
    ListViewIterator<VIEW, ITEM> i( *this );
    (*m_item)++;
    return i;
}

template <typename VIEW, typename ITEM>
ListViewIterator<VIEW, ITEM>& ListViewIterator<VIEW, ITEM>::operator--()
{
    if ( !m_item )
        return *this;
    --(*m_item);
    return *this;
}

template <typename VIEW, typename ITEM>
ListViewIterator<VIEW, ITEM> ListViewIterator<VIEW, ITEM>::operator--(int)
{
    if ( !m_item )
        return *this;
    ListViewIterator<VIEW, ITEM> i( *this );
    (*m_item)--;
    return i;
}

template <typename VIEW, typename ITEM>
ITEM* ListViewIterator<VIEW, ITEM>::operator*() const
{
    if ( !m_item )
        return 0;
    return dynamic_cast<ITEM*>( m_item->current() );
}

template <typename VIEW, typename ITEM>
bool ListViewIterator<VIEW, ITEM>::operator==( const ListViewIterator& a_list )
{
    return operator*() == a_list.operator*();
}

template <typename VIEW, typename ITEM>
bool ListViewIterator<VIEW, ITEM>::operator!=( const ListViewIterator& a_list )
{
    return operator*() != a_list.operator*();
}
