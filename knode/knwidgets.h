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

#include <tqlistbox.h>
#include <tqbitarray.h>

#include <kdockwidget.h>
#include <kprogress.h>

class QPainter;
class QPixmap;

//====================================================================================


class KNListBoxItem : public TQListBoxItem  {

  public:
    KNListBoxItem(const TQString& text, TQPixmap *pm=0);
    ~KNListBoxItem();


  protected:
    virtual void paint(TQPainter *);
    virtual int height(const TQListBox *) const;
    virtual int width(const TQListBox *) const;

    TQPixmap *p_m;
};


//====================================================================================


/** a list box which ignores Enter, useful for dialogs */
class KNDialogListBox : public QListBox
{
   public:
    // alwaysIgnore==false: enter is ignored when the widget isn't visible/out of focus
    KNDialogListBox(bool alwaysIgnore=false, TQWidget * parent=0, const char * name=0);
    ~KNDialogListBox();

  protected:
    void keyPressEvent( TQKeyEvent *e );

    bool a_lwaysIgnore;
};


//====================================================================================


class KNDockWidgetHeaderDrag : public KDockWidgetHeaderDrag
{
  Q_OBJECT

  public:
    KNDockWidgetHeaderDrag(TQWidget *focusWidget, KDockWidgetAbstractHeader* parent, KDockWidget* dock,
                            const char* name = 0);
    virtual ~KNDockWidgetHeaderDrag();

  protected slots:
    void slotFocusChanged(TQFocusEvent *e);

  protected:
    virtual void paintEvent( TQPaintEvent* );

    bool f_ocus;
};


#endif
