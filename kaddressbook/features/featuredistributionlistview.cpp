/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Mirko Boehm <mirko@kde.org>
                                                                        
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

#include <qdragobject.h>

#include "featuredistributionlistview.h"

FeatureDistributionListView::FeatureDistributionListView(QWidget *parent,
                                                         const char* name)
    : KListView(parent, name)
{
  setDragEnabled( true );
  setAcceptDrops( true );
  setAllColumnsShowFocus( true );
}

void FeatureDistributionListView::dragEnterEvent( QDragEnterEvent* e )
{
  bool canDecode = QTextDrag::canDecode( e );
  e->accept( canDecode );
}

void FeatureDistributionListView::viewportDragMoveEvent( QDragMoveEvent *e )
{
  bool canDecode = QTextDrag::canDecode( e );
  e->accept( canDecode );
}

void FeatureDistributionListView::viewportDropEvent( QDropEvent *e )
{
  emit dropped( e );
}

void FeatureDistributionListView::dropEvent( QDropEvent *e )
{
  emit dropped( e );
}

#include "featuredistributionlistview.moc"
