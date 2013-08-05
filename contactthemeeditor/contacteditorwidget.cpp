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

#include "contacteditorwidget.h"
#include "defaultcompletion.h"
#include <QStringListModel>
#include <QCompleter>
#include <QDebug>

ContactEditorWidget::ContactEditorWidget(QWidget *parent)
    : GrantleeThemeEditor::EditorWidget(parent)
{
    createCompleterList();
}

ContactEditorWidget::~ContactEditorWidget()
{
}

void ContactEditorWidget::createCompleterList(const QStringList &extraCompletion)
{
    QStringList listWord;
    listWord << DefaultCompletion::defaultCompetion();
    listWord << DefaultCompletion::defaultOptions();
    listWord << extraCompletion;
    m_completer->setModel( new QStringListModel( listWord, m_completer ) );
}

#include "contacteditorwidget.moc"
