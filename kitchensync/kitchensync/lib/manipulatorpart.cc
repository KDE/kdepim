/*
� � This file is part of the OPIE Project
� � Copyright (c)  2002 Holger Freyther <zecke@handhelds.org>
� �                2002 Maximilian Rei� <harlekin@handhelds.org>



 �             =.            
� � � � � � �.=l.            
� � � � � �.>+-=
�_;:, � � .> � �:=|.         This library is free software; you can 
.> <`_, � > �. � <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.-- � :           the terms of the GNU Library General Public
.="- .-=="i, � � .._         License as published by the Free Software
�- . � .-<_> � � .<>         Foundation; either version 2 of the License,
� � �._= =} � � � :          or (at your option) any later version.
� � .%`+i> � � � _;_.        
� � .i_,=:_. � � �-<s.       This library is distributed in the hope that  
� � �+ �. �-:. � � � =       it will be useful,  but WITHOUT ANY WARRANTY;
� � : .. � �.:, � � . . .    without even the implied warranty of
� � =_ � � � �+ � � =;=|`    MERCHANTABILITY or FITNESS FOR A
� _.=:. � � � : � �:=>`:     PARTICULAR PURPOSE. See the GNU
..}^=.= � � � = � � � ;      Library General Public License for more
++= � -. � � .` � � .:       details.
�: � � = �...= . :.=-        
�-. � .:....=;==+<;          You should have received a copy of the GNU
� -_. . . � )=. �=           Library General Public License along with
� � -- � � � �:-=`           this library; see the file COPYING.LIB. 
                             If not, write to the Free Software Foundation,
                             Inc., 59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.

*/



#include <qobject.h>
#include <qwidget.h>

#include "manipulatorpart.h"


using namespace KitchenSync;

ManipulatorPart::ManipulatorPart(QObject *parent, const char *name )
  : KParts::Part(parent, name )
{

}


//#include "manipulatorpart.moc"
