/***************************************************************************
                          knfocuswidget.h  -  description
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


#ifndef KNFOCUSWIDGET_H
#define KNFOCUSWIDGET_H

#include <qwidget.h>


class KNFocusWidget : public QWidget  {

  Q_OBJECT

  public:
    KNFocusWidget(QWidget *parent=0, const char *name=0);
    ~KNFocusWidget();
    
    void setWidget(QWidget *w);
    QWidget* widget()           { return w_idget; }
    void setFocus(bool f)       { f_ocus=f; update(); }
    bool focus()                { return f_ocus; }
    
  protected:
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);
    
    QWidget *w_idget;
    bool f_ocus;
    
  protected slots:
    void slotFocusChanged(QFocusEvent *e);
};

#endif
