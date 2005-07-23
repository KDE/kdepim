/*
    kncollection.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNCOLLECTION_H
#define KNCOLLECTION_H

#include <qstring.h>

class KNCollectionViewItem;


class KNCollection {

  public:
    enum collectionType {   CTnntpAccount, CTgroup,
                            CTfolder, CTcategory,
                            CTvirtualGroup };

    KNCollection(KNCollection *p);
    virtual ~KNCollection();

    // type
    virtual collectionType type()=0;

    // list item handling
    KNCollectionViewItem* listItem()const  { return l_istItem; }
    void setListItem(KNCollectionViewItem *i);
    virtual void updateListItem();

    // info
    virtual QString path()=0;
    virtual bool readInfo(const QString &confPath)=0;
    virtual void saveInfo()=0;

    // parent
    KNCollection* parent()const                    { return p_arent; }
    virtual void setParent(KNCollection *p)   { p_arent=p; }

    // name
    virtual const QString& name()     { return n_ame; }
    void setName(const QString &s)    { n_ame=s; }

    // count
    int count()const                       { return c_ount; }
    void setCount(int i)              { c_ount=i; }
    void incCount(int i)              { c_ount+=i; }
    void decCount(int i)              { c_ount-=i; }

  protected:
    KNCollection *p_arent;
    KNCollectionViewItem *l_istItem;
    QString n_ame;
    int c_ount;

};

#endif
