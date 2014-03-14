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

#ifndef SIEVEEDITORMAINWIDGET_H
#define SIEVEEDITORMAINWIDGET_H

#include <QSplitter>
#include <KUrl>
class KTabWidget;
class SieveEditorTabWidget;
class SieveEditorScriptManagerWidget;
class SieveEditorPageWidget;
class SieveEditorMainWidget : public QSplitter
{
    Q_OBJECT
public:
    explicit SieveEditorMainWidget(QWidget *parent=0);
    ~SieveEditorMainWidget();

    void createNewScript();
    void deleteScript();
    void updateServerList();
    void editScript();
    void desactivateScript();
    void refreshList();
    void saveScript();
    bool needToSaveScript();

    KTabWidget *tabWidget() const;

public Q_SLOTS:
    void slotGoToLine();

Q_SIGNALS:
    void updateButtons(bool newScriptAction, bool editScriptAction, bool deleteScriptAction, bool desactivateScriptAction);
    void updateScriptList();

private slots:
    void slotCreateScriptPage(const KUrl &url, const QStringList &capabilities, bool isNewScript);
    void slotScriptDeleted(const KUrl &url);
    void slotScriptModified(bool modified, SieveEditorPageWidget *page);
    void slotGeneralPaletteChanged();
    void slotTabCloseRequested(int index);
    void slotTabRemoveAllExclude(int index);
private:
    QWidget *hasExistingPage(const KUrl &url);
    QColor mModifiedScriptColor;
    QColor mScriptColor;
    SieveEditorTabWidget *mTabWidget;
    SieveEditorScriptManagerWidget *mScriptManagerWidget;

};

#endif // SIEVEEDITORMAINWIDGET_H
