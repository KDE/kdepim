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

#include "messageviewer_export.h"

#include <QWidget>
class Ui_Settings;

class KConfigDialogManager;

namespace MessageViewer {

/**
 * Configure widget that can be used in a KConfigDialog.
 *
 * @author Andras Mantia <andras@kdab.net>
 */
class MESSAGEVIEWER_EXPORT ConfigureWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigureWidget( QWidget *parent = 0 );

    ~ConfigureWidget();

    //
    // Read and write config settings to the GlobalSettings.
    // Note that this does not deal with all settings, some of those settings need to saved and read
    // with a KConfigDialogManager, since this widgets correctly sets the objectname to the pattern
    // required by KConfigDialogManager.
    //

    void writeConfig();
    void readConfig();

signals:

    /**
   * Emitted when the user changes the setting in some widget. Useful to enable the "Apply"
   * button after this has been emitted.
   */
    void settingsChanged();

private Q_SLOTS:
    void showCustomHeadersDialog();

private:
    void readCurrentFallbackCodec();
    void readCurrentOverrideCodec();

    Ui_Settings *mSettingsUi;
};

}

#endif
