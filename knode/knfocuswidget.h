/*
    knfocuswidget.h

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
