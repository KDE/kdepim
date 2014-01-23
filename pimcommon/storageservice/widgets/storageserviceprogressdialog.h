/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef STORAGESERVICEPROGRESSDIALOG_H
#define STORAGESERVICEPROGRESSDIALOG_H
#include "pimcommon_export.h"
#include "libkdepim/progresswidget/overlaywidget.h"

namespace PimCommon {
class PIMCOMMON_EXPORT StorageServiceProgressDialog : public KPIM::OverlayWidget
{
    Q_OBJECT
public:
    explicit StorageServiceProgressDialog(QWidget *alignWidget, QWidget *parent=0);
    ~StorageServiceProgressDialog();
};
}
#endif // STORAGESERVICEPROGRESSDIALOG_H
