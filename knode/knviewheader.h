/***************************************************************************
                          knviewheader.h  -  description
                             -------------------
    
    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef KNVIEWHEADER_H
#define KNVIEWHEADER_H

#include <qstring.h>
#include <qbitarray.h>
#include <qlist.h>


class KNViewHeader {
  
  public:
    KNViewHeader();
    ~KNViewHeader();

    //some common headers
    static const char** predefs();
        
    //name
    const QString& name()               { return n_ame; }
    void setName(const QString &s)      { n_ame = s; }
    bool hasName()                      { return !n_ame.isEmpty(); }

    //translated name
    QString translatedName();                     // *trys* to translate the name
    void setTranslatedName(const QString &s);     // *trys* to retranslate the name to english
    void setTranslateName(bool b)       { t_ranslateName=b; }
    bool translateName()                { return t_ranslateName; }

    //header
    const QString& header()             { return h_eader; }
    void setHeader(const QString &s)    { h_eader = s; }

    //flagd
    bool flag(int i)                    { return f_lags.at(i); }
    void setFlag(int i, bool b)         { f_lags.setBit(i, b); }

    //HTML-tags
    void createTags();
    const QString& nameOpenTag()        { return t_ags[0]; }
    const QString& nameCloseTag()       { return t_ags[1]; }
    const QString& headerOpenTag()      { return t_ags[2]; }
    const QString& headerCloseTag()     { return t_ags[3]; }
            
  protected:
    bool t_ranslateName;
    QString n_ame, h_eader, t_ags[4];
    QBitArray f_lags;

};

#endif
