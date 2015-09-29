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

#ifndef VIEWERPLUGINCREATENOTEINTERFACE_H
#define VIEWERPLUGINCREATENOTEINTERFACE_H

#include <viewerplugins/viewerplugininterface.h>
class KActionCollection;

namespace MessageViewer
{
class NoteEdit;
class ViewerPluginCreatenoteInterface : public ViewerPluginInterface
{
    Q_OBJECT
public:
    explicit ViewerPluginCreatenoteInterface(KActionCollection *ac, QWidget *parent = Q_NULLPTR);
    ~ViewerPluginCreatenoteInterface();

    void setText(const QString &text) Q_DECL_OVERRIDE;
    QAction *action() const Q_DECL_OVERRIDE;
    void setMessage(const KMime::Message::Ptr &value) Q_DECL_OVERRIDE;
    void closePlugin() Q_DECL_OVERRIDE;
private:
    void createAction(KActionCollection *ac);
    NoteEdit *mNoteEdit;
    QAction *mAction;

};
}
#endif // VIEWERPLUGINCREATENOTEINTERFACE_H
