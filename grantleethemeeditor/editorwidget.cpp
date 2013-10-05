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

#include "editorwidget.h"
#include "grantleeplaintexteditor.h"

#include <kpimtextedit/htmlhighlighter.h>

#include <QStringListModel>
#include <QCompleter>
#include <QKeyEvent>
#include <QScrollBar>
#include <QDebug>

using namespace GrantleeThemeEditor;

EditorWidget::EditorWidget(QWidget *parent)
    : PimCommon::PlainTextEditorWidget(new GrantleeThemeEditor::GrantleePlainTextEditor(), parent)
{
}

EditorWidget::~EditorWidget()
{
}

void EditorWidget::insertFile(const QString &filename)
{
    if (!filename.isEmpty()) {
        QFile file( filename );

        if ( file.open( QIODevice::ReadOnly ) ) {
            const QByteArray data = file.readAll();
            const QString str = QString::fromUtf8(data);
            editor()->insertPlainText(str);
        }
    }
}

void EditorWidget::createCompleterList(const QStringList &extraCompletion)
{
    (static_cast<GrantleeThemeEditor::GrantleePlainTextEditor *>(editor()))->createCompleterList(extraCompletion);
}

QString EditorWidget::toPlainText() const
{
    return editor()->toPlainText();
}

void EditorWidget::setPlainText(const QString &str)
{
    editor()->setPlainText(str);
}

void EditorWidget::clear()
{
    editor()->clear();
}


#include "editorwidget.moc"
