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

#ifndef FANCYHEADERSTYLEPLUGIN_H
#define FANCYHEADERSTYLEPLUGIN_H

#include "header/headerstyleplugin.h"

#include <QVariant>

namespace MessageViewer
{
class FancyHeaderStylePlugin : public MessageViewer::HeaderStylePlugin
{
    Q_OBJECT
public:
    explicit FancyHeaderStylePlugin(QObject *parent = Q_NULLPTR, const QList<QVariant> & = QList<QVariant>());
    ~FancyHeaderStylePlugin();

    HeaderStyle *headerStyle() const Q_DECL_OVERRIDE;
    HeaderStrategy *headerStrategy() const Q_DECL_OVERRIDE;
    HeaderStyleInterface *createView(KActionMenu *menu, QActionGroup *actionGroup, KActionCollection *ac, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    int elidedTextSize() const Q_DECL_OVERRIDE;
private:
    HeaderStyle *mHeaderStyle;
    HeaderStrategy *mHeaderStrategy;
};
}
#endif // FANCYHEADERSTYLEPLUGIN_H
