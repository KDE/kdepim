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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#ifndef LOOK_HTML_H
#define LOOK_HTML_H

#include <klocale.h>

#include "look_basic.h"

class QColor;

namespace KABC { class Addressee; }
class KTextBrowser;

class KABHtmlView : public KABBasicLook
{
  Q_OBJECT

  public:
    /**
      The constructor.
     */
    KABHtmlView( QWidget *parent = 0, const char* name = 0 );

    /**
      The virtual destructor.
     */
    virtual ~KABHtmlView();

    /**
      Set the addressee.
     */
    void setAddressee( const KABC::Addressee& );

  private:
    KTextBrowser *mTextBrowser;
    QColor mBaseColor;
    QColor mTextColor;
};

class KABHtmlViewFactory : public KABLookFactory
{
  public:
    KABHtmlViewFactory( QWidget *parent = 0, const char *name = 0 )
      : KABLookFactory( parent, name ) {}

    KABBasicLook *create()
    {
      return new KABHtmlView( mParent, mName );
    }

    QString description()
    {
      return i18n( "HTML table style." );
    }
};

#endif
