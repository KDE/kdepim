/*                                                                      
    This file is part of KAddressBook.
    Copyright (c) 2002 Jost Schenck <jost@schenck.de>
                                                                        
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

#ifndef RINGBINDERSTYLE_H
#define RINGBINDERSTYLE_H

#include <kabc/addressee.h>
#include "printstyle.h"

class KPrinter;
class RingBinderStyleAppearanceForm;

namespace KABPrinting
{

class RingBinderPrintStyle : public PrintStyle
{
  Q_OBJECT

  public:
    RingBinderPrintStyle( PrintingWizard* parent, const char* name = 0 );
    ~RingBinderPrintStyle();

    void print( const KABC::Addressee::List &contacts, PrintProgress* );

  protected:
    bool printEntries( const KABC::Addressee::List &contacts, KPrinter *printer, 
                       QPainter *painter, const QRect& window );
    void fillEmpty( const QRect& window, KPrinter *printer, QPainter* painter, 
                    int top, int grpnum );
    bool printEntry( const KABC::Addressee& contact, const QRect& window, 
                     QPainter *painter, int top, bool fake = false, 
                     QRect* brect = 0 );
    QRect entryMetrics( const KABC::Addressee& contact, const QRect& window, 
                        QPainter* painter, int top );
    bool printEmptyEntry( const QRect& window, QPainter* painter, int top );
    QRect emptyEntryMetrics( const QRect& window, QPainter* painter, int top );
    bool printPageHeader( const QString section, const QRect& window, 
                          QPainter* painter );
    QRect pageHeaderMetrics( const QRect& window, QPainter* painter );

  private:
    RingBinderStyleAppearanceForm *mPageAppearance;
    PrintProgress *mPrintProgress;
};

class RingBinderPrintStyleFactory : public PrintStyleFactory
{
  public:
    RingBinderPrintStyleFactory( PrintingWizard *parent, const char *name = 0 );

    PrintStyle *create() const;
    QString description() const;
};

}

#endif
// vim:tw=78 cin et sw=2 comments=sr\:/*,mb\:\ ,ex\:*/,\://
