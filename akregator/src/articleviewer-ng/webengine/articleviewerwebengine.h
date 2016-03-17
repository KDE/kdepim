/*
  Copyright (c) 2015-2016 Montel Laurent <montel@kde.org>

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

#ifndef ARTICLEVIEWERWEBENGINE_H
#define ARTICLEVIEWERWEBENGINE_H

#include <MessageViewer/WebEngineView>
#include <openurlrequest.h>
#include <shareserviceurlmanager.h>
#include <MessageViewer/ViewerPluginInterface>

class KActionCollection;
namespace MessageViewer
{
class WebHitTestResult;
class ViewerPluginToolManager;
class ViewerPluginInterface;
class MailWebEngineAccessKey;
}
namespace PimCommon
{
class ShareServiceUrlManager;
}
namespace KIO
{
class KUriFilterSearchProviderActions;
}
namespace Akregator
{
class ArticleViewerWebEnginePage;
class AKREGATOR_EXPORT ArticleViewerWebEngine : public MessageViewer::WebEngineView
{
    Q_OBJECT
public:
    enum ArticleAction {
        DeleteAction = 0,
        MarkAsRead,
        MarkAsUnRead,
        MarkAsImportant,
        SendUrlArticle,
        SendFileArticle,
        OpenInExternalBrowser,
        Share,
        OpenInBackgroundTab
    };
    explicit ArticleViewerWebEngine(KActionCollection *ac, QWidget *parent);
    ~ArticleViewerWebEngine();

    void showAboutPage();

    void disableIntroduction();
    void setArticleAction(ArticleViewerWebEngine::ArticleAction type, const QString &articleId, const QString &feed);

    bool zoomTextOnlyInFrame() const;

    void createViewerPluginToolManager(KActionCollection *ac, QWidget *parent);

protected:
    QUrl mCurrentUrl;
    KActionCollection *mActionCollection;
    PimCommon::ShareServiceUrlManager *mShareServiceManager;
    KIO::KUriFilterSearchProviderActions *mWebShortcutMenuManager;

private:
    enum MousePressedButtonType {
        RightButton = 0,
        LeftButton,
        MiddleButton,
    };

    void paintAboutScreen(const QString &templateName, const QVariantHash &data);
    QVariantHash introductionData() const;

public Q_SLOTS:
    void slotPrintPreview();
    void slotPrint();
    void slotCopy();
    void slotZoomTextOnlyInFrame(bool textOnlyInFrame);
    void slotSaveLinkAs();
    void slotCopyLinkAddress();
    void slotSaveImageOnDiskInFrame();
    void slotCopyImageLocationInFrame();
    void slotBlockImage();
    void slotExpandUrl();
Q_SIGNALS:
    void signalOpenUrlRequest(Akregator::OpenUrlRequest &);
    void showStatusBarMessage(const QString &link);
    void showContextMenu(const QPoint &pos);
    void articleAction(Akregator::ArticleViewerWebEngine::ArticleAction type, const QString &articleId, const QString &feed);
    void findTextInHtml();
    void textToSpeech();

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

    virtual void displayContextMenu(const QPoint &pos);
    void forwardKeyReleaseEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void forwardKeyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void forwardWheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
    void forwardMouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotServiceUrlSelected(PimCommon::ShareServiceUrlManager::ServiceType type);
    void slotLinkHovered(const QString &link);
    void slotLoadStarted();
    void slotLoadFinished();
    void slotLinkClicked(const QUrl &url);
    void slotOpenLinkInForegroundTab();
    void slotOpenLinkInBackgroundTab();
    void slotOpenLinkInBrowser();
    void slotShowContextMenu(const QPoint &pos);
    void slotWebHitFinished(const MessageViewer::WebHitTestResult &result);
    void slotActivatePlugin(MessageViewer::ViewerPluginInterface *interface);
protected:
    ArticleViewerWebEnginePage *mPageEngine;

    bool adblockEnabled() const;

    void contextMenuEvent(QContextMenuEvent *e) Q_DECL_OVERRIDE;
    QList<QAction *> viewerPluginActionList(MessageViewer::ViewerPluginInterface::SpecificFeatureTypes features);
private:
    MousePressedButtonType mLastButtonClicked;
    MessageViewer::ViewerPluginToolManager *mViewerPluginToolManager;
    MessageViewer::MailWebEngineAccessKey *mWebEngineViewAccessKey;
};
}

#endif // ARTICLEVIEWERWEBENGINE_H