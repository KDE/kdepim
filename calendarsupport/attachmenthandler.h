/*
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
/**
  @file
  This file is part of the API for handling calendar data and provides
  static functions for dealing with calendar incidence attachments.

  @author Allen Winter \<winter@kde.org\>
*/
#ifndef CALENDARSUPPORT_ATTACHMENTHANDLER_H
#define CALENDARSUPPORT_ATTACHMENTHANDLER_H

#include <KCalCore/Attachment>
#include <KCalCore/Incidence>
#include <KCalCore/ScheduleMessage>

#include <QObject>

class KJob;

class QString;
class QWidget;

namespace CalendarSupport
{

/**
  @brief
  Provides methods to handle incidence attachments.

  Includes functions to view and save attachments.
*/
class AttachmentHandler : public QObject
{
    Q_OBJECT
public:

    /**
     * Constructs an AttachmentHandler.
     * @param parent is the parent widget for the dialogs used by this class.
     */
    explicit AttachmentHandler(QWidget *parent);
    ~AttachmentHandler();

    /**
     * Finds the attachment in the user's calendar, by @p attachmentName and @p incidence.
     *
     * @param attachmentName is the name of the attachment
     * @param incidence is a pointer to a valid Incidence object containing the attachment.
     * @return a pointer to the Attachment object located; 0 if no such attachment could be found.
     */
    KCalCore::Attachment::Ptr find(const QString &attachmentName,
                                   const KCalCore::Incidence::Ptr &incidence);

    /**
     * Finds the attachment in the user's calendar, by @p attachmentName and a scheduler message;
     * in other words, this function is intended to retrieve attachments from calendar invitations.
     *
     * @param attachmentName is the name of the attachment
     * @param message is a pointer to a valid ScheduleMessage object containing the attachment.
     * @return a pointer to the Attachment object located; 0 if no such attachment could be found.
     */
    KCalCore::Attachment::Ptr find(const QString &attachmentName,
                                   const KCalCore::ScheduleMessage::Ptr &message);

    /**
     * Launches a viewer on the specified attachment.
     *
     * @param attachment is a pointer to a valid Attachment object.
     * @return true if the viewer program successfully launched; false otherwise.
     */
    bool view(const KCalCore::Attachment::Ptr &attachment);

    /**
     * Launches a viewer on the specified attachment.
     *
     * @param attachmentName is the name of the attachment
     * @param incidence is a pointer to a valid Incidence object containing the attachment.
     * @return true if the attachment could be found and the viewer program successfully launched;
     * false otherwise.
     */
    bool view(const QString &attachmentName,
              const KCalCore::Incidence::Ptr &incidence);

    /**
      Launches a viewer on the specified attachment.

      @param attachmentName is the name of the attachment
      @param uid is a QString containing a UID of the incidence containing the attachment.

      This function is async and will return immediately. Listen to signal viewFinished()
      if you're interested on the success of this operation.

    */
    void view(const QString &attachmentName, const QString &uid);

    /**
      Launches a viewer on the specified attachment.

      @param attachmentName is the name of the attachment
      @param message is a pointer to a valid ScheduleMessage object containing the attachment.

      @return true if the attachment could be found and the viewer program successfully launched;
      false otherwise.
    */
    bool view(const QString &attachmentName,
              const KCalCore::ScheduleMessage::Ptr &message);

    /**
      Saves the specified attachment to a file of the user's choice.

      @param attachment is a pointer to a valid Attachment object.

      @return true if the save operation was successful; false otherwise.
    */
    bool saveAs(const KCalCore::Attachment::Ptr &attachment);

    /**
      Saves the specified attachment to a file of the user's choice.

      @param attachmentName is the name of the attachment
      @param incidence is a pointer to a valid Incidence object containing the attachment.

      @return true if the attachment could be found and the save operation was successful;
      false otherwise.
    */
    bool saveAs(const QString &attachmentName,
                const KCalCore::Incidence::Ptr &incidence);

    /**
      Saves the specified attachment to a file of the user's choice.

      @param attachmentName is the name of the attachment
      @param uid is a QString containing a UID of the incidence containing the attachment.

      This function is async, it will return immediately. Listen to signal saveAsFinished()
      if you're interested on the success of this operation.
    */
    void saveAs(const QString &attachmentName, const QString &uid);

    /**
      Saves the specified attachment to a file of the user's choice.

      @param attachmentName is the name of the attachment
      @param message is a pointer to a valid ScheduleMessage object containing the attachment.

      @return true if the attachment could be found and the save operation was successful;
      false otherwise.
    */
    bool saveAs(const QString &attachmentName,
                const KCalCore::ScheduleMessage::Ptr &message);

Q_SIGNALS:
    void viewFinished(const QString &uid, const QString &attachmentName, bool success);
    void saveAsFinished(const QString &uid, const QString &attachmentName, bool success);

private Q_SLOTS:
    void slotFinishView(KJob *job);
    void slotFinishSaveAs(KJob *job);

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond

}; // class AttachmentHandler

} // namespace CalendarSupport

#endif
