/*
  Copyright (C) 2016 eyeOS S.L.U., a Telefonica company, sales@eyeos.com

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

#include "viewertoolbaraction.h"
#include <KActionCollection>
#include <QLayout>
#include <QDebug>
using namespace MessageViewer;

ViewerToolBarAction::ViewerToolBarAction(QWidget *parent)
    : QToolBar(parent)
{
    setFloatable(false);
    setMovable(false);
    setIconSize(QSize(16,16));
}

ViewerToolBarAction::~ViewerToolBarAction()
{

}

void ViewerToolBarAction::updateViewerToolBar(KActionCollection *ac)
{
    if (ac) {
        addAction(ac->action(QLatin1String("akonadi_move_to_trash")));
        addAction(ac->action(QLatin1String("create_todo")));
        addAction(ac->action(QLatin1String("send_queued")));
        addAction(ac->action(QLatin1String("file_print")));

        QAction *act = ac->action(QLatin1String("reply"));
        addAction(act);

        QWidget *spacer = new QWidget;
        QHBoxLayout *hbox = new QHBoxLayout;
        spacer->setLayout(hbox);
        QSpacerItem *item = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);
        hbox->addSpacerItem(item);
        insertWidget(act, spacer);

        addAction(ac->action(QLatin1String("reply_all")));
        addAction(ac->action(QLatin1String("message_forward")));
    }
}
