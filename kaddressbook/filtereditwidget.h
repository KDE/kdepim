#ifndef FILTEREDITWIDGET_H
#define FILTEREDITWIDGET_H

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

class QGroupBox;
class QToolButton;
class KListBox;

#include <qwidget.h>

#include "filter.h"

class FilterEditWidget : public QWidget
{
  Q_OBJECT
  
  public:
    FilterEditWidget(QWidget *parent, const char *name = 0);
    ~FilterEditWidget();
    
    /** Sets the dialog to an empty state
    */
    void clear();
    
    void setFilter(const Filter &f);
    const Filter &filter();
    
  protected slots:
    void availableHighlighted(int index);
    void selectedHighlighted(int index);
    void add();
    void remove();
    
  private:
    void initGUI();
    
    Filter mFilter;
    
    QToolButton *mAddButton;
    QToolButton *mRemoveButton;
    KListBox *mAvailableBox;
    KListBox *mSelectedBox;
    QButtonGroup *mMatchRuleGroup;
};

#endif
