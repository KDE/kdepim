/*
    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "kdeclarativemainview.h"

#include <messagecomposer/messagefactory.h>

class KJob;

/** The new KMMainWidget ;-) */
class MainView : public KDeclarativeMainView
{
  Q_OBJECT
  public:
    explicit MainView(QWidget* parent = 0);

  public slots:
    void startComposer();
    void reply( quint64 id );
    void replyToAll( quint64 id );
    void forwardInline( quint64 id );

    void markImportant(bool checked);
    void markMailTask(bool checked);
    void modifyDone(KJob *job);
    void dataChanged();

  private:
    void reply( quint64 id, MessageComposer::ReplyStrategy replyStrategy );
};

#endif
