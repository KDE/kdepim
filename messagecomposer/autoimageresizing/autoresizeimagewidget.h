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

#ifndef AUTORESIZEIMAGEWIDGET_H
#define AUTORESIZEIMAGEWIDGET_H
#include <QWidget>
#include "messagecomposer_export.h"

class KComboBox;
namespace Ui {
class AutoResizeImageWidget;
}

class QButtonGroup;

namespace MessageComposer {
class MESSAGECOMPOSER_EXPORT AutoResizeImageWidget : public QWidget
{
  Q_OBJECT
public:
  explicit AutoResizeImageWidget(QWidget *parent = 0);
  ~AutoResizeImageWidget();

  void loadConfig();
  void writeConfig();
  void resetToDefault();

Q_SIGNALS:
  void changed();

private Q_SLOTS:
  void slotComboboxChanged(int index);
  void slotSourceFilterClicked(int);

private:
  void initComboBox(KComboBox *combo);
  void initWriteImageFormat();
  Ui::AutoResizeImageWidget *ui;
  QButtonGroup *mSourceFilterGroup;
  bool mWasChanged;
};
}

#endif // AUTORESIZEIMAGEWIDGET_H
