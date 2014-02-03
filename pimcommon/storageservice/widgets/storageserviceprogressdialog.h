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
#include "libkdepim/widgets/overlaywidget.h"



#include <QScrollArea>
class KVBox;
namespace PimCommon {
class StorageServiceProgressWidget;
class StorageServiceAbstract;
class ProgressIndicatorView : public QScrollArea
{
    Q_OBJECT
public:
    explicit ProgressIndicatorView( QWidget * parent = 0 );
    ~ProgressIndicatorView();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    StorageServiceProgressWidget *addTransactionItem(PimCommon::StorageServiceAbstract *service, bool first);

public Q_SLOTS:
    void slotLayoutFirstItem();

protected:
    void resizeEvent ( QResizeEvent *event );

private:
    KVBox *mBigBox;
};


class PIMCOMMON_EXPORT StorageServiceProgressDialog : public KPIM::OverlayWidget
{
    Q_OBJECT
public:
    explicit StorageServiceProgressDialog(QWidget *alignWidget, QWidget *parent=0);
    ~StorageServiceProgressDialog();

private:
    ProgressIndicatorView *mScrollView;
};
}
#endif // STORAGESERVICEPROGRESSDIALOG_H
