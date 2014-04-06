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

#ifndef OPENATTACHMENTFOLDERWIDGET_H
#define OPENATTACHMENTFOLDERWIDGET_H

#include <KMessageWidget>
#include <KUrl>
class QTimer;
namespace MessageViewer {
class OpenAttachmentFolderWidget : public KMessageWidget
{
    Q_OBJECT
public:
    explicit OpenAttachmentFolderWidget(QWidget *parent=0);
    ~OpenAttachmentFolderWidget();

    void setFolder(const KUrl &url);

public slots:
    void slotShowWarning();
    void slotHideWarning();

private slots:
    void slotOpenAttachmentFolder();
    void slotTimeOut();
private:
    KUrl mUrl;
    QTimer *mTimer;
};
}

#endif // OPENATTACHMENTFOLDERWIDGET_H
