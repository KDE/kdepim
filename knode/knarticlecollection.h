/*
    knarticlecollection.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNARTICLECOLLECTION_H
#define KNARTICLECOLLECTION_H

#include <qfile.h>

#include "kncollection.h"

class KNArticle;

class KNArticleCollection : public KNCollection {

  public:
    KNArticleCollection(KNCollection *p=0);
    ~KNArticleCollection();
    
    bool resize(int s=0);
    bool append(KNArticle *a);
    void clearList();
    void compactList();
    
    //get   
    bool isEmpty()                { return ( (list==0) || (len==0) ); }
    bool isFilled()               { return (!(c_ount>0 && len==0)); }
    int size()                    { return siz; }
    int length()                  { return len; }
    int increment()               { return incr; }
    unsigned int lockedArticles() { return l_ockedArticles; }
    
    //set
    void setIncrement(int i)      { incr=i; }
    void setLastID();
    void articleLocked()          { l_ockedArticles++; }
    void articleUnlocked()        { l_ockedArticles--; }
          
  protected:
    int findId(int id);
    int siz, len, lastID, incr;
    unsigned int l_ockedArticles;
    KNArticle **list;
};


//==============================================================================


class KNFile : public QFile {

  public:
    KNFile(const QString& fname=QString::null);
    ~KNFile();
    const QCString& readLine();
    const QCString& readLineWnewLine();

   protected:
    bool increaseBuffer();

    QCString buffer;
    char *dataPtr;
    int filePos, readBytes;
};

#endif
