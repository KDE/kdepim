#ifndef KAB_ENTITY_H
#define KAB_ENTITY_H

#include <qstring.h>
#include <qdatastream.h>

#include <KabEnum.h>
#include <KabField.h>


namespace KAB
{

class Entity
{
  public:
    
    /**
     * Create a new entity with name 'unknown'.
     */
    Entity()
      : name_         ("unknown"),
        type_         (EntityTypeEntity),
        dirty_        (false),
        seq_          (SEQ++)
    {
      if (!initialised_) { _init(); initialised_ = true; }
      _generateID();
    }
    
    /**
     * @internal
     */ 
    Entity(EntityType t)
      : type_(t)
    {
      if (!initialised_) { _init(); initialised_ = true; }
      _generateID();
    }
    
    /**
     * Create a new entity with the specified name.
     */
    Entity(const QString & name);
    
    Entity(const QString & name, EntityType);

    /**
     * Copy ctor.
     */
    Entity(const Entity & e)
      : id_     (e.id_    ),
        name_   (e.name_  ),
        fields_ (e.fields_),
        dirty_  (e.dirty_ ),
        seq_    (SEQ++    )
    {
      // Empty.
    }

    /**
     * Dtor.
     */
    virtual ~Entity()
    {
      // Empty.
    }
    
    /**
     * xxref
     */
    Entity & operator = (const Entity & e)
    {
      if (this == &e) return *this;
      id_     = e.id_;
      fields_ = e.fields_;
      type_   = e.type_;
      dirty_  = e.dirty_;
      seq_    = SEQ++;

      return *this; 
    }
    
    /**
     * Comparison operator.
     */
    bool operator == (const Entity & e) const
    {
      return ((id_ == e.id_) && (fields_ == e.fields_));
    }
    
    /**
     * @return The unique id of this entity.
     */
    UniqueID id() const
    {
      return id_;
    }
    
    /**
     * @return The name of this entity.
     */
    QString name() const
    {
      return name_;
    }
    
    /**
     * @return The type of this entity (EntityTypeEntity / EntityTypeGroup)
     */
    EntityType type() const
    {
      return type_;
    }
    
    /**
     * Add a new field.
     */
    void add(const Field & f)
    {
      touch();
      fields_.append(f);
    }

    /**
     * Add a new field. This function is provided for convenience and differs
     * from the above only in the arguments it accepts.
     */
    void add(
      const QString & n,
      const QByteArray & v,
      const QString & t = QString::null,
      const QString & s = QString::null)
    {
      touch();
      fields_.append(Field(n, v, t, s));
    }


    /**
     * Remove the field with the specified name.
     */
    bool remove(const QString & name)
    {
      FieldList::ConstIterator it;
        
        for (it = fields_.begin(); it != fields_.end(); ++it)
          if ((*it).name() == name) {
            touch();
            fields_.remove(*it);
            return true;
          }

        return false;
    }
 
    /**
     * Remove the given field.
     */
    bool remove(const Field & f)
    {
      return remove(f.name());
    }
  
    /**
     * Find the field with the given name.
     *
     * f is set to the found field, if found.
     * 
     * @return true if the field was found, else false.
     */
    bool field(const QString & fieldName, Field & f)
    {
      FieldList::ConstIterator it;
      
      for (it = fields_.begin(); it != fields_.end(); ++it)
        if ((*it).name() == fieldName) {
          f = *it;
          return true;
        }

      return false;
    }

    /**
     * @return All fields.
     */
    FieldList fieldList() const
    {
      return fields_;
    }
    
    /**
     * Clear out the field list and assign l to it.
     */
    void setFieldList(const FieldList & l)
    {
      touch();
      fields_ = l;
    } 
    
    /**
     * Is this entity modified since last write ?
     * @return true if this entity has been modified since last write.
     */
    bool isDirty() const
    {
      return dirty_;
    }
    
    /**
     * Sets this entity to be unmodified since last write. Use with caution.
     */
    void  setClean()
    {
      dirty_ = false;
    }
    
    /**
     * Set this entity to be modified since last write.
     *
     * You shouldn't need to do this unless you modify object data without
     * using the member functions.
     */
    void  touch()
    {
      dirty_ = true;
    }
    
    /**
     * @internal
     */
    virtual bool write(const QCString &, backendWrite *);
    /**
     * @internal
     */
    virtual bool read (const QCString &, backendRead *);
    
    /**
     * @internal
     */
    virtual void save(QDataStream &);
    /**
     * @internal
     */
    virtual void load(QDataStream &);

    /**
     * @internal
     */
    void setID(const QString & s) { initialised_ = true; id_ = s; }

  protected:
    
    /**
     * @internal
     */
    UniqueID id_;

    // Order dependency

    /**
     * @internal
     */
    QString   name_;
    /**
     * @internal
     */
    FieldList fields_;
  
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

typedef QList<Entity> EntityList;
typedef QListIterator<Entity> EntityListIterator;

} // End namespace KAB

#endif

