/*                                                                      
    This file is part of KAddressBook.
    Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>
                                                                        
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

#ifndef DETAILSVIEWCONTAINER_H
#define DETAILSVIEWCONTAINER_H

#include <qptrlist.h>

#include "look_basic.h"

class QComboBox;
class QWidgetStack;

class ViewContainer : public QWidget
{
  Q_OBJECT

  public:
    ViewContainer( QWidget *parent = 0, const char* name = 0 );

    /**
      Return the look currently selected. If there is none, it
      returns zero. Do not use this pointer to store a reference
      to a look, the user might select another one (e.g., create
      a new object) at any time.
     */
    KABBasicLook *currentLook();

    /**
      Return the contact currently displayed.
     */
    KABC::Addressee addressee();

  public slots:
    /**
      Set the contact currently displayed.
     */
    void setAddressee( const KABC::Addressee& addressee );

    /**
      Set read-write state.
     */
    void setReadOnly( bool state );

  signals:
    /**
      The contact has been changed.
     */
    void addresseeChanged();

    /**
      The user acticated the email address displayed. This may happen
      by, for example, clicking on the displayed mailto-URL.
     */
    void sendEmail( const QString& );

    /**
      The user activated one of the displayed HTTP URLs. For example
      by clicking on the displayed homepage address.
     */
    void browse( const QString& );

  protected:
    /**
      A style has been selected. Overloaded from base class.
     */
    void slotStyleSelected( int );

    /**
      Register the available looks.
     */
    void registerLooks();

  private:
    KABC::Addressee mCurrentAddressee;
    KABBasicLook *mCurrentLook;
    QPtrList<KABLookFactory> mLookFactories;

    QComboBox *mStyleCombo;
    QWidgetStack *mDetailsStack;
};

#endif
