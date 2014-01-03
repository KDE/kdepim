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

#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include "grantleethemeeditor_export.h"
#include "pimcommon/texteditor/plaintexteditor/plaintexteditorwidget.h"

namespace GrantleeThemeEditor {
class GRANTLEETHEMEEDITOR_EXPORT EditorWidget : public PimCommon::PlainTextEditorWidget
{
    Q_OBJECT
public:
    explicit EditorWidget(QWidget *parent = 0);
    ~EditorWidget();

    void insertFile(const QString &filename);

    virtual void createCompleterList(const QStringList &extraCompletion = QStringList());

    QString toPlainText() const;
    void setPlainText(const QString &str);
    void clear();
};
}

#endif // EDITORWIDGET_H
