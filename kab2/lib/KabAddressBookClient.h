#ifndef KAB_ADDRESSBOOK_CLIENT_H
#define KAB_ADDRESSBOOK_CLIENT_H

#include <qregexp.h>
#include <qstrlist.h>

#include <kurl.h>

#include <KabEnum.h>
#include <KabEntity.h>


namespace KAB
{
 
class Group;
    
class AddressBookClient
{
  public:
    
    AddressBookClient(
      const QString & name, const KURL & url);
    
    ~AddressBookClient();
    
    /**
     * The name of this addressbook.
     */
    QString name() const { return name_; }
    /**
     * The location of (the server for) this addressbook.
     */
    KURL url() const { return url_; }
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

    Entity * entity(const QString &);
    Group * group(const QString &);

  private:
    
    QString name_;
    KURL url_;
};


} // End namespace KAB

#endif

