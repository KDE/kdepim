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

#ifndef GRAVATARDOWNLOADPIXMAPWIDGET_H
#define GRAVATARDOWNLOADPIXMAPWIDGET_H

#include <QWidget>
#include "gravatar_export.h"
class QLabel;
class QLineEdit;
class QPushButton;
class QCheckBox;
namespace Gravatar
{
class GravatarResolvUrlJob;
class GRAVATAR_EXPORT GravatarDownloadPixmapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GravatarDownloadPixmapWidget(QWidget *parent = Q_NULLPTR);
    ~GravatarDownloadPixmapWidget();

    QPixmap gravatarPixmap() const;

private Q_SLOTS:
    void slotSearchButton();

    void slotTextChanged(const QString &text);
    void slotResolvUrlFinish(Gravatar::GravatarResolvUrlJob *job);
private:
    QPixmap mGravatarPixmap;
    QLabel *mResultLabel;
    QLineEdit *mLineEdit;
    QPushButton *mGetPixmapButton;
    QCheckBox *mUseLibravatar;
    QCheckBox *mFallbackGravatar;
    QCheckBox *mUseHttps;
};
}

#endif // GRAVATARDOWNLOADPIXMAPWIDGET_H
