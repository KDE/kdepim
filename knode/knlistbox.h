/***************************************************************************
                     knlistbox.h - description
 copyright            : (C) 1999 by Christian Thurner
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

#ifndef KNLISTBOX_H
#define KNLISTBOX_H

#include <qlistbox.h>
#include <qpixmap.h>


class KNListBoxItem : public QListBoxItem  {
  
  public:
    KNListBoxItem(const QString& text, QPixmap *pm=0);
    ~KNListBoxItem();


  protected:
    virtual void paint(QPainter *);
    virtual int height(const QListBox *) const;
    virtual int width(const QListBox *) const;

    QPixmap *p_m;
};



#endif

