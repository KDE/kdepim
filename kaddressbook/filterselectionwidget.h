#ifndef FILTERSELECTIONWIDGET_H
#define FILTERSELECTIONWIDGET_H

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

#include <qhbox.h>

class KComboBox;

/** A simple widget which consists of a label and a combo box in a 
* horizontal line. The combo box allows the user to select the active
* filter.
*/
class FilterSelectionWidget : public QHBox
{
  Q_OBJECT
  
  public:
    FilterSelectionWidget(QWidget *parent, const char *name = 0);
    ~FilterSelectionWidget();
    
    void setFilterNames(const QStringList &names);
    
    int currentItem() const;
    void setCurrentItem(int index);
    
    QString text(int index) const;
    unsigned int count() const;
    
    QString currentFilterName() const;
    void setCurrentFilterName(const QString &name);
    
  signals:
    void filterActivated(int);
    
  protected slots:
    void activated(int);
    
  private:
    KComboBox *mFilterCombo;
};
#endif
