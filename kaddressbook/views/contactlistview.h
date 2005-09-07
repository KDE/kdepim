/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef CONTACTLISTVIEW_H
#define CONTACTLISTVIEW_H

#include <qcolor.h>
#include <qpixmap.h>
#include <qtooltip.h>
#include <qstring.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QDropEvent>

#include <klistview.h>

#include <kabc/field.h>

class QDropEvent;
class KAddressBookTableView;
class ContactListView;
class KIMProxy;

/** The whole tooltip design needs a lot of work. Currently it is
* hacked together to function.
*/
class DynamicTip : public QToolTip
{
  public:
    DynamicTip( ContactListView * parent );

  protected:
    void maybeTip( const QPoint & );

  private:
};

class ContactListViewItem : public KListViewItem
{

public:
  ContactListViewItem(const KABC::Addressee &a, ContactListView* parent,
                      KABC::AddressBook *doc, const KABC::Field::List &fields, KIMProxy *proxy );
  const KABC::Addressee &addressee() const { return mAddressee; }
  virtual void refresh();
  virtual ContactListView* parent();
  virtual QString key ( int, bool ) const;
  void setHasIM( bool hasIM );
  /** Adds the border around the cell if the user wants it.
  * This is how the single line config option is implemented.
  */
  virtual void paintCell(QPainter * p, const QColorGroup & cg,
                         int column, int width, int align );

private:
  KABC::Addressee mAddressee;
  KABC::Field::List mFields;
  ContactListView *parentListView;
  KABC::AddressBook *mDocument;
  KIMProxy *mIMProxy;
  bool mHasIM;
};


/////////////////////////////////////////////
// ContactListView

class ContactListView : public KListView
{
  Q_OBJECT

public:
  ContactListView(KAddressBookTableView *view,
                  KABC::AddressBook *doc,
                  QWidget *parent,
                  const char *name = 0L );
  virtual ~ContactListView() {}
  //void resort();

  /** Returns true if tooltips should be displayed, false otherwise
  */
  bool tooltips() const { return mToolTips; }
  void setToolTipsEnabled(bool enabled) { mToolTips = enabled; }

  bool alternateBackground() const { return mABackground; }
  void setAlternateBackgroundEnabled(bool enabled);

  bool singleLine() const { return mSingleLine; }
  void setSingleLineEnabled(bool enabled) { mSingleLine = enabled; }

  const QColor &alternateColor() const { return mAlternateColor; }

  /** Sets the background pixmap to <i>filename</i>. If the
  * QString is empty (QString::isEmpty()), then the background
  * pixmap will be disabled.
  */
  void setBackgroundPixmap(const QString &filename);

  /**
   * Sets whether instant messaging presence should be shown in the first column
   */
  void setShowIM( bool enabled );

  /**
   * Is presence being shown?
   */
  bool showIM();

  /**
   * Set the column index of the column used for instant messaging presence.
   * This method is necessary because presence, unlike the other fields, is not
   * a KABC::Field, and cannot be handled using their methods.
   * TODO: make presence a KABC::Field post 3.3
   */
  void setIMColumn( int column );

  /**
   * get the column used for IM presence
   */
  int imColumn();

protected:
  /** Paints the background pixmap in the empty area. This method is needed
  * since Qt::FixedPixmap will not scroll with the list view.
  */
  virtual void paintEmptyArea( QPainter * p, const QRect & rect );
  virtual void contentsMousePressEvent(QMouseEvent*);
  void contentsMouseMoveEvent( QMouseEvent *e );
  void contentsDropEvent( QDropEvent *e );
  virtual bool acceptDrag(QDropEvent *e) const;

protected slots:
  void itemDropped(QDropEvent *e);

public slots:

signals:
  void startAddresseeDrag();
  void addresseeDropped(QDropEvent *);

private:
  KAddressBookTableView *pabWidget;
  int oldColumn;
  int column;
  bool ascending;

  bool mABackground;
  bool mSingleLine;
  bool mToolTips;
  bool mShowIM;

  QColor mAlternateColor;

  QPoint presspos;
  int mInstantMsgColumn;
};


#endif
