#include <qregexp.h>
#include <qstrlist.h>
#include <KabEnum.h>
#include <KabEntity.h>
#include <kurl.h>

#ifndef KAB_ADDRESSBOOK_H
#define KAB_ADDRESSBOOK_H

namespace KAB
{
 
class Person;
class Location;
class Member;
class Group;
    
class AddressBook
{
  public:
    
    /**
     * Default ctor.
     * Call init() afterwards, before using any methods.
     */
    AddressBook(const QString & format, const KURL & url);
    /**
     * Check this after calling the ctor. If it returns false,
     * DO NOT use this object. (You may safely delete it)
     */
    bool usable() { return usable_; }
    /**
     * Copy ctor.
     */
    AddressBook(const AddressBook &);
    /**
     * xxref.
     */
    AddressBook & operator = (const AddressBook &);
    /**
     * dtor.
     */
    ~AddressBook();
    /**
     *  Find the Entity with the specified key.
     *  @returns 0 if not found, else a pointer to the found Entity.
     */
    Entity    * entity    (const QString & key);
    /**
     *  Find the Person with the specified id.
     *  @returns 0 if not found, else a pointer to the found Person.
     */
    Person    * person    (const QString & key);
    /**
     *  Find the Location with the specified key.
     *  @returns 0 if not found, else a pointer to the found Location.
     */
    Location  * location  (const QString & key);
    /**
     *  Find the Member with the specified key.
     *  @returns 0 if not found, else a pointer to the found Member.
     */
    Member    * member    (const QString & key);
    /**
     *  Find the Group with the specified key.
     *  @returns 0 if not found, else a pointer to the found Group.
     */
    Group     * group     (const QString & key);
    
    QStrList keysOfType(EntityType);

    QStrList topLevelGroups();
    /**
     * Add a new entity. The entity will be copied so you may delete it.
     */
    void write(Entity *);
    /**
     * Delete an entity.
     * @returns true if the entity existed, false if not.
     */
    bool remove(const QString &);
    /**
     * Replace an entity.
     */
    void update(Entity *);
    /**
     * Write any touched entities.
     */
    void write();
    /**
     * Import entries from another addressbook.
     * @arg format the format the addressbook is stored in. Currently only
     * VCARD and LDIF are recognised.
     * @arg filename where to get the addressbook from.
     * @returns -1 if failed to import, else the number of entries that
     * were imported.
     */
    Q_UINT32 import(const QString & format, const QString & filename);
    
    QStrList allKeys();
    
    EntityType  keyToEntityType(const QCString &);
    Entity *    createEntityOfType(EntityType);
    
  private:
    
    bool _initBackend();
    
    QString format_;
    KURL url_;

    backendInit       init_;
    backendRead       read_;
    backendWrite      write_;
    backendRemove     remove_;
    backendAllKeys    allKeys_;
    KabBackendHandle  handle_;
    
    bool usable_;
};


} // End namespace KAB

#endif

