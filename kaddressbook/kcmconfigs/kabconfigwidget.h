/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
                                                                        
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#ifndef KABCONFIGWIDGET_H
#define KABCONFIGWIDGET_H

#include <qwidget.h>

class QCheckBox;
class QLineEdit;
class QListViewItem;
class QPushButton;

class KListView;

class AddresseeWidget;

class KABConfigWidget : public QWidget
{
  Q_OBJECT
  
  public:
    KABConfigWidget( QWidget *parent, const char *name = 0 );
    
    void restoreSettings();
    void saveSettings();
    void defaults();

  signals:
    void changed( bool );

  public slots:
    void modified();

  private slots:
    void configureExtension();
    void selectionChanged( QListViewItem* );
    void itemClicked( QListViewItem* );

  private:
    void restoreExtensionSettings();
    void saveExtensionSettings();

    KListView *mExtensionView;

    QCheckBox *mNameParsing;
    QCheckBox *mViewsSingleClickBox;
    QPushButton *mConfigureButton;
    QLineEdit *mPhoneHook;

    AddresseeWidget *mAddresseeWidget;
};

#endif
