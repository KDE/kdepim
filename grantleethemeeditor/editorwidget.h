/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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
#include <KTextEdit>

class QCompleter;
namespace GrantleeThemeEditor {
class GRANTLEETHEMEEDITOR_EXPORT EditorWidget : public KTextEdit
{
    Q_OBJECT
public:
    explicit EditorWidget(QWidget *parent = 0);
    ~EditorWidget();

    void insertFile(const QString &filename);

    virtual void createCompleterList(const QStringList &extraCompletion = QStringList());

private Q_SLOTS:
    void slotInsertCompletion( const QString &completion );

protected:
    void keyPressEvent(QKeyEvent* e);

protected:
    QCompleter *m_completer;

private:
    void initCompleter();
    QString wordUnderCursor() const;
};
}

#endif // EDITORWIDGET_H
