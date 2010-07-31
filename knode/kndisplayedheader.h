/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNDISPLAYEDHEADER_H
#define KNDISPLAYEDHEADER_H

#include <tqbitarray.h>


class KNDisplayedHeader {

  public:
    KNDisplayedHeader();
    ~KNDisplayedHeader();

    //some common headers
    static const char** predefs();

    //name
    const TQString& name()               { return n_ame; }
    void setName(const TQString &s)      { n_ame = s; }
    bool hasName() const                     { return !n_ame.isEmpty(); }

    //translated name
    TQString translatedName();                     // *tries* to translate the name
    void setTranslatedName(const TQString &s);     // *tries* to retranslate the name to english
    void setTranslateName(bool b)       { t_ranslateName=b; }
    bool translateName() const                { return t_ranslateName; }

    //header
    const TQString& header()             { return h_eader; }
    void setHeader(const TQString &s)    { h_eader = s; }

    //flags
    bool flag(int i)                    { return f_lags.at(i); }
    void setFlag(int i, bool b)         { f_lags.setBit(i, b); }

    //HTML-tags
    void createTags();
    const TQString& nameOpenTag()        { return t_ags[0]; }
    const TQString& nameCloseTag()       { return t_ags[1]; }
    const TQString& headerOpenTag()      { return t_ags[2]; }
    const TQString& headerCloseTag()     { return t_ags[3]; }

  protected:
    bool t_ranslateName;
    TQString n_ame, h_eader, t_ags[4];
    TQBitArray f_lags;

};

#endif
