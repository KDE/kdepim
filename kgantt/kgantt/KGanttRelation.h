#ifndef _KGANTTRELATION_H_
#define _KGANTTRELATION_H_
 
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

    file    : KGanttRelation.h
    date    : 17.02.2001

    changelog :

*/


#include <qobject.h>
#include <qdatetime.h> 
#include <qtextstream.h> 
#include <qlist.h> 
#include <qpainter.h>


class KGanttItem;


///  KGanttRelation.
/*!
 *   This class describes a item. It contains dates on which the item starts and
 *   ends. It also contains attributes that gouverns the graphical representation
 *   in a gantt diagramm.
 */
//////////////////////////////////
class KGanttRelation : public QObject
//////////////////////////////////
{

  Q_OBJECT

  friend class KGanttItem;

public:

  enum Change { 

    NoChange        = 0,
    TextChanged     = 32,

    ///  Item has been selected.
    Selected        = 2048,
    
    ///  Item has been unselected.
    Unselected      = 4096
  };
 
 


  ///   Destructor.
  /*
   *    Emits signal destroyed(KGanttRelation* this).
   */
  ~KGanttRelation();
  
 


  ///  Select/unselect item.
  /*!
   *
   */
  void select(bool f);


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



  ///  Get brush which has to be used for drawing this item as selected.
  /*!
   *
   */
  QPen& getSelectPen() {
    return _selectPen;
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
  KGanttItem* getFrom();



  ///  Get date of ending.
  /*!
   *
   */
  KGanttItem* getTo();



  ///  Dump to cout.
  /*!
   *
   */
  void dump(QTextOStream& cout, const QString& pre);


  QString ChangeAsString(Change c);


signals:

  ///  Item has changed.
  /*!
   *   This signal is emitted if any of the items
   *   properties have been changed.
   */
  void changed(KGanttRelation*, KGanttRelation::Change);



  ///  Item will be deleted.
  /*!
   *   This signal will be emitted immediately before
   *   the object will be deleted.
   */
  void destroyed(KGanttRelation*);


public slots:

  void itemDestroyed(KGanttItem* item);


protected:

 ///  Constructor.
  /*!
   * 
   */
  KGanttRelation(KGanttItem* from, KGanttItem* to,
		 const QString& text );


private:


  bool _selected;

  bool _editable;

  KGanttItem* _from;
  KGanttItem* _to;
  
  QString _text;

  QPen _pen, _textPen;
  
  static QPen _selectPen;

};

#endif
