/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>                   
                                                                        
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

#ifndef INCSEARCHWIDGET_H
#define INCSEARCHWIDGET_H

#include <qcombobox.h>
#include <qwidget.h>

#include <klineedit.h>

namespace KABC {
class Field;
}

class IncSearchWidget : public QWidget
{
  Q_OBJECT

  public:
    IncSearchWidget( QWidget *parent, const char *name = 0 );
    ~IncSearchWidget();

    void setFields( const KABC::Field::List &list );
    KABC::Field *currentField();

  signals:
    /**
      This signal is emmited whenever the text in the input
      widget is changed. You can get the sorting field by
      @ref currentField.
     */
    void doSearch( const QString& text );

    /**
      This signal is emmited whenever the search field changes.
     */
    void fieldChanged();

  private slots:
    void announceDoSearch();
    void announceFieldChanged();

  private:
    QComboBox* mFieldCombo;
    KLineEdit* mSearchText;
    KABC::Field::List mFieldList;
};

#endif
