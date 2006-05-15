/*
    This file is part of libkolabformat - the library implementing the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004  Bo Thorsen <bo@sonofthor.dk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <note.h>


int main( int /*argc*/, char** /*argv*/ )
{
  KolabFormat::Note note;
  note.setBody( "Bla\nBla\nBla" );
  note.setSummary( "My summary" );

  QString xml = note.save();

  qDebug( "XML:\n%s\n", xml.utf8().data() );

  qDebug( "\nLoading:" );
  KolabFormat::Note n2;
  n2.load( xml );

  qDebug( "XML2:\n%s\n", n2.save().utf8().data() );
  return 0;
}
