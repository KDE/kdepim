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

#include "composerlineedit.h"

#include "libkdepim/recentaddresses.h"
#include "libkdepim/completionconfiguredialog.h"
#include "settings/messagecomposersettings.h"
#include "messageviewer/autoqpointer.h"

#include <MessageCore/StringUtil>

#include <KEmailAddress>
#include <kcontacts/vcarddrag.h>
#include <kcontacts/contactgroup.h>
#include <kcontacts/vcardconverter.h>

#include <kmessagebox.h>
#include <kcompletionbox.h>
#include <KLocalizedString>
#include <KIO/StoredTransferJob>
#include <KJobWidgets>

#include <qmenu.h>
#include <qurl.h>
#include <QFile>
#include <QCursor>
#include <QKeyEvent>
#include <QDropEvent>
#include <kcontacts/contactgrouptool.h>
#include <Akonadi/Contact/ContactGroupExpandJob>
#include <Akonadi/Contact/ContactGroupSearchJob>
#include <QBuffer>

using namespace MessageComposer;

ComposerLineEdit::ComposerLineEdit(bool useCompletion, QWidget *parent)
    : KPIM::AddresseeLineEdit(parent, useCompletion)
{
    allowSemicolonAsSeparator(MessageComposerSettings::allowSemicolonAsAddressSeparator());
    setShowRecentAddresses(MessageComposerSettings::self()->showRecentAddressesInComposer());
    setRecentAddressConfig(MessageComposerSettings::self()->config());
    loadContacts();
    setEnableBalooSearch(MessageComposerSettings::showBalooSearchInComposer());
    connect(this, &ComposerLineEdit::editingFinished, this, &ComposerLineEdit::slotEditingFinished);
    connect(this, &ComposerLineEdit::textCompleted, this, &ComposerLineEdit::slotEditingFinished);

}

ComposerLineEdit::~ComposerLineEdit()
{
}

//-----------------------------------------------------------------------------
void ComposerLineEdit::keyPressEvent(QKeyEvent *e)
{
    if ((e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) &&
            !completionBox()->isVisible()) {
        Q_EMIT focusDown();
        AddresseeLineEdit::keyPressEvent(e);
        return;
    } else if (e->key() == Qt::Key_Up) {
        Q_EMIT focusUp();
        return;
    } else if (e->key() == Qt::Key_Down) {
        Q_EMIT focusDown();
        return;
    }
    AddresseeLineEdit::keyPressEvent(e);
}

