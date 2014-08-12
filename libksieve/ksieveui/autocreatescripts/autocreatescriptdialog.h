/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef AUTOCREATESCRIPTDIALOG_H
#define AUTOCREATESCRIPTDIALOG_H

#include "ksieveui_export.h"

#include <QDialog>

class QDomDocument;

namespace KSieveUi {
class SieveEditorGraphicalModeWidget;
class KSIEVEUI_EXPORT AutoCreateScriptDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AutoCreateScriptDialog(QWidget *parent = 0);
    ~AutoCreateScriptDialog();

    QString script(QString &requires) const;
    void setSieveCapabilities(const QStringList &capabilities);

    void loadScript(const QDomDocument &doc, QString &error);

protected:
    bool event(QEvent *e);

private:
    void readConfig();
    void writeConfig();

private:
    SieveEditorGraphicalModeWidget *mEditor;
};
}

#endif // AUTOCREATESCRIPTDIALOG_H
