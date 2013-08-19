/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNCOLLECTION_H
#define KNCOLLECTION_H

#include <boost/shared_ptr.hpp>
#include <QString>

class KNCollectionViewItem;


/** Abstract base class for everything that is visible in the folder tree.
 * This includes:
 * - news groups
 * - folders
 * - news server accounts
 */
class KNCollection {

  public:
    enum collectionType {   CTnntpAccount, CTgroup,
                            CTfolder, CTcategory,
                            CTvirtualGroup };

    /**
     * Shared pointer to a KNCollection. To be used instead of raw KNCollection*.
     */
    typedef boost::shared_ptr<KNCollection> Ptr;


    /** Create a new collection.
     * @param p The parent collection.
     */
    explicit KNCollection( KNCollection::Ptr p );
    virtual ~KNCollection();

    /// Returns the collection type.
    virtual collectionType type() = 0;

    /** Returns the listview item representing this collection in the
     * folder tree.
     */
    KNCollectionViewItem* listItem() const { return l_istItem; }
    /** Sets the listview item which represents this collection in the
     * folder tree.
     */
    void setListItem(KNCollectionViewItem *i);
    /** Updates the listview item after the collection has changed. */
    virtual void updateListItem();

    // info
    virtual QString path() = 0;
    /** Load the properties/settings of this collection. */
    virtual bool readInfo( const QString &confPath ) = 0;
    /** Save the properties/settings of this collection. */
    virtual void writeConfig() = 0;

    /// Returns the parent collection.
    KNCollection::Ptr parent() const { return p_arent; }
    /// Sets the parent collection.
    virtual void setParent( KNCollection::Ptr p ) { p_arent = p; }

    /// Returns the collection name.
    virtual const QString& name() { return n_ame; }
    /// Sets the collection name.
    void setName( const QString &s ) { n_ame = s; }

    // count
    int count()const                       { return c_ount; }
    void setCount(int i)              { c_ount=i; }
    void incCount(int i)              { c_ount+=i; }
    void decCount(int i)              { c_ount-=i; }

  protected:
    /**
     * Returns a shared pointer pointing to this collection.
     */
    virtual KNCollection::Ptr selfPtr() = 0;

    /// A pointer to the parent collection.
    KNCollection::Ptr p_arent;
    /// The list view item representing this collection in the folder tree.
    KNCollectionViewItem *l_istItem;
    /// The name of this collection.
    QString n_ame;
    int c_ount;

};

#endif
