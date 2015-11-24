/* Copyright 2010 Thomas McGuire <mcguire@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MESSAGEVIEWER_MAILWEBVIEW_H
#define MESSAGEVIEWER_MAILWEBVIEW_H

#include "messageviewer/messageviewersettings.h"
#include "messageviewer_export.h"
#include <KWebView>

#include <boost/function.hpp>

class QLabel;
class KActionCollection;

namespace MessageViewer
{
class ScamDetection;

/// MailWebView extends KWebView so that it can emit the popupMenu() signal
class MESSAGEVIEWER_EXPORT MailWebView : public KWebView
{
    Q_OBJECT
public:

    explicit MailWebView(KActionCollection *actionCollection = Q_NULLPTR, QWidget *parent = Q_NULLPTR);
    ~MailWebView();

    enum FindFlag {
        FindWrapsAroundDocument = 1,
        FindBackward = 2,
        FindCaseSensitively = 4,
        HighlightAllOccurrences = 8,

        NumFindFlags
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag)

    bool findText(const QString &test, FindFlags flags);
    void clearFindSelection();

    void scrollUp(int pixels);
    void scrollDown(int pixels);
    bool isScrolledToBottom() const;
    bool hasVerticalScrollBar() const;
    void scrollPageDown(int percent);
    void scrollPageUp(int percent);
    void scrollToAnchor(const QString &anchor);

    QString selectedText() const;
    bool isAttachmentInjectionPoint(const QPoint &globalPos) const;
    void injectAttachments(const boost::function<QString()> &delayedHtml);
    bool removeAttachmentMarking(const QString &id);
    void markAttachment(const QString &id, const QString &style);
    bool replaceInnerHtml(const QString &id, const boost::function<QString()> &delayedHtml);
    void setElementByIdVisible(const QString &id, bool visible);
    void setHtml(const QString &html, const QUrl &baseUrl);
    QString htmlSource() const;
    void selectAll();
    void clearSelection();
    void scrollToRelativePosition(double pos);
    double relativePosition() const;

    void setAllowExternalContent(bool allow);

    QUrl linkOrImageUrlAt(const QPoint &global) const;

    void setScrollBarPolicy(Qt::Orientation orientation, Qt::ScrollBarPolicy policy);
    Qt::ScrollBarPolicy scrollBarPolicy(Qt::Orientation orientation) const;

    bool isAShortUrl(const QUrl &url) const;
    void expandUrl(const QUrl &url);

    void scamCheck();
    void saveMainFrameScreenshotInFile(const QString &filename);
    void openBlockableItemsDialog();

public Q_SLOTS:
    void slotShowDetails();

Q_SIGNALS:

    /// Emitted when the user right-clicks somewhere
    /// @param url if an URL was under the cursor, this parameter contains it. Otherwise empty
    /// @param point position where the click happened, in local coordinates
    void popupMenu(const QUrl &url, const QUrl &imageUrl, const QPoint &point);

    void linkHovered(const QString &link, const QString &title = QString(), const QString &textContent = QString());
    void messageMayBeAScam();

protected:
    /// Reimplemented to catch context menu events and emit popupMenu()
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    /// Reimplement for access key
    void keyReleaseEvent(QKeyEvent *) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void hideAccessKeys();

private:
    bool checkForAccessKey(QKeyEvent *event);
    void showAccessKeys();
    void makeAccessKeyLabel(QChar accessKey, const QWebElement &element);
    enum AccessKeyState {
        NotActivated,
        PreActivated,
        Activated
    };
    AccessKeyState mAccessKeyActivated;
    QList<QLabel *> mAccessKeyLabels;
    QHash<QChar, QWebElement> mAccessKeyNodes;
    QHash<QString, QChar> mDuplicateLinkElements;

    ScamDetection *mScamDetection;
    KActionCollection *mActionCollection;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(MessageViewer::MailWebView::FindFlags)

#endif /* MESSAGEVIEWER_MAILWEBVIEW_H */
