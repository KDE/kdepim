/* -*- c++ -*-

  CRYPTPLUG - an independent cryptography plug-in
  API. CryptPlugWrapperList holds any number of crypto plug-ins.

  Copyright (C) 2001 by Klarälvdalens Datakonsult AB

  CRYPTPLUG is free software; you can redistribute it and/or modify
  it under the terms of GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  CRYPTPLUG is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "cryptplugwrapperlist.h"

CryptPlugWrapper * CryptPlugWrapperList::findForLibName( const QString & libName ) const
{
  for ( Q3PtrListIterator<CryptPlugWrapper> it( *this ) ; it.current() ; ++it )
    if ( (*it)->libName().find( libName, 0, false ) >= 0 )
      return *it;
  return 0;
}
