// $Id$
// (C) 2001 by Cornelius Schumacher

#ifndef _KANDYPREFS_H
#define _KANDYPREFS_H

#include <qstring.h>
#include <qfont.h>
#include <qcolor.h>
#include <qstringlist.h>
#include <qdict.h>

#include <kconfigskeleton.h>

class KConfig;

class KandyPrefs : public KConfigSkeleton
{
  public:
    virtual ~KandyPrefs();
  
    /** Get instance of KandyPrefs. It is made sure that there is only one
    instance. */
    static KandyPrefs *instance();
  
  private:
    /** Constructor disabled for public. Use instance() to create a KandyPrefs
    object. */
    KandyPrefs();

    static KandyPrefs *mInstance;

  public:
    // preferences data
    QString mSerialDevice;
    bool mStartupModem;
    bool mStartupTerminalWin;
    bool mStartupMobileWin;
};

#endif
