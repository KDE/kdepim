#include <sys/time.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "KabEntity.h"
#include "KabAddressBook.h"
#include "KabBackend.h"

using namespace KAB;

bool Entity::initialised_ = false;
unsigned long int Entity::SEQ = 0;

QString Entity::hostName_;
QString Entity::timeStr_;
QString Entity::pidStr_;

Entity::Entity(EntityType type, AddressBook & ab, const QString & name)
  : name_(name),
    addressBook_(&ab),
    type_(type),
    dirty_(false),
    seq_(SEQ++)
{
  if (!initialised_) { _init(); initialised_ = true; }
  _generateID();
  addressBook_->add(this);
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
    case EntityTypePerson:          id_ += ":p"; break;
    case EntityTypeLocation:        id_ += ":l"; break;
    case EntityTypeGroup:           id_ += ":g"; break;
    case EntityTypeEntity: default: id_ += ":e"; break;
  }
}

  bool
Entity::write(const QCString & key, backendWrite * writer) const
{
  QByteArray a;
  
  QDataStream str(a, IO_WriteOnly);
  str << *this;
  
  bool retval =
  (*writer)(key, a);
  
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
  str >> *this;
  
  return true;
}

  QDataStream &
KAB::operator << (QDataStream & str, const Entity & e)
{
  str << e.name_ << e.xValues_ << (int)e.type_ << (int)(e.seq_);
  return str;
}
 
  QDataStream &
KAB::operator >> (QDataStream & str, Entity & e)
{
  str >> e.name_ >> e.xValues_;
  int i;
  str >> i;
  e.type_ = (EntityType)i;
  str >> i;
  e.seq_ = i;
  return str;
}


