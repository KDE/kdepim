/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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
#ifndef CONTACTPRINTTHEMEPREVIEW_H
#define CONTACTPRINTTHEMEPREVIEW_H

#include <QWidget>

#include <KContacts/Addressee>
#include "grantleethemeeditor/previewwidget.h"

namespace KAddressBookGrantlee
{
class GrantleePrint;
}
class QWebView;
class ContactPrintThemePreview : public GrantleeThemeEditor::PreviewWidget
{
    Q_OBJECT
public:
    explicit ContactPrintThemePreview(const QString &projectDirectory, QWidget *parent = Q_NULLPTR);
    ~ContactPrintThemePreview();

    void updateViewer() Q_DECL_OVERRIDE;
    void createScreenShot(const QStringList &fileName) Q_DECL_OVERRIDE;
    void setThemePath(const QString &projectDirectory, const QString &mainPageFileName) Q_DECL_OVERRIDE;
    void loadConfig() Q_DECL_OVERRIDE;

Q_SIGNALS:
    void needUpdateViewer();

private:
    KContacts::Addressee mContact;
    KAddressBookGrantlee::GrantleePrint *mGrantleePrint;
    QWebView *mViewer;
};

#endif // CONTACTPRINTTHEMEPREVIEW_H
