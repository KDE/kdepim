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

class QString;
class QWidget;

namespace KCal {

class Attachment;

/**
  @brief
  Provides methods to handle incidence attachments.

  Includes functions to view and save attachments.
*/
namespace AttachmentHandler {

  /**
    Finds the attachment in the user's calendar, by @p attachmentName and @uid.

    @param parent is the parent widget for the dialogs used in this function.
    @param attachmentName is the name of the attachment
    @param uid is a QString containing a UID of the incidence containing the attachment.

    @return a pointer to the Attachment object located; 0 if no such attachment could be found.
  */
  Attachment *find( QWidget *parent, const QString &attachmentName, const QString &uid );

  /**
    Launches a viewer on the specified attachment.

    @param parent is the parent widget for the dialogs used in this function.
    @param attachmentName is the name of the attachment
    @param uid is a QString containing a UID of the incidence containing the attachment.

    @return true if the attachment could be found and the viewer program successfully launched;
    false otherwise.
  */
  bool view( QWidget *parent, const QString &attachmentName, const QString &uid );

  /**
    Saves the specified attachment to a file of the user's choice.

    @param parent is the parent widget for the dialogs used in this function.
    @param attachmentName is the name of the attachment
    @param uid is a QString containing a UID of the incidence containing the attachment.

    @return true if the attachment could be found and the save operation was successful;
    false otherwise.
  */
  bool saveAs( QWidget *parent, const QString &attachmentName, const QString &uid );

}

}

#endif
