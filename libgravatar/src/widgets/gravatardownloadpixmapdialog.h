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

#ifndef GRAVATARDOWNLOADPIXMAPDIALOG_H
#define GRAVATARDOWNLOADPIXMAPDIALOG_H

#include <QDialog>
#include "gravatar_export.h"

namespace Gravatar
{
class GravatarDownloadPixmapWidget;
class GRAVATAR_EXPORT GravatarDownloadPixmapDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GravatarDownloadPixmapDialog(QWidget *parent = Q_NULLPTR);
    ~GravatarDownloadPixmapDialog();

    QPixmap gravatarPixmap() const;

private Q_SLOTS:
    void slotAccepted();

private:
    Gravatar::GravatarDownloadPixmapWidget *mGravatarDownloadPixmapWidget;
};
}

#endif // GRAVATARDOWNLOADPIXMAPDIALOG_H
