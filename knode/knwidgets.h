/*
    knwidgets.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
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

#include <qlistbox.h>
#include <qbitarray.h>

#include <kdockwidget.h>
#include <kprogress.h>

class QPainter;
class QPixmap;

//====================================================================================


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


//====================================================================================


/** a list box which ignores Enter, useful for dialogs */
class KNDialogListBox : public QListBox
{
   public:
    // alwaysIgnore==false: enter is ignored when the widget isn't visible/out of focus
    KNDialogListBox(bool alwaysIgnore=false, QWidget * parent=0, const char * name=0);
    ~KNDialogListBox();

  protected:
    void keyPressEvent( QKeyEvent *e );

    bool a_lwaysIgnore;
};


//====================================================================================


class KNDockWidgetHeaderDrag : public KDockWidgetHeaderDrag
{
  Q_OBJECT

  public:
    KNDockWidgetHeaderDrag(QWidget *focusWidget, KDockWidgetAbstractHeader* parent, KDockWidget* dock,
                            const char* name = 0);
    virtual ~KNDockWidgetHeaderDrag();

  protected slots:
    void slotFocusChanged(QFocusEvent *e);

  protected:
    virtual void paintEvent( QPaintEvent* );

    bool f_ocus;
};


#endif
