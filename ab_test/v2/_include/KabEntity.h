#include <qstring.h>
#include <qdict.h>
#include <qvaluelist.h>

#include <KabEnum.h>
#include <KabSubValue.h>

#ifndef KAB_ENTITY_H
#define KAB_ENTITY_H

namespace KAB
{

// Forward decl.
class AddressBook;

class Entity
{
  public:
    
    Entity()
      : name_         ("unknown"  ),
        addressBook_  (0          ),
        type_         ("unknown"  ),
        dirty_        (false      ),
        seq_          (SEQ++      )
    {
      if (!initialised_) { _init(); initialised_ = true; }
      _generateID();
    }
    
    Entity(const QString & type, AddressBook & pab, const QString & name);
    
    Entity(const Entity & e)
      : id_           (e.id_          ),
        name_         (e.name_        ),
        addressBook_  (e.addressBook_ ),
        xValues_      (e.xValues_     ),
        type_         (e.type_        ),
        dirty_        (e.dirty_       ),
        seq_          (SEQ++          )
    {
      // Empty.
    }

    virtual ~Entity()
    {
      // Empty.
    }
    
    Entity & operator = (const Entity & e)
    {
      if (this == &e) return *this;
      id_           = e.id_;
      addressBook_  = e.addressBook_;
      xValues_      = e.xValues_;
      type_         = e.type_;
      dirty_        = e.dirty_;
      seq_          = SEQ++;

      return *this; 
    }
    
    bool operator == (const Entity & e) const
    {
      return (
        (id_      == e.id_      ) &&
        (xValues_ == e.xValues_ ));
    }
    
    UniqueID  id() const { return id_; }
    
    void setID(const UniqueID & id)
    { touch(); id_ = id; }
    
    QString   name() const { return name_; }
    
    void setName(const QString & name)
    { touch(); name_ = name; }
    
    QString type() const { return type_; }
    
    bool isDirty()  { return dirty_; }
    void setClean() { dirty_ = false;}
    void touch()    { dirty_ = true; }
    
    AddressBook * addressBook() const { return addressBook_; }
    
    XValueList    xValues()       const { return xValues_;      }
    void setXValues       (const XValueList     & l)
    { touch(); xValues_ = l; } 

  protected:
    
    UniqueID id_;

    // Order dependency
    QString       name_;
    AddressBook   * addressBook_;
    XValueList    xValues_;
    
  private:
    
    QString           type_;
    bool              dirty_;
    unsigned long int seq_;
    // End order dependency

    void _init();
    void _generateID();
    
    static bool               initialised_;
    static unsigned long int  SEQ;
    
    static QString hostName_;
    static QString timeStr_;
    static QString pidStr_;
};

typedef QDict<Entity> EntityDict;
typedef QDictIterator<Entity> EntityDictIterator;
typedef QList<Entity> EntityList;
typedef QListIterator<Entity> EntityListIterator;

} // End namespace KAB

#endif

