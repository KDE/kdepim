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
    
    static void loadAll();
    static void saveAll();
    static KNViewHeader* first()        { return instances.first(); }
    static KNViewHeader* next()         { return instances.next(); }
    static KNViewHeader* newItem();
    static void remove(KNViewHeader *h);
    static void clear();
    static void up(KNViewHeader *h);
    static void down(KNViewHeader *h);
    static const char** predefs();                // some common headers
        
    const QString& name()               { return n_ame; }
    QString translatedName();                     // *trys* to translate the name
    const QString& header()             { return h_eader; }
    bool flag(int i)                    { return flags.at(i); }
    bool hasName()                      { return !n_ame.isEmpty(); }
    void setHeader(const QString &s)    { h_eader = s; }
    void setName(const QString &s)      { n_ame = s; }
    void setTranslatedName(const QString &s);     // *trys* to retranslate the name to english
    void setFlag(int i, bool b)         { flags.setBit(i, b); }
    
    void createTags();
    const QString& nameOpenTag()        { return tags[0]; }
    const QString& nameCloseTag()       { return tags[1]; }   
    const QString& headerOpenTag()      { return tags[2]; }
    const QString& headerCloseTag()     { return tags[3]; }
            
  protected:
    QString n_ame, h_eader, tags[4];
    QBitArray flags;
    static QList<KNViewHeader> instances;
            
};

#endif
