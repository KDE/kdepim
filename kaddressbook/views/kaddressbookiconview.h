#ifndef KADDRESSBOOKICONVIEW_H
#define KADDRESSBOOKICONVIEW_H

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <qstring.h>
#include <kiconview.h>
#include "kaddressbookview.h"

class QIconViewItem;
class KConfig;
class AddresseeIconView;
class AddresseeIconViewItem;

namespace KABC { class AddressBook; }

/** This is an example kaddressbook view that is implemented using
* KIconView. This view is not the most useful view, but it displays
* how simple implementing a new view can be.
*/
class KAddressBookIconView : public KAddressBookView
{
  Q_OBJECT
    
  public:
    KAddressBookIconView( KABC::AddressBook *ab, QWidget *parent,
                          const char *name = 0 );
    virtual ~KAddressBookIconView();
    
    virtual QStringList selectedUids();
    virtual QString type() const { return "Icon"; }
    
    virtual void readConfig(KConfig *config);
    
    virtual void incrementalSearch(const QString &value,KABC::Field *field );
    
  public slots:
    void refresh(QString uid = QString::null);
    void setSelected(QString uid = QString::null, bool selected = true);
  
  protected slots:
    void addresseeExecuted(QIconViewItem *item);
    void addresseeSelected();
  
  private:
    AddresseeIconView *mIconView;
    QPtrList<AddresseeIconViewItem> mIconList;
};


class AddresseeIconView : public KIconView
{
  Q_OBJECT
  
  public:
    AddresseeIconView(QWidget *parent, const char *name);
    ~AddresseeIconView();
    
  signals:
    void addresseeDropped(QDropEvent *);
    void startAddresseeDrag();
  
  protected:
    virtual QDragObject *dragObject();
    
  protected slots:
    void itemDropped(QDropEvent *, const QValueList<QIconDragItem> &);
};
#endif
