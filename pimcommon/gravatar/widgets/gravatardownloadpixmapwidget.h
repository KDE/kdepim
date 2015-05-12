/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  based on code from Sune Vuorela <sune@vuorela.dk> (Rawatar source code)

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
#include "pimcommon_export.h"
class QLabel;
class QLineEdit;
class QPushButton;
namespace PimCommon {
class GravatarResolvUrlJob;
class PIMCOMMON_EXPORT GravatarDownloadPixmapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GravatarDownloadPixmapWidget(QWidget *parent = 0);
    ~GravatarDownloadPixmapWidget();

private Q_SLOTS:
    void slotSearchButton();

    void slotTextChanged(const QString &text);
    void slotResolvUrlFinish(PimCommon::GravatarResolvUrlJob *job);
private:
    QLabel *mResultLabel;
    QLineEdit *mLineEdit;
    QPushButton *mGetPixmapButton;
};
}

#endif // GRAVATARDOWNLOADPIXMAPWIDGET_H
