/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#ifndef LISTHELPER_H
#define LISTHELPER_H

#include <QWebElement>
#include "extendattributes/extendattributesdialog.h"

namespace ComposerEditorNG
{

namespace ListHelper
{
QWebElement ulElement(const QWebElement &element);
QWebElement olElement(const QWebElement &element);
QWebElement dlElement(const QWebElement &element);
QWebElement listElement(const QWebElement &element);
ExtendAttributesDialog::ExtendType listType(const QWebElement &element);
}
}

#endif // LISTHELPER_H
