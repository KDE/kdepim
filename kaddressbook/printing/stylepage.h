/*                                                                      
    This file is part of KAddressBook.
    Copyright (c) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>
                       Tobias Koenig <tokoe@kde.org>
                                                                        
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

#ifndef STYLEPAGE_H
#define STYLEPAGE_H

#include <qwidget.h>

#include <kabc/addressbook.h>
#include <kabc/field.h>

class QLabel;
class QPixmap;
class QRadioButton;
class KComboBox;

class StylePage : public QWidget
{
  Q_OBJECT

  public:
    StylePage( KABC::AddressBook *ab, QWidget* parent = 0, const char* name = 0 );
    ~StylePage();

    /**
     * Set a preview image. If @ref pixmap is 'null' a text will
     * be displayed instead.
     */
    void setPreview( const QPixmap &pixmap );

    /**
     * Add a style name.
     */
    void addStyleName( const QString &name );

    /**
     * Clear the style name list.
     */
    void clearStyleNames();

    /**
     * Set the sort criterion field.
     */
    void setSortField( KABC::Field *field );

    /**
     * Returns the sort criterion field.
     */
     KABC::Field* sortField();
    
    /**
     * Set the sort type.
     */
    void setSortAscending( bool value = true );

    /**
     * Returns whether the sort type is ascending.
     */
    bool sortAscending();

  signals:
    /**
     * This signal is emmited when the user selects a new style in the
     * style combo box.
     */
    void styleChanged( int index );

  private:
    void initGUI();
    void initFieldCombo();

    KComboBox *mFieldCombo;
    KComboBox *mSortTypeCombo;
    KComboBox *mStyleCombo;
    QLabel *mPreview;

    KABC::AddressBook *mAddressBook;
    KABC::Field::List mFields;
};

#endif // STYLEPAGE_H
