/*
  Copyright (c) 2015-2016 Montel Laurent <montel@kde.org>

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

#ifndef VCARDEXPORTSELECTIONWIDGET_H
#define VCARDEXPORTSELECTIONWIDGET_H

#include <QWidget>

class QCheckBox;
class VCardExportSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VCardExportSelectionWidget(QWidget *parent = Q_NULLPTR);
    ~VCardExportSelectionWidget();
    enum ExportField {
        None = 0,
        Private = 1,
        Business = 2,
        Other = 4,
        Encryption = 8,
        Picture = 16,
        DiplayName = 32
    };
    Q_ENUMS(ExportField)
    Q_DECLARE_FLAGS(ExportFields, ExportField)

    ExportFields exportType() const;
private:
    void readSettings();
    void writeSettings();
    QCheckBox *mPrivateBox;
    QCheckBox *mBusinessBox;
    QCheckBox *mOtherBox;
    QCheckBox *mEncryptionKeys;
    QCheckBox *mPictureBox;
    QCheckBox *mDisplayNameBox;
};

#endif // VCARDEXPORTSELECTIONWIDGET_H
