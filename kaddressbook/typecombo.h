#ifndef TYPECOMBO_H
#define TYPECOMBO_H
/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>                   
                                                                        
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

#include <kabc/phonenumber.h>

#include <kcombobox.h>

/**
  Combo box for type information of Addresses and Phone numbers.
*/
class TypeCombo : public KComboBox
{
    Q_OBJECT
  public:

    class TypeList
    {
      public:
        void addType( int type, const QString &label );

        void clear();

        uint count();

        int type( int index );
        QString label( int index );

      private:
        QValueList<int> mTypes;
        QStringList mTypeLabels;
    };

    TypeCombo( QWidget *parent, const char *name = 0 );
    TypeCombo( TypeList *, QWidget *parent, const char *name = 0 );

    void setTypeList( TypeList *typeList );
    TypeList *typeList() const { return mTypeList; }

    void setLineEdit( QLineEdit *edit ) { mLineEdit = edit; }
    QLineEdit *lineEdit() const { return mLineEdit; }

    void updateTypes();

    void selectType( int type );

    int selectedType();

    KABC::PhoneNumber::List::Iterator selectedElement( KABC::PhoneNumber::List & );

  private:
    TypeList *mTypeList;
    QLineEdit *mLineEdit;
};

#endif
