/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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


#ifndef STORAGEAUTHVIEWWIDGET_H
#define STORAGEAUTHVIEWWIDGET_H

#include <QWidget>
#include <QUrl>
class QProgressBar;
namespace PimCommon {
class StorageAuthWebView;
class StorageAuthViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StorageAuthViewWidget(QWidget *parent=0);
    ~StorageAuthViewWidget();

    void setUrl(const QUrl &url);

Q_SIGNALS:
    void urlChanged(const QUrl &);

private slots:
    void slotLoadStarted();
    void slotLoadFinished(bool success);

private:
    StorageAuthWebView *mWebView;
    QProgressBar *mProgressBar;
};
}

#endif // STORAGEAUTHVIEWWIDGET_H
