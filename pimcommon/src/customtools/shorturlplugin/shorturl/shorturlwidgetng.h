/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef ShortUrlWidgetNg_H
#define ShortUrlWidgetNg_H

#include <QWidget>

class KLineEdit;
class QPushButton;
class QLabel;
class QLineEdit;
class QNetworkConfigurationManager;
namespace KPIM
{
class ProgressIndicatorLabel;
}

namespace PimCommon
{
class ShortUrlEngineInterface;
class ShortUrlWidgetNg : public QWidget
{
    Q_OBJECT
public:
    explicit ShortUrlWidgetNg(QWidget *parent = Q_NULLPTR);
    ~ShortUrlWidgetNg();

    void setStandalone(bool b);
public Q_SLOTS:
    void settingsUpdated();

private Q_SLOTS:
    void slotConvertUrl();
    void slotPasteToClipboard();
    void slotOriginalUrlChanged(const QString &text);
    void slotShortUrlChanged(const QString &text);
    void slotShortUrlDone(const QString &url);
    void slotShortUrlFailed(const QString &errMsg);
    void slotCloseWidget();
    void slotConfigure();
    void slotInsertShortUrl();
    void slotOpenShortUrl();

Q_SIGNALS:
    void toolsWasClosed();
    void insertText(const QString &Url);

private:
    void initializePlugins();
    void loadEngine();
    QLabel *mShorturlServiceName;
    KLineEdit *mOriginalUrl;
    QLineEdit *mShortUrl;
    QPushButton *mConvertButton;
    QPushButton *mCopyToClipboard;
    QPushButton *mInsertShortUrl;
    QPushButton *mOpenShortUrl;
    KPIM::ProgressIndicatorLabel *mIndicatorLabel;
    QNetworkConfigurationManager *mNetworkConfigurationManager;
    QHash<QString, PimCommon::ShortUrlEngineInterface *> mLstInterface;
    PimCommon::ShortUrlEngineInterface *mCurrentEngine;
};
}

#endif // ShortUrlWidgetNg_H
