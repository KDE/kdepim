/*
  Copyright (C) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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

#ifndef INCIDENCEEDITOR_INCIDENCEDEFAULTS_H
#define INCIDENCEEDITOR_INCIDENCEDEFAULTS_H

#include "incidenceeditors_ng_export.h"

#include <KCalCore/Incidence>

namespace IncidenceEditorNG
{

class IncidenceDefaultsPrivate;

class INCIDENCEEDITORS_NG_EXPORT IncidenceDefaults
{
public:
    explicit IncidenceDefaults(bool cleanupAttachmentTEmporaryFiles = false);
    IncidenceDefaults(const IncidenceDefaults &other);
    ~IncidenceDefaults();

    IncidenceDefaults &operator=(const IncidenceDefaults &other);

    /**
      Sets the attachments that are added by default to incidences.
    */
    void setAttachments(const QStringList &attachments,
                        const QStringList &attachmentMimetypes = QStringList(),
                        const QStringList &attachmentLabels = QStringList(),
                        bool inlineAttachment = false);

    /**
      Sets the attendees that are added by default to incidences.
      @param attendees Expected to be of the form "name name <email>"
    */
    void setAttendees(const QStringList &attendees);

    /**
      Sets the list of identities to be used for the user. The items in the list
      are expected to be of the form: "name [name] <email>".

      If the list is empty, it is assumed that no valid identities are configured.

      @param fullEmails The list of name email pairs that the user has configured as identities.
    */
    void setFullEmails(const QStringList &fullEmails);

    /**
      This is used to do a smarter guess about which identity to use for the organizer.
      If the groupware server is not set, the first avaialble identity will be used.
      @param domain The gropuware server domain name without any protocol prefixes
      (e.g. demo.kolab.org).
    */
    void setGroupWareDomain(const QString &domain);

    /**
      Sets the incidence related to the incidence for which to set the defaults. For
      example the parent todo of a new sub todo.
    */
    void setRelatedIncidence(const KCalCore::Incidence::Ptr &incidence);

    /**
      Set the start date/time to use for passed incidences. This defaults to the
      current start date/time. The main purpose of this method is supporting
      defaults for new incidences that where created with a given time slot.
      @param startDT The start date time to set on the incidence.
    */
    void setStartDateTime(const KDateTime &startDT);

    /**
      Set the end date/time to use for passed incidences. This defaults to the
      current start date/time. The main purpose of this method is supporting
      defaults for new incidences that where created with a given time slot.
      @param endDT The start date time to set on the incidence.
    */
    void setEndDateTime(const KDateTime &endDT);

    /**
      Sets the default values for @param incidence. This method is merely meant for
      <em>new</em> icidences. However, it will clear out all fields and set them
      to default values.
      @param incidence The incidence that will get default values for all of its field.
    */
    void setDefaults(const KCalCore::Incidence::Ptr &incidence) const;

    /**
     * Returns minimal incidence defaults: e-mails and groupware domain.
     *
     * TODO: See if this is always called when using IncidenceDefaults.
     * If yes, this should be done inside ctor.
     */
    static IncidenceDefaults minimalIncidenceDefaults(bool cleanupAttachmentTempFiles = false);

    // Returns the e-mail address used for the organizer when we can't find anything useful
    // This is something like "invalid@invalid"
    static QString invalidEmailAddress();

private:
    IncidenceDefaultsPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(IncidenceDefaults)
};

}

#endif
