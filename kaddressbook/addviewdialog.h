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

#ifndef ADDVIEWDIALOG_H
#define ADDVIEWDIALOG_H

#include <kdialogbase.h>
#include <qdict.h>
#include <qstring.h>

class QButtonGroup;
class QLineEdit;
class ViewWrapper;


/**
  Modal dialog used for adding a new view. The dialog asks for the name of
  the view as well as the type. Someday it would be nice for this to be a
  wizard.
 */
class AddViewDialog : public KDialogBase
{
  Q_OBJECT
    
  public:
    AddViewDialog( QDict<ViewWrapper> *viewWrapperDict, QWidget *parent,
                   const char *name = 0 );
    ~AddViewDialog();
    
    QString viewName();
    
    QString viewType();
    
  protected slots:
    /**
      Called when the user selects a type radio button.
     */
    void clicked( int id );
    
    /**
      Called when the user changes the text in the name of the view.
     */
    void textChanged( const QString &text );
    
  private:
    QDict<ViewWrapper> *mViewWrapperDict;
    QLineEdit *mViewNameEdit;
    QButtonGroup *mTypeGroup;
    
    int mTypeId;
};

#endif
