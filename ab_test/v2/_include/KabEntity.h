#include <qstring.h>
#include <qdict.h>
#include <qvaluelist.h>
#include <qdatastream.h>

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
      : name_         ("unknown"),
        type_         (EntityTypeEntity),
        dirty_        (false),
        seq_          (SEQ++)
    {
      if (!initialised_) { _init(); initialised_ = true; }
      _generateID();
    }
        
    Entity(EntityType t)
      : type_(t)
    {
    }
    
    Entity(EntityType type, const QString & name);
    
    Entity(const Entity & e)
      : id_           (e.id_          ),
        name_         (e.name_        ),
        xValues_      (e.xValues_     ),
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
      xValues_      = e.xValues_;
      type_         = e.type_;
      dirty_        = e.dirty_;
      seq_          = SEQ++;

      return *this; 
    }
    
    bool operator == (const Entity & e) const
    { return ((id_ == e.id_) && (xValues_ == e.xValues_)); }
    
    UniqueID      id          () const                { return id_;           }
    QString       name        () const                { return name_;         }
    EntityType    type        () const                { return type_;         }
    
    void          setID       (const UniqueID & id)   { touch(); id_ = id;    }
    void          setName     (const QString & s)     { touch(); name_ = s;   }
    
    XValueList    xValues     () const                { return xValues_;      }
    void          setXValues  (const XValueList & l)  { touch(); xValues_ = l;} 
    
    bool          isDirty()                           { return dirty_;        }
    void          setClean()                          { dirty_ = false;       }
    void          touch()                             { dirty_ = true;        }
    
    bool          write       (const QCString &, backendWrite *);
    bool          read        (const QCString &, backendRead *);
    
    virtual void save(QDataStream &);
    virtual void load(QDataStream &);

  protected:
    
    UniqueID id_;

    // Order dependency
    QString       name_;
    XValueList    xValues_;
    
  private:
    
    EntityType        type_;
    bool              dirty_;
    unsigned long int seq_;
    // End order dependency

    static bool     initialised_;
    static Q_UINT32 SEQ;
    
    static QString  hostName_;
    static QString  timeStr_;
    static QString  pidStr_;
    
    void            _init();
    void            _generateID();
};

typedef QDict<Entity> EntityDict;
typedef QDictIterator<Entity> EntityDictIterator;
typedef QList<Entity> EntityList;
typedef QListIterator<Entity> EntityListIterator;

} // End namespace KAB

#endif

