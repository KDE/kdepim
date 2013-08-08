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


#ifndef DESKTOPFILEPAGE_H
#define DESKTOPFILEPAGE_H

#include <QWidget>
#include "grantleethemeeditor_export.h"
class KLineEdit;
class KZip;

namespace PimCommon {
class SimpleStringListEditor;
}
namespace GrantleeThemeEditor {
class GRANTLEETHEMEEDITOR_EXPORT DesktopFilePage : public QWidget
{
    Q_OBJECT
public:
    explicit DesktopFilePage(const QString &defaultFileName, bool allowToAddExtraDisplayVariables, QWidget *parent = 0);
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
    void setAllowToAddExtraDisplayVariables(bool b);


Q_SIGNALS:
    void mainFileNameChanged(const QString &filename);
    void extraDisplayHeaderChanged(const QStringList &headers);
    void changed();

private Q_SLOTS:
    void slotFileNameChanged(const QString &);
    void slotExtraDisplayHeadersChanged();

private:
    void saveAsFilename(const QString &filename);
    QString mDefaultDesktopName;
    KLineEdit *mName;
    KLineEdit *mDescription;
    KLineEdit *mFilename;
    KLineEdit *mAuthor;
    KLineEdit *mEmail;
    KLineEdit *mVersion;
    PimCommon::SimpleStringListEditor *mExtraDisplayHeaders;
};
}

#endif // DESKTOPFILEPAGE_H
