/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#ifndef IMAGESCALINGWIDGET_H
#define IMAGESCALINGWIDGET_H
#include <QWidget>
#include "messagecomposer_export.h"

class KComboBox;
namespace Ui
{
class ImageScalingWidget;
}

class QButtonGroup;

namespace MessageComposer
{
class MESSAGECOMPOSER_EXPORT ImageScalingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageScalingWidget(QWidget *parent = 0);
    ~ImageScalingWidget();

    void loadConfig();
    void writeConfig();
    void resetToDefault();

Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void slotComboboxChanged(int index);
    void slotSourceFilterClicked(int);
    void slotRecipientFilterClicked(int);
    void slotHelpLinkClicked(const QString &);

private:
    void updateFilterSourceTypeSettings();
    void initComboBox(KComboBox *combo);
    void initWriteImageFormat();
    void updateEmailsFilterTypeSettings();
    void updateSettings();
    Ui::ImageScalingWidget *ui;
    QButtonGroup *mSourceFilenameFilterGroup;
    QButtonGroup *mRecipientFilterGroup;
    bool mWasChanged;
};
}

#endif // IMAGESCALINGWIDGET_H
