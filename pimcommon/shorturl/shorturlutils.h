/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef SHORTURLUTILS_H
#define SHORTURLUTILS_H

#include <QObject>

namespace PimCommon {
class AbstractShortUrl;
namespace ShortUrlUtils
{
    enum EngineType {
        Google = 0,
        //TODO add more engine.
        EndListEngine
    };
    QString stringFromEngineType(EngineType type);
    AbstractShortUrl *loadEngine(QObject *parent);
}
}

#endif // SHORTURLUTILS_H
