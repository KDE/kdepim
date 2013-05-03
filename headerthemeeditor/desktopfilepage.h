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

class KLineEdit;
class KZip;

namespace PimCommon {
class SimpleStringListEditor;
}

class DesktopFilePage : public QWidget
{
    Q_OBJECT
public:
    explicit DesktopFilePage(QWidget *parent = 0);
    ~DesktopFilePage();

    void saveTheme(const QString &path);
    void loadTheme(const QString &path);

    void setThemeName(const QString &themeName);
    QString filename() const;


    QString themeName() const;
    void createZip(const QString &themeName, KZip *zip);

    bool wasChanged() const;

private Q_SLOTS:
    void slotChanged();

private:
    void saveAsFilename(const QString &filename);
    KLineEdit *mName;
    KLineEdit *mDescription;
    KLineEdit *mFilename;
    PimCommon::SimpleStringListEditor *mExtraDisplayHeaders;
    bool mChanged;
};

#endif // DESKTOPFILEPAGE_H
