/*
  This file is part of the kcal library.

  Copyright (c) 2010 Klar�lvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef KCAL_ATTACHMENTHANDLER_H
#define KCAL_ATTACHMENTHANDLER_H

class TQString;
class TQWidget;

namespace KCal {

class Attachment;
class Incidence;
class ScheduleMessage;

/**
  @brief
  Provides methods to handle incidence attachments.

  Includes functions to view and save attachments.
*/
namespace AttachmentHandler {

  /**
    Finds the attachment in the user's calendar, by @p attachmentName and @p incidence.

    @param parent is the parent widget for the dialogs used in this function.
    @param attachmentName is the name of the attachment
    @param incidence is a pointer to a valid Incidence object containing the attachment.

    @return a pointer to the Attachment object located; 0 if no such attachment could be found.
  */
  Attachment *find( TQWidget *parent, const TQString &attachmentName, Incidence *incidence );

  /**
    Finds the attachment in the user's calendar, by @p attachmentName and a scheduler message;
    in other words, this function is intended to retrieve attachments from calendar invitations.

    @param parent is the parent widget for the dialogs used in this function.
    @param attachmentName is the name of the attachment
    @param message is a pointer to a valid ScheduleMessage object containing the attachment.

    @return a pointer to the Attachment object located; 0 if no such attachment could be found.
  */
  Attachment *find( TQWidget *parent, const TQString &attachmentName, ScheduleMessage *message );

  /**
    Finds the attachment in the user's calendar, by @p attachmentName and @p uid.

    @param parent is the parent widget for the dialogs used in this function.
    @param attachmentName is the name of the attachment
    @param uid is a TQString containing a UID of the incidence containing the attachment.

    @return a pointer to the Attachment object located; 0 if no such attachment could be found.
  */
  Attachment *find( TQWidget *parent, const TQString &attachmentName, const TQString &uid );

  /**
    Launches a viewer on the specified attachment.

    @param parent is the parent widget for the dialogs used in this function.
    @param attachment is a pointer to a valid Attachment object.

    @return true if the viewer program successfully launched; false otherwise.
  */
  bool view( TQWidget *parent, Attachment *attachment );

  /**
    Launches a viewer on the specified attachment.

    @param parent is the parent widget for the dialogs used in this function.
    @param attachmentName is the name of the attachment
    @param incidence is a pointer to a valid Incidence object containing the attachment.

    @return true if the attachment could be found and the viewer program successfully launched;
    false otherwise.
  */
  bool view( TQWidget *parent, const TQString &attachmentName, Incidence *incidence );

  /**
    Launches a viewer on the specified attachment.

    @param parent is the parent widget for the dialogs used in this function.
    @param attachmentName is the name of the attachment
    @param uid is a TQString containing a UID of the incidence containing the attachment.

    @return true if the attachment could be found and the viewer program successfully launched;
    false otherwise.
  */
  bool view( TQWidget *parent, const TQString &attachmentName, const TQString &uid );

  /**
    Launches a viewer on the specified attachment.

    @param parent is the parent widget for the dialogs used in this function.
    @param attachmentName is the name of the attachment
    @param message is a pointer to a valid ScheduleMessage object containing the attachment.

    @return true if the attachment could be found and the viewer program successfully launched;
    false otherwise.
  */
  bool view( TQWidget *parent, const TQString &attachmentName, ScheduleMessage *message );

  /**
    Saves the specified attachment to a file of the user's choice.

    @param parent is the parent widget for the dialogs used in this function.
    @param attachment is a pointer to a valid Attachment object.

    @return true if the save operation was successful; false otherwise.
  */
  bool saveAs( TQWidget *parent, Attachment *attachment );

  /**
    Saves the specified attachment to a file of the user's choice.

    @param parent is the parent widget for the dialogs used in this function.
    @param attachmentName is the name of the attachment
    @param incidence is a pointer to a valid Incidence object containing the attachment.

    @return true if the attachment could be found and the save operation was successful;
    false otherwise.
  */
  bool saveAs( TQWidget *parent, const TQString &attachmentName, Incidence *incidence );

  /**
    Saves the specified attachment to a file of the user's choice.

    @param parent is the parent widget for the dialogs used in this function.
    @param attachmentName is the name of the attachment
    @param uid is a TQString containing a UID of the incidence containing the attachment.

    @return true if the attachment could be found and the save operation was successful;
    false otherwise.
  */
  bool saveAs( TQWidget *parent, const TQString &attachmentName, const TQString &uid );

  /**
    Saves the specified attachment to a file of the user's choice.

    @param parent is the parent widget for the dialogs used in this function.
    @param attachmentName is the name of the attachment
    @param message is a pointer to a valid ScheduleMessage object containing the attachment.

    @return true if the attachment could be found and the save operation was successful;
    false otherwise.
  */
  bool saveAs( TQWidget *parent, const TQString &attachmentName, ScheduleMessage *message );
}

}

#endif
