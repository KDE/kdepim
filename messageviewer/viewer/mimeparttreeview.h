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

#ifndef MIMEPARTTREEVIEW_H
#define MIMEPARTTREEVIEW_H

#include <QTreeView>
#include <kmime/kmime_message.h>

namespace MessageViewer
{
class MimeTreeModel;
class MimePartTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit MimePartTreeView(QWidget *parent = 0);
    ~MimePartTreeView();

    MessageViewer::MimeTreeModel *mimePartModel() const;

    void clearModel();
    void setRoot(KMime::Content *root);

    KMime::Content::List selectedContents();
private slots:
    void slotMimePartDestroyed();

private:
    void saveMimePartTreeConfig();
    void restoreMimePartTreeConfig();
    MimeTreeModel *mMimePartModel;
};
}

#endif // MIMEPARTTREEVIEW_H
