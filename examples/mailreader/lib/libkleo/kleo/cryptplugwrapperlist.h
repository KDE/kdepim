/* -*- Mode: C++ -*-

  CRYPTPLUG - an independent cryptography plug-in
  API. CryptPlugWrapperList holds any number of crypto plug-ins.

  Copyright (C) 2001,2004 by Klarälvdalens Datakonsult AB

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

#ifndef __CRYPTPLUGWRAPPERLIST_H__
#define __CRYPTPLUGWRAPPERLIST_H__

#include "kleo/cryptplugwrapper.h"

#include <Qt3Support/Q3PtrList>

class CryptPlugWrapperList : public Q3PtrList<CryptPlugWrapper>
{
public:
    CryptPlugWrapper * findForLibName( const QString & libName ) const;
};

typedef Q3PtrListIterator<CryptPlugWrapper> CryptPlugWrapperListIterator;

#endif
