#ifndef LISTVIEWITERATOR_H
#define LISTVIEWITERATOR_H

/**
 * \file listviewiterator.h
 * Declaration of an iteration through an entire QListView.
 *
 * \version $Header$
 */

#include <iterator>

class QListViewItemIterator;

template <typename VIEW, typename ITEM>
class ListViewIterator : public std::iterator <std::bidirectional_iterator_tag, ITEM>
{
public:
    ListViewIterator();
    ListViewIterator( const ListViewIterator& a_list );
    ListViewIterator( ITEM* a_item );
    ListViewIterator( VIEW* a_view );

    ~ListViewIterator();

    ListViewIterator& operator=( const ListViewIterator& a_list );

    ListViewIterator& operator++();
    ListViewIterator operator++(int);

    ListViewIterator& operator--();
    ListViewIterator operator--(int);

    ITEM* operator*() const;

    bool operator==( const ListViewIterator& a_list );
    bool operator!=( const ListViewIterator& a_list );

private:
    QListViewItemIterator* m_item;
};

/**
 * \class ListViewIterator
 * \brief Bidirectional Iterator through an entire QListView.
 *
 * This class is a wrapper around QListViewItemIterator. The primary value of this class is that it is a template
 * and uses dynamic_cast. This means that it will downcast correctly and can be used where classes derived from
 * QListView and QListViewItem are being used. Without this using a normal QListViewItemIterator would require
 * a cast throughout client code. This just cleans that up a bit.
 *
 * \par Note:
 * Since this class has a default constructor, m_item may be null. Each member routine must check this
 * before dereferencing the pointer.
 *
 * \par Incrementing Algorithm:
 * Incrementing and decrementing are forwarded to the QListViewItemIterator.
 */

/**
 * \fn ListViewIterator::ListViewIterator()
 * \brief Default constructor, won't do much at first.
 */

/**
 * \fn ListViewIterator::ListViewIterator( const ListViewIterator& a_list )
 * \brief Create a ListViewIterator as a copy of an existing iterator.
 */

/**
 * \fn ListViewIterator::ListViewIterator( ITEM* a_item )
 * \brief Construct a ListViewIterator with a QListViewItem as the root.
 */

/**
 * \fn ListViewIterator::ListViewIterator( VIEW* a_view )
 * \brief Construct a ListViewIterator from a QListView. The firstChild of the QListView will be the root of the iterator.
 *
 * Note that on construction, the firstChild will be called on a_view. The first increment of the iterator will move
 * to the second item. An alternative design would be for the first increment to move it to the first valid item. That does
 * not make as much sense following the idea that construction is resource acquisition. So, the following is valid:
 * 
 * \code
 * QListView* view = new QListView();
 * // Enter elements into the view.
 * ListViewIterator<QListView, QListViewItem> iter( view );
 * cout << (*iter)->text(0);
 * \endcode
 */

/**
 * \fn ListViewIterator::~ListViewIterator()
 * \brief QListViewItemIterator has a destructor so should call it.
 */

/**
 * \fn ListViewIterator& ListViewIterator::operator=( const ListViewIterator& a_list )
 * \brief Assign one iterator to another.
 */

/**
 * \fn ListViewIterator& ListViewIterator::operator++()
 * \brief Move to the next element and return it, preincrement.
 */

/**
 * \fn ListViewIterator ListViewIterator::operator++(int)
 * \brief Move to the next element, returning the current element, postincrement.
 */

/**
 * \fn ListViewIterator& ListViewIterator::operator--()
 * \brief Move to the previous element and return it, predecrement.
 */

/**
 * \fn ListViewIterator ListViewIterator::operator--(int)
 * \brief Move to the previous, returning the current element, postdecrement.
 */

/**
 * \fn ITEM* ListViewIterator::operator*() const
 * \brief Return the item at the current iterator position.
 */

/**
 * \fn bool ListViewIterator::operator==( const ListViewIterator& a_list )
 * \brief Compares the equality of the pointers returned by operator*().
 *
 * This function is not defined in QListViewItemIterator. Comparing pointers is the most straightforward
 * way of checking equality. But, there may be some differences in how QListViewItemIterator is implemented
 * to make this not an appropriate solution. In particulary, there may be issues with reference counting,
 * but none are known at this time.
 */

/**
 * \fn bool ListViewIterator::operator!=( const ListViewIterator& a_list )
 * \brief Compares the inequality of the pointers return by operator*().
 *
 * \sa The note for ListVIewIterator::operator==(const ListViewIterator&).
 */

#include "listviewiterator_tpl.h"

#endif
