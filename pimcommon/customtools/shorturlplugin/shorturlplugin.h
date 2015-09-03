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

#ifndef SHORTURLPLUGIN_H
#define SHORTURLPLUGIN_H
#include "customtools/customtoolsplugin.h"

#include <QVariant>
namespace PimCommon
{
class ShortUrlWidget;
class ShorturlPlugin : public PimCommon::CustomToolsPlugin
{
    Q_OBJECT
public:
    explicit ShorturlPlugin(QObject *parent = Q_NULLPTR, const QList<QVariant> & = QList<QVariant>());
    ~ShorturlPlugin();

    void createAction() Q_DECL_OVERRIDE;
    KToggleAction *action() const Q_DECL_OVERRIDE;
    QWidget *createView(QWidget *parent) Q_DECL_OVERRIDE;
    QString customToolName() const Q_DECL_OVERRIDE;
    void setShortcut(KActionCollection *ac) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotActivateShorturl(bool b);

private:
    KToggleAction *mAction;
    ShortUrlWidget *mShortUrlWidget;
};
}
#endif // SHORTURLPLUGIN_H
