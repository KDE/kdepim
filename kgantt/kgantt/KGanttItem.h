#ifndef _KGANTTITEM_H_
#define _KGANTTITEM_H_
 
/*

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    author  : jh, jochen@ifb.bv.tu-berlin.de

    file    : KGanttItem.h
    date    : 26 oct 2000


    changelog : 26 dec 2000, jh
                09 jan 2001, jh - added signal destroyed(xQTask*)

		11 jan 2001, jh changed to kde : xQTask -> KGanttItem

*/


#include <qobject.h>
#include <qdatetime.h> 
#include <qtextstream.h> 
#include <qptrlist.h> 
#include <qpainter.h>

#include <kdepimmacros.h>

#include "KGanttRelation.h"



///  KGanttItem.
/*!
 *   This class describes a item. It contains dates on which the item starts and
 *   ends. It also contains attributes that gouverns the graphical representation
 *   in a gantt diagramm.
 */
//////////////////////////////////
class KDE_EXPORT KGanttItem : public QObject
//////////////////////////////////
{

  Q_OBJECT


public:
  
  enum Change { 
    NoChange        = 0,
    StartChanged    = 1,
    EndChanged      = 2,
    
    ///  Height for this item has changed. The
    ///  height doesn't include the subitems.
    HeightChanged   = 4,
    
    ///  Total height has changed. This 
    ///  happens if the item was opened, closed
    ///  or subitems has been added or removed while
    ///  item is open.
    TotalHeightChanged = 8,
    
    ///  Style for drawing has changed.
    StyleChanged    = 16,
    TextChanged     = 32,
    ModeChanged     = 64,
    MinChanged      = 128,
    MaxChanged      = 256,
    
    /// Draw item including subitems.
    Opened          = 512,
    
    /// Draw item without subitems.
    Closed          = 1024,
    
    ///  Item has been selected.
    Selected        = 2048,
    
    ///  Item has been unselected.
    Unselected      = 4096,
    
    ///  Changes may occurred but the types are unknown
    Unknown         = 8192,

    ///  Relation between two subitems has been added
    RelationAdded   = 16384,

    /// Relation between two subitems has been removed
    RelationRemoved = 32768

  };



  enum Style {
    /// Set item invisible
    DrawNothing     = 0,
    
    /// Draw border.
    DrawBorder      = 1, 
    
    //  Fill item with brush.
    DrawFilled      = 2, 
    DrawText        = 4,
    
    //  Draw handlke for opening/closing item.
    DrawHandle      = 16,
    
    /// Draw handle only if item contains subitems
    DrawHandleWSubitems = 32,
    
    DrawAll         = 255 
  };


  enum Mode { 
    Normal, 
    Rubberband 
  };


  ///  Constructor.
  /*!
   * 
   */
  KGanttItem(KGanttItem* parentItem, const QString& text, 
	 const QDateTime& start, const QDateTime& end);



  ///  Constructor.
  /*!
   * 
   */
  KGanttItem(KGanttItem* parentItem, const QString& text, 
	 const QDateTime& start, long durationMin);



  ///   Destructor.
  /*
   *    Emits signal destroyed(KGanttItem* this).
   */
  ~KGanttItem();



  ///   Add relation between two subitems.
  /*
   *
   */
  KGanttRelation* addRelation(KGanttItem* from, KGanttItem* to,
			      const QString& text);



 
  ///  Returns true if item is open (subitems has to be drawn)
  /*!
   *
   */
  bool isOpen() {
    return _open;
  }



  ///  Open / Close item
  /*!
   *   Draw/don't draw subitems.
   */
  void open(bool f);



  ///  Set item editable or not.
  /*!
   *   If item is not editable these methods have no effect :
   *   setStart(), setEnd(), setText(), select(), setMode(), setStyle(), 
   *   setHeight(),
   *   
   */
  void setEditable(bool f) {
    _editable = f;
  }



  ///  Returns if item is editable.
  /*!
   *   See also setEditable().
   */
  bool isEditable() {
    return _editable;
  }



  ///  Returns true if item is selected.
  /*!
   *
   */
  bool isSelected() {
    return _selected;
  }



  ///  Select/unselect item.
  /*!
   *
   */
  void select(bool f);



  ///  Set mode.
  /*!
   *   If mode is 'Rubberband' and the number of subtaks is greater than 0,
   *   the start and end of the item is determined by the start and end of the
   *   earliest/latest subitem. <br>
   *   Default is 'Normal'.
   */
  void setMode(Mode flag);



  ///  Set drawing style.
  /*!  
   *   
   */
  void setStyle(int flag, bool includeSubitems = false);



  ///  Get drawing style.
  /*!
   *
   */
  int getStyle() {
    return _style;
  }



  ///  Set brush for filling
  /*!
   *
   */
  void setBrush(const QBrush& brush);



  ///  Get brush that is used for filling the item.
  /*!
   *
   */
  QBrush& getBrush() {
    return _brush;
  }



  ///  Get brush which has to be used for drawing this item as selected.
  /*!
   *
   */
  QBrush& getSelectBrush() {
    return _selectBrush;
  }



  ///  Set pen for border.
  /*!
   *
   */
  void setPen(const QPen& pen);



  ///
  /*!
   *
   */
  QPen& getPen() {
    return _pen;
  }



  ///
  /*!
   *
   */
  void setTextPen(const QPen& pen) {
    _textPen = pen;
  }



  ///
  /*!
   *
   */
  QPen& getTextPen() {
    return _textPen;
  }



  ///  Set text.
  /*!
   *
   */
  void setText(const QString& text);



  ///  Get text.
  /*!
   *
   */
  QString getText() { return _text; }
  


  ///  Get date of starting.
  /*!
   *   If mode == ´Rubberband´ and this item contains
   *   subitems, start of the item is determined by the start of the
   *   earliest subitem. <br>
   */
  QDateTime getStart();



  ///  Get date of ending.
  /*!
   *
   */
  QDateTime getEnd();



  ///  Set time/date of start.
  /*!
   *
   */
  void setStart(const QDateTime& start);



  ///  Set time/date of end.
  /*!
   *
   */
  void setEnd(const QDateTime& end);



  ///  Set height.
  /*!
   *   Set height in pixel. These are scaled when this item is drawn
   *   by the barview.
   */
  void setHeight(int h);



  ///  Get height.
  /*!
   *  Returns the height in pixel of this item. This does not include the height 
   *  of any subitems; getTotalHeight() returns that if the subitems have
   *  to be drawn.
   */
  int getHeight() {
    return _height;
  }



  ///  Get total height.
  /*!
   *   Returns the total height of this object in pixel, including any 
   *   visible subitems. Notice, that the pixels are no screen pixel since
   *   the barview scales the height of a item.
   */
  int getTotalHeight();

  

  ///  Get width in minutes.
  /*!
   *   
   */
  int getWidth();



  ///  Get list of subitems.
  /*!
   *
   */
  QPtrList<KGanttItem>& getSubItems() {
    return _subitems;
  }



  ///  Get list of relations.
  /*!
   *
   */
  QPtrList<KGanttRelation>& getRelations() {
    return _relations;
  }



  ///  Start a transaction.
  /*!
   *   If you want to add a lot of subitems -> block signals
   */
  void startTransaction(){
    blockSignals(true);
  }



  ///  End a transaction.
  /*!
   *   If you started a transaction and all signals have been blocked
   *   by method startTransaction(), invoke endTransaction() to unblock signals.<br>
   *   Signal changed(this,Unknown) is emitted.
   */
  void endTransaction();



  ///  Return a given change as a string.
  /*!
   *
   */
  static QString ChangeAsString(Change c);




  ///  Dump to cout.
  /*!
   *
   */
  void dump(QTextOStream& cout, const QString& pre);


signals:

  ///  Item has changed.
  /*!
   *   This signal is emitted if any of the items
   *   properties have been changed.
   */
  void changed(KGanttItem*, KGanttItem::Change);



  ///  Item will be deleted.
  /*!
   *   This signal will be emitted immediately before
   *   the object will be deleted.
   */
  void destroyed(KGanttItem*);



private slots:
 
  void subItemChanged(KGanttItem*, KGanttItem::Change);

  void removeRelation(KGanttRelation* rel);


private:

  void registerItem(KGanttItem* item);
  void unregisterItem(KGanttItem* item);

  void init(KGanttItem* parentItem, const QString& text,
	    const QDateTime& start, const QDateTime& end);


  //  set min/max date and time according to subitems
  Change adjustMinMax();

  /*  if min < start set start to _min,
      if max > end set end to max */      
  Change adjustStartEnd();


  // is item open/closed
  bool _open;
  bool _selected;


  // is this item editable by the user, if it is false, invoking
  // of some methods has no effect
  bool _editable;

  int _height, _style, _mode;


  KGanttItem*            _parentItem;
  QPtrList<KGanttItem>      _subitems;  
  QPtrList<KGanttRelation>  _relations;


  // start/end date. 
  // start must always be earlier then _minDateTime
  // end must always be later then _maxDateTime
  QDateTime _start, _end, _minDateTime, _maxDateTime;
  
  QString _text;

  QBrush _brush;
  QPen _pen, _textPen;

  static QBrush _selectBrush;
 

};

#endif
