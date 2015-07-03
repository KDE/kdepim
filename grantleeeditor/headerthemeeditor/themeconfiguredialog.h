/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef THEMECONFIGUREDIALOG_H
#define THEMECONFIGUREDIALOG_H

#include <QDialog>

namespace PimCommon
{
class PlainTextEditorWidget;
}
namespace GrantleeThemeEditor
{
class ConfigureWidget;
}

class ThemeConfigureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ThemeConfigureDialog(QWidget *parent = Q_NULLPTR);
    ~ThemeConfigureDialog();

    void readConfig();
    void writeConfig();

private Q_SLOTS:
    void slotOkClicked();
    void slotDefaultClicked();

private:
    GrantleeThemeEditor::ConfigureWidget *mConfigureWidget;
    PimCommon::PlainTextEditorWidget *mDefaultTemplate;
    PimCommon::PlainTextEditorWidget *mDefaultEmail;
};

#endif // THEMECONFIGUREDIALOG_H
