#include <sys/time.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "KabEntity.h"
#include "KabAddressBook.h"

using namespace KAB;

bool Entity::initialised_ = false;
unsigned long int Entity::SEQ = 0;

QString Entity::hostName_;
QString Entity::timeStr_;
QString Entity::pidStr_;

Entity::Entity(const QString & type, AddressBook & ab, const QString & name)
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
}

