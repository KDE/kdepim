/*
    This file is part of Akregator2.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR2_ARTICLEVIEWER_H
#define AKREGATOR2_ARTICLEVIEWER_H

#include <khtml_part.h>

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <QPointer>
#include <QTimer>
#include <QWidget>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <vector>

class KJob;
class KUrl;
class QAbstractItemModel;

namespace Akregator2 {

class ArticleFormatter;
class OpenUrlRequest;

class ArticleViewerPart;

class ArticleViewer : public QWidget
{
    Q_OBJECT
    public:
        explicit ArticleViewer(QWidget* parent);
        ~ArticleViewer();


        /** Repaints the view. */
        void reload();

        void displayAboutPage();

        KParts::ReadOnlyPart* part() const;

        void setNormalViewFormatter(const boost::shared_ptr<ArticleFormatter>& formatter);

        void setCombinedViewFormatter(const boost::shared_ptr<ArticleFormatter>& formatter);

        void showItem( const Akonadi::Item& item );

        /** Shows the articles of the tree node @c node (combined view).
         * Changes in the node will update the view automatically.
         *
         *  @param node The node to observe */
        void showNode( QAbstractItemModel* );
        QSize sizeHint() const;

    public slots:

        void slotScrollUp();
        void slotScrollDown();
        void slotZoomIn(int);
        void slotZoomOut(int);
        void slotSetZoomFactor(int percent);
        void slotPrint();

        /** Update view if combined view mode is set. Has to be called when
         * the displayed node gets modified.
         */
        void slotUpdateCombinedView();

        /**
         * Clears the canvas and disconnects from the currently observed node
         * (if in combined view mode).
         */
        void slotClear();

        void slotShowSummary( const Akonadi::Collection& collection, int unreadCount );

        void slotPaletteOrFontChanged();

    signals:

        /** This gets emitted when url gets clicked */
        void signalOpenUrlRequest(Akregator2::OpenUrlRequest&);

        void started(KIO::Job*);
        void selectionChanged();
        void completed();

    protected: // methods
        int pointsToPixel(int points) const;

        bool openUrl(const KUrl &url);

    protected slots:

        void slotOpenUrlRequestDelayed(const KUrl&, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&);

        void slotCreateNewWindow(const KUrl& url,
                                    const KParts::OpenUrlArguments& args,
                                    const KParts::BrowserArguments& browserArgs,
                                    const KParts::WindowArgs& windowArgs,
                                    KParts::ReadOnlyPart** part);

        void slotPopupMenu(const QPoint&, const KUrl&, mode_t, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&, KParts::BrowserExtension::PopupFlags);

        /** Copies current link to clipboard. */
        void slotCopyLinkAddress();

        /** Copies currently selected text to clipboard */
        void slotCopy();

        /** Opens @c m_url inside this viewer */
        void slotOpenLinkInternal();

        /** Opens @c m_url in external viewer, eg. Konqueror */
        void slotOpenLinkInBrowser();

        /** Opens @c m_url in foreground tab */
        void slotOpenLinkInForegroundTab();

        /** Opens @c m_url in background tab */
        void slotOpenLinkInBackgroundTab();

        void slotSaveLinkAs();

        /** This changes cursor to wait cursor */
        void slotStarted(KIO::Job *);

        /** This reverts cursor back to normal one */
        void slotCompleted();

        void slotSelectionChanged();

        void triggerUpdate();
    // from ArticleViewer
    private:

        virtual void keyPressEvent(QKeyEvent* e);

        /** renders @c body. Use this method whereever possible.
         *  @param body html to render, without header and footer */
        void renderContent(const QString& body);

        /** Resets the canvas and adds writes the HTML header to it.
            */
        void beginWriting();

        /** Finishes writing to the canvas and completes the HTML (by adding closing tags) */
        void endWriting();

        void updateCss();

#ifdef KRSS_PORT_DISABLED
        void connectToNode(TreeNode* node);
        void disconnectFromNode(TreeNode* node);
#endif
        void setArticleActionsEnabled(bool enabled);

    private:
        QTimer m_updateTimer;
        KUrl m_url;
        QString m_normalModeCSS;
        QString m_combinedModeCSS;
        QString m_htmlFooter;
        QString m_currentText;
        KUrl m_imageDir;
        KUrl m_link;
        enum ViewMode { NormalView, CombinedView, SummaryView };
        ViewMode m_viewMode;
        ArticleViewerPart* m_part;
        QAbstractItemModel* m_model;
        boost::shared_ptr<ArticleFormatter> m_normalViewFormatter;
        boost::shared_ptr<ArticleFormatter> m_combinedViewFormatter;
};

class ArticleViewerPart : public KHTMLPart
{
    Q_OBJECT

    public:
        explicit ArticleViewerPart(QWidget* parent);

        bool closeUrl();

        int button() const;

    protected:

        /** reimplemented to get the mouse button */
        bool urlSelected(const QString &url, int button, int state, const QString &_target,
                         const KParts::OpenUrlArguments& args = KParts::OpenUrlArguments(),
                         const KParts::BrowserArguments& browserArgs = KParts::BrowserArguments());

    private:

        int m_button;
};

} // namespace Akregator2

#endif // AKREGATOR2_ARTICLEVIEWER_H

