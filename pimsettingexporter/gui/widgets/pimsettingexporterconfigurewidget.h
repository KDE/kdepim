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

#ifndef PIMSETTINGEXPORTERCONFIGUREWIDGET_H
#define PIMSETTINGEXPORTERCONFIGUREWIDGET_H

#include <QWidget>
class QCheckBox;
class PimSettingExporterConfigureWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PimSettingExporterConfigureWidget(QWidget *parent = Q_NULLPTR);
    ~PimSettingExporterConfigureWidget();

    void save();

public Q_SLOTS:
    void resetToDefault();

private:
    void initialize();
    QCheckBox *mAlwaysOverrideFile;
    QCheckBox *mAlwaysOverrideDirectory;
    QCheckBox *mAlwaysMergeConfigFile;
};

#endif // PIMSETTINGEXPORTERCONFIGUREWIDGET_H
