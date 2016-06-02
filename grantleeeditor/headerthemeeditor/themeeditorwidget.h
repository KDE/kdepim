/*
   Copyright (C) 2013-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef THEMEEDITORWIDGET_H
#define THEMEEDITORWIDGET_H
#include "grantleethemeeditor/editorwidget.h"

class ThemeEditorWidget : public GrantleeThemeEditor::EditorWidget
{
    Q_OBJECT
public:
    explicit ThemeEditorWidget(QWidget *parent = Q_NULLPTR);
    ~ThemeEditorWidget();

public Q_SLOTS:
    void createCompleterList(const QStringList &extraCompletion = QStringList()) Q_DECL_OVERRIDE;
};

#endif // THEMEEDITORWIDGET_H
