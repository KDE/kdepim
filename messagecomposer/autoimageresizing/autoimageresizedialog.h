/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#ifndef AUTOIMAGERESIZEDIALOG_H
#define AUTOIMAGERESIZEDIALOG_H

#include <KDialog>
class QCheckBox;
class QSpinBox;

namespace MessageComposer {

class AutoImageResizeDialog : public KDialog
{
  Q_OBJECT
public:
  enum ImageType {
      JPEG = 0,
      PNG
  };
  explicit AutoImageResizeDialog(QWidget *parent = 0);
  ~AutoImageResizeDialog();

  void setImageFromData(const QByteArray& data, ImageType type);
  QByteArray imageData() const;

private Q_SLOTS:
  void slotUser1();
  void slotKeepOriginalSizeClicked(bool);
  void slotImageWidthChanged(int);
  void slotImageHeightChanged(int);

private:
  qreal mImageRatio;
  QCheckBox *mKeepOriginalSize;
  QCheckBox *mKeepImageRatio;
  QSpinBox *mWidth;
  QSpinBox *mHeight;
};
}

#endif // AUTOIMAGERESIZEDIALOG_H
