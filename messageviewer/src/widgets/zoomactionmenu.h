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

#ifndef ZOOMACTIONMENU_H
#define ZOOMACTIONMENU_H

#include <KActionMenu>
class KToggleAction;
class KActionCollection;
namespace MessageViewer
{
class MailWebView;
class ZoomActionMenu : public KActionMenu
{
    Q_OBJECT
public:
    explicit ZoomActionMenu(MessageViewer::MailWebView *mailViewer, QObject *parent = Q_NULLPTR);
    ~ZoomActionMenu();

    void createZoomActions();
    KToggleAction *zoomTextOnlyAction() const;

    QAction *zoomInAction() const;

    QAction *zoomOutAction() const;

    QAction *zoomResetAction() const;

    void setActionCollection(KActionCollection *ac);

    void setZoomFactor(qreal zoomFactor);
    qreal zoomFactor() const;

    void setWebViewerZoomFactor(qreal zoomFactor);
    bool zoomTextOnly() const;

public Q_SLOTS:
    void slotZoomIn();
    void slotZoomOut();
    void setZoomTextOnly(bool textOnly);
    void slotZoomTextOnly();
    void slotZoomReset();

private:

    qreal mZoomFactor;
    KToggleAction *mZoomTextOnlyAction;
    QAction *mZoomInAction;
    QAction *mZoomOutAction;
    QAction *mZoomResetAction;
    KActionCollection *mActionCollection;
    MessageViewer::MailWebView *mMailWebViewer;
    bool mZoomTextOnly;
};
}

#endif // ZOOMACTIONMENU_H
