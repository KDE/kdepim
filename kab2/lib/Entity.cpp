#include <iostream>

#include <sys/time.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <KabEntity.h>
#include <KabAddressBook.h>

using namespace KAB;

bool      Entity::initialised_  (false);
Q_UINT32  Entity::SEQ           (0);
QString   Entity::hostName_;
QString   Entity::timeStr_;
QString   Entity::pidStr_;

Entity::Entity(const QString & name)
  : name_(name),
    type_(EntityTypeEntity),
    dirty_(false),
    seq_(SEQ++)
{
  if (!initialised_) { _init(); initialised_ = true; }
  _generateID();
}


Entity::Entity(const QString & name, EntityType type)
  : name_(name),
    type_(type),
    dirty_(false),
    seq_(SEQ++)
{
  if (!initialised_) { _init(); initialised_ = true; }
  _generateID();
}

  void
Entity::_init()
{
  struct utsname utsName;
  hostName_ = uname(&utsName) == 0 ? utsName.nodename : "localhost";
  
  struct timeval timeVal;
  struct timezone timeZone;
    
  gettimeofday(&timeVal, &timeZone);
  timeStr_.setNum(timeVal.tv_sec);
  
  pidStr_.setNum(getpid());
}

  void
Entity::_generateID()
{
  QString seqStr;
  id_ = timeStr_ + '.' + pidStr_ + '_' + seqStr.setNum(seq_) + '.' + hostName_;
  switch (type_) {
    case EntityTypeGroup:           id_ += ":g"; break;
    case EntityTypeEntity: default: id_ += ":e"; break;
  }
}

  bool
Entity::write(const QCString & key, backendWrite * writer)
{
  QByteArray a;
  
  QDataStream str(a, IO_WriteOnly);
  save(str);
  
  bool retval =
  (*writer)(key, a);
  
  dirty_ = false;
  return retval;
}

  bool
Entity::read(const QCString & key, backendRead * reader)
{
  QByteArray a;
  id_ = key;
  
  if (!(*reader)(key, a))
    return false;
  
  QDataStream str(a, IO_ReadOnly);
  load(str);
  
  return true;
}

  void
Entity::save(QDataStream & str)
{
  str << name_;
  str << fields_.count();
  FieldList::Iterator it(fields_.begin());
  for (; it != fields_.end(); ++it)
    (*it).save(str);
  str << (int)type_ << (int)(seq_);
}
 
  void
Entity::load(QDataStream & str)
{
  str >> name_;
  int valCount;
  str >> valCount;
  for (int i = 0; i < valCount; i++) {
    Field x;
    x.load(str);
    fields_.append(x);
  }
  int i;
  str >> i;
  type_ = (EntityType)i;
  str >> i;
  seq_ = i;
}


