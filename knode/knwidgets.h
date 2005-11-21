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

#ifndef KNWIDGETS_H
#define KNWIDGETS_H

#include <q3listbox.h>
#include <qbitarray.h>
//Added by qt3to4:
#include <QPixmap>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QPaintEvent>

#include <k3dockwidget.h>
#include <kprogress.h>

class QPainter;
class QPixmap;

//====================================================================================


/** List box items that contain an additional pixmap. */
class KNListBoxItem : public Q3ListBoxItem  {

  public:
    KNListBoxItem(const QString& text, QPixmap *pm=0);
    ~KNListBoxItem();


  protected:
    virtual void paint(QPainter *);
    virtual int height(const Q3ListBox *) const;
    virtual int width(const Q3ListBox *) const;

    QPixmap *p_m;
};


//====================================================================================


/** a list box which ignores Enter, useful for dialogs */
class KNDialogListBox : public Q3ListBox
{
   public:
    // alwaysIgnore==false: enter is ignored when the widget isn't visible/out of focus
    KNDialogListBox( bool alwaysIgnore = false, QWidget * parent = 0 );
    ~KNDialogListBox();

  protected:
    void keyPressEvent( QKeyEvent *e );

    bool a_lwaysIgnore;
};


#endif
