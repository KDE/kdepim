/***************************************************************************
 *   snippet feature from kdevelop/plugins/snippet/                        *
 *                                                                         * 
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SNIPPETCONFIG_H
#define SNIPPETCONFIG_H

#include <tqstring.h>
#include <tqrect.h>


/**
This class stores the values that can be configured via the
KDevelop settings dialog
@author Robert Gruber
*/
class SnippetConfig{
public:
    SnippetConfig();

    ~SnippetConfig();

  bool useToolTips() { return (bToolTip); };
  int getInputMethod() { return (iInputMethod); };
  TQString getDelimiter() { return (strDelimiter); };
  TQRect getSingleRect() { return (rSingle); };
  TQRect getMultiRect() { return (rMulti); };
  int getAutoOpenGroups() { return iAutoOpenGroups; }
  
  void setToolTips(bool b) { bToolTip=b; };
  void setInputMethod(int i) { iInputMethod=i; };
  void setDelimiter(TQString s) { strDelimiter=s; };
  void setSingleRect(TQRect r) {
    rSingle = (r.isValid())?r:TQRect();
  }
  void setMultiRect(TQRect r) {
    rMulti = (r.isValid())?r:TQRect();
  }
  void setAutoOpenGroups(int autoopen) { iAutoOpenGroups = autoopen; }

protected:
    bool bToolTip;
    int iInputMethod;
    TQString strDelimiter;
    TQRect rSingle;
    TQRect rMulti;
    int iMultiBasicHeight;
    int iMultiCount;
    int iAutoOpenGroups;
};

#endif
