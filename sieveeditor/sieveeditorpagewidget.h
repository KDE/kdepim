/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef SIEVEEDITORPAGEWIDGET_H
#define SIEVEEDITORPAGEWIDGET_H
#include <QWidget>
#include <KUrl>
namespace KManageSieve {
class SieveJob;
}
namespace KSieveUi {
class SieveEditorWidget;
}

class SieveEditorPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveEditorPageWidget(QWidget *parent=0);
    ~SieveEditorPageWidget();

    void loadScript(const KUrl &url, const QStringList &capabilities);
    KUrl currentUrl() const;
    void setIsNewScript(bool isNewScript);
    void saveScript(bool showInformation = true);
    bool needToSaveScript();

    bool isModified() const;
Q_SIGNALS:
    void refreshList();
    void scriptModified(bool, SieveEditorPageWidget *);

private slots:
    void slotGetResult(KManageSieve::SieveJob *, bool success, const QString &script, bool isActive);
    void slotCheckSyntaxClicked();
    void slotPutResultDebug(KManageSieve::SieveJob *, bool success, const QString &errorMsg);
    void slotPutResult(KManageSieve::SieveJob *, bool success);
    void slotValueChanged();

private:
    void setModified(bool b);
    KUrl mCurrentURL;
    KSieveUi::SieveEditorWidget *mSieveEditorWidget;
    bool mWasActive;
    bool mIsNewScript;
    bool mWasChanged;
};

#endif // SIEVEEDITORPAGEWIDGET_H
