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
#ifndef DESKTOPFILEPAGE_H
#define DESKTOPFILEPAGE_H

#include <QWidget>
#include "grantleethemeeditor_export.h"
class QLineEdit;
class KZip;
class KEditListWidget;
namespace KPIMTextEdit
{
class PlainTextEditorWidget;
}
namespace GrantleeThemeEditor
{
class GRANTLEETHEMEEDITOR_EXPORT DesktopFilePage : public QWidget
{
    Q_OBJECT
public:
    enum DesktopFileOption {
        None = 1,
        ExtraDisplayVariables = 2,
        SpecifyFileName = 4
    };
    Q_DECLARE_FLAGS(DesktopFileOptions, DesktopFileOption)

    explicit DesktopFilePage(const QString &defaultFileName, DesktopFilePage::DesktopFileOptions options, QWidget *parent = Q_NULLPTR);
    ~DesktopFilePage();

    void saveTheme(const QString &path);
    void loadTheme(const QString &path);

    void setThemeName(const QString &themeName);
    QString filename() const;

    QString description() const;

    QString themeName() const;
    void createZip(const QString &themeName, KZip *zip);
    void installTheme(const QString &themePath);
    void setDefaultDesktopName(const QString &name);

Q_SIGNALS:
    void mainFileNameChanged(const QString &filename);
    void extraDisplayHeaderChanged(const QStringList &headers);
    void changed();

private:
    void slotFileNameChanged(const QString &);
    void slotExtraDisplayHeadersChanged();
    void saveAsFilename(const QString &filename);
    QString mDefaultDesktopName;
    QLineEdit *mName;
    KPIMTextEdit::PlainTextEditorWidget *mDescription;
    QLineEdit *mFilename;
    QLineEdit *mAuthor;
    QLineEdit *mEmail;
    QLineEdit *mVersion;
    KEditListWidget *mExtraDisplayHeaders;
};
}

#endif // DESKTOPFILEPAGE_H
