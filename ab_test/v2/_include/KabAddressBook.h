#include <qregexp.h>
#include <KabEntity.h>

#ifndef KAB_ADDRESSBOOK_H
#define KAB_ADDRESSBOOK_H

namespace KAB
{

class AddressBook
{
  public:
    
    /**
     * Default ctor.
     * Call init() afterwards, before using any methods.
     */
    AddressBook()
    {
      // Empty.
    }

    /**
     * Copy ctor.
     */
    AddressBook(const AddressBook & ab)
        :  entityDict_    (ab.entityDict_)
    {
      // Empty.
    }

    /**
     * xxref.
     */
    AddressBook & operator = (const AddressBook & ab)
    {
      if (this == &ab) return *this;
      entityDict_ = ab.entityDict_;
      return *this;
    }
    /**
     * dtor.
     */
    ~AddressBook()
    {
      // Empty.
    }
    /**
     * You must call this to initialise the addressbook. Do it after
     * you have created an addressbook and before you use any of
     * its methods.
     */
    void init();
    /**
     *  Find the entity with the specified id.
     *  @returns 0 if not found, else a pointer to the found entity.
     */
    Entity * entity(const QString & id)
    {
      return entityDict_[id];
    }
    
    EntityList entitiesOfType(const QString & t)
    {
      EntityList l;
      EntityDictIterator it(entityDict_);
      for (; it.current(); ++it)
        if (it.current()->type() == t)
          l.append(it.current());
      return l;
    }
 
    EntityList entitiesOfType(const QRegExp & r)
    {
      EntityList l;
      EntityDictIterator it(entityDict_);
      for (; it.current(); ++it)
        if (r.match(it.current()->type()))
          l.append(it.current());
      return l;
    }
 
    EntityList allEntities()
    {
      EntityList l;
      EntityDictIterator it(entityDict_);
      for (; it.current(); ++it)
          l.append(it.current());
      return l;
    }
 
    EntityList dirtyEntities()
    {
      EntityList l;
      EntityDictIterator it(entityDict_);
      for (; it.current(); ++it)
        if (it.current()->isDirty())
          l.append(it.current());
      return l;
    }
    
    /**
     * Add a new entity. The entity will be copied so you may delete it.
     */
    void add(const Entity * e)
    {
      entityDict_.insert(e->id(), e);
    }
    /**
     * Delete an entity.
     * @returns true if the entity existed, false if not.
     */
    bool remove(const QString & key)
    {
      return entityDict_.remove(key);
    }
    /**
     * Replace an entity.
     */
    void update(const Entity & e)
    {
      entityDict_.replace(e.id(), new Entity(e));
    }
    /**
     * Import entries from another addressbook.
     * @arg format the format the addressbook is stored in. Currently only
     * VCARD and LDIF are recognised.
     * @arg filename where to get the addressbook from.
     * @returns -1 if failed to import, else the number of entries that
     * were imported.
     */
    Q_UINT32 import(const QString & format, const QString & filename);
    
  private:
    
    EntityDict  entityDict_;
};

} // End namespace KAB

#endif

