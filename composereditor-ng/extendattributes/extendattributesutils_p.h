/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#ifndef EXTENDATTRIBUTESUTIL_P_H
#define EXTENDATTRIBUTESUTIL_P_H
#include "extendattributesdialog.h"

namespace ComposerEditorNG
{
namespace ExtendAttributesUtils {
QMap<QString, QStringList> attributesMap(ExtendAttributesDialog::ExtendType type);
QMap<QString, QStringList> attributesMapImage();
QMap<QString, QStringList> attributesMapTable();
QMap<QString, QStringList> attributesMapCell();
QMap<QString, QStringList> attributesMapLink();
//List
QMap<QString, QStringList> listGlobalAttribute();
QMap<QString, QStringList> attributesMapListUL();
QMap<QString, QStringList> attributesMapListOL();
QMap<QString, QStringList> attributesMapListDL();

QMap<QString, QStringList> globalAttribute();
QMap<QString, QStringList> attributesJavascript();
QMap<QString, QStringList> attributesJavascriptWindowAndBase();
QMap<QString, QStringList> attributesMapBody();
}
}
#endif // EXTENDATTRIBUTESUTIL_P_H
