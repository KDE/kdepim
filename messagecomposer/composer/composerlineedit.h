/*
  Copyright (c) 2010 Volker Krause <vkrause@kde.org>

  Based on kmail/kmlineeditspell.h/cpp
  Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef MESSAGECOMPOSER_COMPOSERLINEEDIT_H
#define MESSAGECOMPOSER_COMPOSERLINEEDIT_H

#include "messagecomposer_export.h"
#include <libkdepim/addressline/addresslineedit/addresseelineedit.h>
class KConfig;
namespace MessageComposer
{

class MESSAGECOMPOSER_EXPORT ComposerLineEdit : public KPIM::AddresseeLineEdit
{
    Q_OBJECT

public:
    explicit ComposerLineEdit(bool useCompletion, QWidget *parent = Q_NULLPTR);

    void setRecentAddressConfig(KConfig *config);

Q_SIGNALS:
    void focusUp();
    void focusDown();

protected:

    // Inherited. Always called by the parent when this widget is created.
    virtual void loadContacts();

    void keyPressEvent(QKeyEvent *) Q_DECL_OVERRIDE;

#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *e) Q_DECL_OVERRIDE;
#endif

private Q_SLOTS:
    void editRecentAddresses();
    void groupDropExpandResult(KJob *);

private:
#ifndef QT_NO_DRAGANDDROP
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
#endif
    void insertEmails(const QStringList &emails);

private:
    KConfig *m_recentAddressConfig;
};

}

#endif
