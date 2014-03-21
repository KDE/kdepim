/*
  Copyright (c) 2011-2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This progam is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef TEMPLATEPARSER_TEMPLATESUTIL_H
#define TEMPLATEPARSER_TEMPLATESUTIL_H

#include "templateparser_export.h"

class QString;
class QStringList;
namespace TemplateParser {

namespace Util {

TEMPLATEPARSER_EXPORT void deleteTemplate( const QString &id );

TEMPLATEPARSER_EXPORT QStringList keywords();
TEMPLATEPARSER_EXPORT QStringList keywordsWithArgs();

}
}

#endif
