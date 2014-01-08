/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#ifndef SIEVEEDITORSCRIPTMANAGERWIDGET_H
#define SIEVEEDITORSCRIPTMANAGERWIDGET_H

#include <QWidget>
#include <KUrl>

class SieveEditorManageSieveWidget;
class QTreeWidgetItem;
class SieveEditorScriptManagerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveEditorScriptManagerWidget(QWidget *parent=0);
    ~SieveEditorScriptManagerWidget();

    void updateServerList();
    void editScript();
    void desactivateScript();

Q_SIGNALS:
    void createScriptPage(const KUrl &url, const QStringList &capabilities, bool isNewScript);
    void updateButtons(bool newScriptAction, bool editScriptAction, bool deleteScriptAction, bool desactivateScriptAction);

public Q_SLOTS:
    void slotCreateNewScript();
    void slotDeleteScript();

private Q_SLOTS:
    void slotNewScript(const KUrl &url, const QStringList &capabilities);
    void slotEditScript(const KUrl &url, const QStringList &capabilities);
    void slotUpdateButtons(QTreeWidgetItem *item);

private:
    SieveEditorManageSieveWidget *mTreeView;
};

#endif // SIEVEEDITORSCRIPTMANAGERWIDGET_H
