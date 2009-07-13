/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef MESSAGECOMPOSER_UTIL_H
#define MESSAGECOMPOSER_UTIL_H

//#include "messagecomposer_export.h"

#include <QtCore/QString>

#include <kmime/kmime_headers.h>
 
namespace MessageComposer {

// TODO move to KMime?
// or move to ContentJob if we'll only need it there.
QString nameForEncoding( KMime::Headers::contentEncoding enc );

}

#endif
