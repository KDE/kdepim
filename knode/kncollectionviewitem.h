/***************************************************************************
                          kncollectionviewitem.h  -  description
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


#ifndef KNCOLLECTIONVIEWITEM_H
#define KNCOLLECTIONVIEWITEM_H

#include "knlistview.h"

class KNCollection;


class KNCollectionViewItem : public KNLVItemBase  {
  
  public:
    KNCollectionViewItem(KNListView *vi);
    KNCollectionViewItem(KNLVItemBase *it);
    ~KNCollectionViewItem();

    void setNumber ( int column, int number );  // avoid converting back and forth in key()
    QString key(int, bool) const;
    
    KNCollection *coll;
    
  protected:
    bool firstColBold();
    int num[3];
};


#endif
