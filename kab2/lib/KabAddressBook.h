#ifndef KAB_ADDRESSBOOK_H
#define KAB_ADDRESSBOOK_H

#include <qregexp.h>
#include <qstrlist.h>

#include <kurl.h>

#include <KabEnum.h>
#include <KabEntity.h>


namespace KAB
{
 
class Group;
    

/**
 * @short A KAB AddressBook
 *
 * An AddressBook has an associated 'backend', which provides the
 * loading and saving functionality. An AddressBook doesn't actually
 * store any data itself.
 *
 * You usually don't have to worry about creating these objects.
 * Nearly everything you need is available through the @ref Kab2
 * interface, which encapsulates a collection of AddressBook.
 *
 * The AddressBook object will try to load a plugin to handle the
 * data type specified in 'format'. If it is unable to do this,
 * it is impossible to use the AddressBook object, so it will be
 * deleted and this method will return 0.
 *
 * The url is usually just a filename (e.g. 'file:/tmp/test.gdbm').
 *
 * You may specify something like 'ldap://ldap_server:port/' to access
 * a remote server, if your backend plugin supports this. You'll know if
 * it doesn't because create() will return 0.
 */
class AddressBook
{
  public:
    /**
     * @short Create an AddressBook object.
     *
     * Attempts to create and initialise a new AddressBook object.
     *
     * @return 0 on failure, else a pointer to an AddressBook object.
     * 
     * @param name The name of this addressbook.
     * @param format The format that the addressbook is stored in.
     * @param url The location of the addressbook data.
     */
    static AddressBook * create(const QString & name, const QString & format, const KURL & url);

    /**
     * @short Destructor.
     * 
     * The backend plugin will have its reference count decremented.
     */
    ~AddressBook();
    /**
     * The name of this addressbook.
     */
    QString name() const { return name_; }
    /**
     * The format this addressbook is stored with (by its backend).
     */
    QString format() const { return format_; }
    /**
     * The location of (the backend of) this addressbook.
     */
    KURL url() const { return url_; }
    /**
     *  Find the Entity with the specified key.
     *  @return 0 if not found, else a pointer to the found Entity.
     */
    Entity    * entity    (const QString & key);
    /**
     *  Find the Group with the specified key.
     *  @return 0 if not found, else a pointer to the found Group.
     */
    Group     * group     (const QString & key);
    /**
     * @short List of all toplevel groups.
     *
     * Get a list of all toplevel groups.
     *
     * @return List of the keys of all toplevel groups, i.e. those with no
     * parent group. The list is set to automatically delete its members
     * upon destruction.
     */
    QStrList topLevelGroups();
    /**
     * @short Add a new entity.
     *
     * Add a new entity.
     * 
     * The entity will be copied so you may delete it.
     */
    void write(Entity *);
    /**
     * @short Delete an entity.
     * 
     * Delete an entity.
     * 
     * @return true if the entity existed, false if not.
     */
    bool remove(const QString &);
    /**
     * @short Replace an entity.
     * 
     * Replace an entity.
     *
     * If the given entity did not already exist, it is simply added.
     */
    void update(Entity *);
    /**
     * Write any touched entities.
     *
     * Does not return until all dirty entries have been written.
     */
    void write();
    /**
     * @short Import entries from another addressbook using a different format.
     *
     * Import entries from another addressbook using a different format.
     * 
     * @param format the format the addressbook is stored in. Currently only
     * "vcard" and "ldif" are recognised, but these are provided by plugins,
     * so if you have more, just use them !
     *
     * @param filename where to get the addressbook from.
     * @return The number of entities that were imported.
     */
    Q_UINT32 import(const QString & format, const QString & filename);
    /**
     * @short Retrieve a list of all keys in the addressbook.
     *
     * Retrieve a list of all keys in the addressbook. Note that this
     * may involve a large network transaction (or a large amount of
     * disk access), so don't call it unless you need to. Generally
     * used for debugging.
     *
     * @return A QStrList holding all the keys in the database. This list
     * is set to automatically delete its members on destruction.
     */
    QStrList allKeys();
    /**
     * @internal
     */
    EntityType  keyToEntityType(const QCString &);
    /**
     * @internal
     */
    Entity *    createEntityOfType(EntityType);
    /**
     * @internal
     */
    QStrList keysOfType(EntityType);

  protected: 
    /**
     * @internal
     * Default ctor.
     */
    AddressBook(const QString & name, const QString & format, const KURL & url);
    /**
     * @internal
     * Check this after calling the ctor. If it returns false,
     * DO NOT use this object. (You may safely delete it)
     */
    bool usable() { return usable_; }
    /**
     * @internal
     * Disabled copy ctor.
     */
    AddressBook(const AddressBook &);
    /**
     * @internal
     * Disabled xxref.
     */
    AddressBook & operator = (const AddressBook &);
   
  private:
    
    bool _initBackend();
    
    QString name_;
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

