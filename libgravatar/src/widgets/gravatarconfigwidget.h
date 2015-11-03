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

#ifndef GRAVATARCONFIGWIDGET_H
#define GRAVATARCONFIGWIDGET_H

#include <QWidget>
#include "gravatar_export.h"
namespace Gravatar
{
class GravatarConfigWidgetPrivate;
class GRAVATAR_EXPORT GravatarConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GravatarConfigWidget(QWidget *parent = Q_NULLPTR);
    ~GravatarConfigWidget();

    void save();
    void doLoadFromGlobalSettings();
    void doResetToDefaultsOther();

Q_SIGNALS:
    void configChanged(bool);

private Q_SLOTS:
    void slotGravatarEnableChanged(bool state);
    void slotConfigureSettings();

private:
    void updateWidgetState(bool state);
    GravatarConfigWidgetPrivate *const d;
};
}

#endif // GRAVATARCONFIGWIDGET_H
