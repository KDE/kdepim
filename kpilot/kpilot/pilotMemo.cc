#include <iostream.h>
#include <klocale.h>
#include "pi-source.h"
#include "pi-memo.h"
#include "pilotMemo.h"

PilotMemo::PilotMemo(PilotRecord* rec)
  : PilotAppCategory(rec)
    {
    unpack(rec->getData(), 1);
    }

void PilotMemo::unpack(const void *text, int firstTime) 
{
  if (!firstTime && fText)
    {
      delete fText;
      delete fTitle;
    }
  
  fSize = strlen((const char *) text) + 1;
  fText = new char [fSize];
  (void) strcpy(fText, (const char *) text);

  int memoTitleLen = 0;
  while(fText[memoTitleLen] && (fText[memoTitleLen] != '\n'))
    memoTitleLen++;
  fTitle = new char[memoTitleLen+1];
  strncpy(fTitle, fText, memoTitleLen);
  fTitle[memoTitleLen] = 0;
}

// The indirection just to make the base class happy
void *PilotMemo::internalPack(unsigned char *buf) 
    {
    return strcpy((char *) buf, fText);
    }

void *PilotMemo::pack(void *buf, int *len) 
    {
    if (*len < fSize)
	return NULL;

    *len = fSize;

    return internalPack((unsigned char *) buf);
    }


QString
PilotMemo::shortTitle() const
{
	QString t = QString(getTitle()).simplifyWhiteSpace();

	if (t.length() < 32) return t;
	t.truncate(40);

	int spaceIndex = t.findRev(' ');
	if (spaceIndex > 32)
	{
		t.truncate(spaceIndex);
	}
	
	t += "...";

	return t;
}

QString
PilotMemo::sensibleTitle() const
{
	const char *s = getTitle();

	if (s && *s)
	{
		return QString(s);
	}
	else
	{
		return QString(i18n("[unknown]"));
	}
}
