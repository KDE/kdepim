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

#ifndef PIMSETTINGCOMMANDLINEOPTION_H
#define PIMSETTINGCOMMANDLINEOPTION_H

#include <QObject>
#include <QCommandLineParser>
#include "pimsettingexporter_export.h"

class QApplication;
class PimSettingExporterWindow;
class PIMSETTINGEXPORT_EXPORT PimSettingCommandLineOption : public QObject
{
    Q_OBJECT
public:
    explicit PimSettingCommandLineOption(QObject *parent = Q_NULLPTR);
    ~PimSettingCommandLineOption();

    void createParser(const QApplication &app);
    void setExportWindow(PimSettingExporterWindow *exporterWindow);
    void handleCommandLine();

public Q_SLOTS:
    void slotActivateRequested(const QStringList &arguments, const QString &workingDirectory);

private:
    QCommandLineParser mParser;
    PimSettingExporterWindow *mExporterWindow;
};

#endif // PIMSETTINGCOMMANDLINEOPTION_H
