/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef UR1CASHORTURLENGINEPLUGIN_H
#define UR1CASHORTURLENGINEPLUGIN_H

#include "../shorturlengineplugin.h"

#include <QVariant>

namespace PimCommon
{
class Ur1CaShortUrlEnginePlugin : public PimCommon::ShortUrlEnginePlugin
{
    Q_OBJECT
public:
    explicit Ur1CaShortUrlEnginePlugin(QObject *parent = Q_NULLPTR, const QList<QVariant> & = QList<QVariant>());
    ~Ur1CaShortUrlEnginePlugin();

    ShortUrlEngineInterface *createInterface(QObject *parent) Q_DECL_OVERRIDE;
    QString engineName() const Q_DECL_OVERRIDE;
};
}

#endif // UR1CASHORTURLENGINEPLUGIN_H
