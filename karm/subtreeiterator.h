#ifndef SUBTREEITERATOR_H
#define SUBTREEITERATOR_H

/**
 * \file subtreeiterator.h
 * Declaration of an iteration through a complete subtree of a QListView.
 *
 * \version $Header$
 */

// #include <iterator> // include if inheriting from iterator
#include <vector>

// class QListView;
// class QListViewItem;

template <typename VIEW, typename ITEM>
class SubtreeIterator /* this isn't working */ /* : public std::iterator <std::forward_iterator_tag, ITEM> */
{
public:
    SubtreeIterator();
    SubtreeIterator( const SubtreeIterator& a_subtree );
    SubtreeIterator( ITEM* a_item );
    SubtreeIterator( VIEW* a_view );

    SubtreeIterator& operator=( const SubtreeIterator& a_subtree );

    SubtreeIterator& operator++();
    SubtreeIterator operator++(int);

    ITEM* operator*();

    bool operator==( const SubtreeIterator& a_subtree );
    bool operator!=( const SubtreeIterator& a_subtree );

private:
    void increment();

    ITEM* m_item;
    std::vector<ITEM*> m_childStack;
};

/**
 * \class SubtreeIterator
 * \brief Forward Iterator through an entire subtree.
 *
 * Take a pointer to the root of a subtree and iterate through all of the elements of the subtree.
 * Use this to perform an operation on every node in the total tree.
 * This iterator is not shared and does not perform a deep copy. Standard pointer semantics.
 *
 * \par Note:
 * Since this class has a default constructor, m_item may be null. Each member routine must check this
 * before dereferencing the pointer.
 *
 * \par Incrementing Algorithm:
 * The incrementing algorithm here is a little more complicated. Normally, this is a problem calling for recursion.
 * Since, that is not really an option had to be a little more careful. Given the following tree.
 *
 * \verbatim
                Root
              ___+___
             |   |   |
             A   B   C
          ___+___
         |   |   |
         D   E   F
      ___+___
     G   H   I
         |
         J
   \endverbatim
 *
 * The following locations are where the special cases in the increment logic need to be:
 * <OL>
 * <LI>Root, transitioning to A. The next item will be the firstChild if it exists. Push the new item to the stack.
 * <LI>G, transitioning to H. If there is no firstChild, the nextSibling will be used. Replace the top of the stack with the new item.
 * <LI>J, transitioning to I. If there is no firstChild and no nextSibling, move to the next sibling of the parent.
 *     This pops one element from the stack and pushes the sibling.
 * <LI>C, finishing the traversal. If there is no firstChild, no nextSibling, and no parents other than Root, the traversal is done.
 *     Set m_item to null and return.
 * </OL>
 * 
 * \par Note:
 * QListViewItemIterator almost does what is needed. The problem is that it also iterates to the siblings of the root of the tree.
 */

/**
 * \fn SubtreeIterator::SubtreeIterator()
 * \brief Default constructor, won't do much at first.
 */

/**
 * \fn SubtreeIterator::SubtreeIterator( const SubtreeIterator& a_subtree )
 * \brief Construct a SubtreeIterator as a copy of an existing iterator.
 */

/**
 * \fn SubtreeIterator::SubtreeIterator( ITEM* a_item )
 *
 * \brief Construct a SubtreeIterator with a QListViewItem as the root of the subtree.
 */

/**
 * \fn SubtreeIterator::SubtreeIterator( VIEW* a_view )
 *
 * \brief Construct a SubtreeIterator from a QListView. The firstChild of the QListView will be the root of the iterator.
 */

/**
 * \fn SubtreeIterator& SubtreeIterator::operator=( const SubtreeIterator& a_subtree )
 * \brief Assign one iterator to another.
 */

/**
 * \fn SubtreeIterator& SubtreeIterator::operator++()
 *
 * \brief Move to the next element and return it, preincrement.
 */

/**
 * \fn SubtreeIterator SubtreeIterator::operator++(int)
 *
 * \brief Move to the next element, returning the current element, postincrement.
 */

/**
 * \fn ITEM* SubtreeIterator::operator*() 
 *
 * \brief Return the item at the current iterator position.
 */

/**
 * \fn bool SubtreeIterator::operator==( const SubtreeIterator& a_subtree )
 * \brief Are two iterators equal? Do they point to the same QListViewItem?
 *
 * Also performs an equality comparison on m_childStack. So, it is equal if the current pointer
 * is the same and if the child stack is the same. If the pointers are the same but the stacks
 * are different, or vice versa, then something probably got corrupted. Or maybe elements were
 * inserted while an iterator was active.
 */

/**
 * \fn bool SubtreeIterator::operator!=( const SubtreeIterator& a_subtree )
 * \brief Are two iterators not equal? Do they point to different QListVIewItems?
 */

/**
 * \fn void SubtreeIterator::increment()
 *
 * \brief Used internally to perform the actual incrementing of the iterator.
 */

#include "subtreeiterator_tpl.h"

#endif
