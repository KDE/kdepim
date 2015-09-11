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

#ifndef LONGHEADERSTYLEINTERFACE_H
#define LONGHEADERSTYLEINTERFACE_H

#include <header/headerstyleinterface.h>

namespace MessageViewer
{

class LongHeaderStyleInterface : public MessageViewer::HeaderStyleInterface
{
    Q_OBJECT
public:
    explicit LongHeaderStyleInterface(MessageViewer::HeaderStyle *headerStyle, MessageViewer::HeaderStrategy *headerStrategy, QObject *parent = Q_NULLPTR);
    ~LongHeaderStyleInterface();

    void createAction(KActionMenu *menu, QActionGroup *actionGroup, KActionCollection *ac) Q_DECL_OVERRIDE;
};

}

#endif // LONGHEADERSTYLEINTERFACE_H
