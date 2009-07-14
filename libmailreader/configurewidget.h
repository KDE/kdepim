/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef MAILVIEWERCONFIGUREWIDGET_H
#define MAILVIEWERCONFIGUREWIDGET_H

#include "mailviewer_export.h"

#include <QWidget>
class Ui_Settings;

namespace MailViewer {

/**
Configure widget that can be used in a KConfigDialog.

	@author Andras Mantia <andras@kdab.net>
*/
class MAILVIEWER_EXPORT ConfigureWidget : public QWidget
{
Q_OBJECT
public:
    ConfigureWidget(QWidget *parent = 0);

    ~ConfigureWidget();

private slots:
  void slotSettingsChanged();

signals:
  void settingsChanged();

private:
  void readCurrentFallbackCodec();
  void readCurrentOverrideCodec();

  Ui_Settings *mSettingsUi;

};

}

#endif
