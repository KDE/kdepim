/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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


#ifndef PRINTINGSETTINGS_H
#define PRINTINGSETTINGS_H

#include "messageviewer_export.h"
#include <QWidget>

class Ui_PrintingSettings;
namespace MessageViewer {

class MESSAGEVIEWER_EXPORT PrintingSettings : public QWidget
{
    Q_OBJECT
public:
    explicit PrintingSettings(QWidget *parent=0);
    ~PrintingSettings();

    void save();
    void doLoadFromGlobalSettings();
    void doResetToDefaultsOther();

Q_SIGNALS:
    void changed();

private:
    Ui_PrintingSettings *mPrintingUi;
};
}
#endif // PRINTINGSETTINGS_H
