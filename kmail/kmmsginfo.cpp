// kmmsginfo.cpp

#include "kmmsginfo.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>


//-----------------------------------------------------------------------------
KMMsgInfo::~KMMsgInfo()
{
  if (mMsg) delete mMsg;
}


//-----------------------------------------------------------------------------
void KMMsgInfo::init(KMMessage::Status aStatus, unsigned long aOffset, 
		     unsigned long aSize, KMMessage* aMsg)
{
  mStatus = aStatus;
  mOffset = aOffset;
  mSize   = aSize;
  mMsg    = aMsg;
}


//-----------------------------------------------------------------------------
void KMMsgInfo::init(const char* aStatusStr, unsigned long aOffset, 
		     unsigned long aSize, KMMessage* aMsg)
{
  init(KMMessage::stUnknown, aOffset, aSize, aMsg);
  setStatus(aStatusStr);
}


//-----------------------------------------------------------------------------
void KMMsgInfo::deleteMsg(void)
{
  if (mMsg)
  {
    delete mMsg;
    mMsg = NULL;
  }
}


//-----------------------------------------------------------------------------
void KMMsgInfo::setStatus(KMMessage::Status aStatus)
{
  mStatus = aStatus;
}


//-----------------------------------------------------------------------------
void KMMsgInfo::setStatus(const char* aStatusStr)
{
  static KMMessage::Status stList[] = 
  {
    KMMessage::stDeleted, KMMessage::stNew, 
    KMMessage::stUnread, KMMessage::stOld, 
    KMMessage::stUnknown   /*...must be at the end of this list! */
  };
  int i;

  for (i=0; stList[i]!=KMMessage::stUnknown; i++)
    if (strchr(aStatusStr, (char)stList[i])) break;

  mStatus = stList[i];
}


//-----------------------------------------------------------------------------
void KMMsgInfo::fromString(const char* aStr)
{
  char st;
  static char fromStr[256], subjStr[256];

  assert(aStr != NULL);

  sscanf(aStr,"%c %lu %lu |%s|%s", &st, &mOffset, &mSize, fromStr, subjStr);
  mStatus = (KMMessage::Status)st;
  mMsg = NULL;
}


//-----------------------------------------------------------------------------
const char* KMMsgInfo::asString(void) const
{
  static char str[512];

  sprintf(str, "%c %-.8lu %-.8lu |%s|%s", (char)mStatus, mOffset, mSize,
	mMsg ? mMsg->from() : "", mMsg ? mMsg->subject() : "");
  return str;
}
