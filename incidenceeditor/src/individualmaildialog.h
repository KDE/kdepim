/*
 * Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * As a special exception, permission is given to link this program
 * with any edition of Qt, and distribute the resulting executable,
 * without including the source code for Qt in the source distribution.
 */

#ifndef INCIDENCEEDITOR_INDIVIDUALMAILDIALOG_H
#define INCIDENCEEDITOR_INDIVIDUALMAILDIALOG_H

#include <KCalCore/Attendee>
#include <KDialog>

#include <QComboBox>

class TestIndividualMailDialog;

namespace IncidenceEditorNG
{

// Shows a dialog with a question and the option to select which attendee should get the mail or to open a composer for him.
// Used to get individual mails for attendees of an event.
class IndividualMailDialog : public KDialog
{
    Q_OBJECT
    friend class ::TestIndividualMailDialog;
public:
    enum Decisions {
        Update,         /**< send automatic mail to attendee */
        NoUpdate,       /**< do not send mail to attendee */
        Edit            /**< open composer for attendee */
    };
    explicit IndividualMailDialog(const QString &question, const KCalCore::Attendee::List &attendees,
                                  const KGuiItem &buttonYes, const KGuiItem &buttonNo, QWidget *parent = Q_NULLPTR);
    virtual ~IndividualMailDialog();

    KCalCore::Attendee::List editAttendees() const;
    KCalCore::Attendee::List updateAttendees() const;

private:
    QHash<KCalCore::Attendee::Ptr, QComboBox *> mAttendeeDecision;
};

}

#endif
