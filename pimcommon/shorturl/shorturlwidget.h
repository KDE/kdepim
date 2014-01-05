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

#ifndef SHORTURLWIDGET_H
#define SHORTURLWIDGET_H

#include <QWidget>
#include "pimcommon_export.h"

#include <Solid/Networking>

class KLineEdit;
class KToggleAction;
class QPushButton;

namespace KPIMUtils {
class ProgressIndicatorLabel;
}

namespace PimCommon {
class AbstractShortUrl;
class PIMCOMMON_EXPORT ShortUrlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ShortUrlWidget(QWidget *parent=0);
    ~ShortUrlWidget();

    void setStandalone(bool b);
    KToggleAction *toggleAction();


public Q_SLOTS:
    void settingsUpdated();

private Q_SLOTS:
    void slotConvertUrl();
    void slotPasteToClipboard();
    void slotOriginalUrlChanged(const QString &text);
    void slotShortUrlChanged(const QString &text);
    void slotShortUrlDone(const QString &url);
    void slotShortUrlFailed(const QString &errMsg);
    void slotSystemNetworkStatusChanged(Solid::Networking::Status status);
    void slotCloseWidget();    
    void slotConfigure();
    void slotInsertShortUrl();
    void slotOpenShortUrl();

Q_SIGNALS:
    void shortUrlWasClosed();
    void insertShortUrl(const QString &Url);

private:
    void loadEngine();
    KLineEdit *mOriginalUrl;
    KLineEdit *mShortUrl;
    QPushButton *mConvertButton;
    QPushButton *mCopyToClipboard;
    QPushButton *mInsertShortUrl;
    QPushButton *mOpenShortUrl;
    AbstractShortUrl *mEngine;
    KPIMUtils::ProgressIndicatorLabel *mIndicatorLabel;
    KToggleAction *mToggleAction;
    bool mNetworkUp;
    bool mStandalone;
};
}

#endif // SHORTURLWIDGET_H
