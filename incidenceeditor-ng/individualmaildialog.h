/*
  Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>

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

#ifndef INCIDENCEEDITOR_INDIVIDUALMAILDIALOG_H
#define INCIDENCEEDITOR_INDIVIDUALMAILDIALOG_H

#include <kcalcore/attendee.h>
#include <KDialog>

#include <QComboBox>

class TestIndividualMailDialog;

namespace IncidenceEditorNG {

// Shows a dialog with a question and the option to select which attendee should get the mail or to open a composer for him.
// Used to get individual mails for attendees of an event.
class IndividualMailDialog : public KDialog
{
    Q_OBJECT
    friend TestIndividualMailDialog;
public:
    enum Decisions {
        Update,         /**< send automatic mail to attendee */
        NoUpdate,       /**< do not send mail to attendee */
        Edit            /**< open composer for attendee */
    };
    explicit IndividualMailDialog(const QString &question, const KCalCore::Attendee::List &attendees,
                                  const KGuiItem &buttonYes, const KGuiItem &buttonNo, QWidget *parent = 0);
    virtual ~IndividualMailDialog();

    KCalCore::Attendee::List editAttendees() const;
    KCalCore::Attendee::List updateAttendees() const;

private:
    QHash<KCalCore::Attendee::Ptr, QComboBox *> mAttendeeDecision;
};

}

#endif
