/*
  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.trolltech.com and http://www.kde.org respectively

  Copyright (c) 2002-2004 Klarälvdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>

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
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#ifndef CALENDARSUPPORT_GROUPWARE_H
#define CALENDARSUPPORT_GROUPWARE_H

#include "calendarsupport_export.h"
#include "incidencechanger.h"

#include <KCalCore/ICalFormat>

namespace Akonadi {
  class Item;
}

namespace CalendarSupport {

class CALENDARSUPPORT_EXPORT GroupwareUiDelegate
{
  public:
    virtual ~GroupwareUiDelegate();
    virtual void requestIncidenceEditor( const Akonadi::Item &item ) = 0;
};

class CALENDARSUPPORT_EXPORT Groupware : public QObject
{
  Q_OBJECT
  public:

    /**
       Flags for sendICalMessage()'s dialogResults parameter.
    */
    enum SendICalMessageDialogResult {
      NoDialog = 0,               /**> No dialog was presented to the user */
      SendEmail  = 1,             /**> Regular "Do you wish to send email?" dialog, and user pressed "send". */
      DoNotSendEmail = 2,         /**> Regular "Do you wish to send email?" dialog, and user pressed "No". */
      NotOrganizerAbort = 4,      /**> "You are not the organizer, abort?" dialog, and user pressed "Abort" */
      NotOrganizerConfirm = 8,    /**> "You are not the organizer, abort?" dialog, and user pressed "Continue" */
      SendingErrorAbort = 16,     /**> "There was an error sending e-mail, abort?" dialog, and user pressed "Abort" */
      SendingErrorDoNotSend = 32  /**> "There was an error sending e-mail, abort?" dialog, and user pressed "Do not send" */
    };
    Q_DECLARE_FLAGS( SendICalMessageDialogResults, SendICalMessageDialogResult )

    static Groupware *create( CalendarSupport::Calendar *, GroupwareUiDelegate * );
    static Groupware *instance();

    /** Send iCal messages after asking the user
         Returns false if the user cancels the dialog, and true if the
         user presses Yes or No.

         @param dialogCode will contain user answers to all dialogs that
                were presented. Use this information for example to see
                if user pressed cancel so you don't call this twice on atomic
                operations like dissociating occurrences ( which are composed
                of an "add incidence" and a "change incidence" ).
    */
    bool sendICalMessage( QWidget *parent, KCalCore::iTIPMethod method,
                          const KCalCore::Incidence::Ptr &incidence,
                          IncidenceChanger::HowChanged action,
                          bool attendeeStatusChanged,
                          SendICalMessageDialogResults &dialogResults );

    /**
      Send counter proposal message.
      @param oldEvent The original event provided in the invitations.
      @param newEvent The new event as edited by the user.
    */
    void sendCounterProposal( KCalCore::Event::Ptr oldEvent, KCalCore::Event::Ptr newEvent ) const;

    // DoNotNotify is a flag indicating that the user does not want
    // updates sent back to the organizer.
    void setDoNotNotify( bool notify ) { mDoNotNotify = notify; }
    bool doNotNotify() { return mDoNotNotify; }

    bool handleInvitation( const QString &receiver, const QString &iCal, const QString &type );

  protected:
    Groupware( CalendarSupport::Calendar *, GroupwareUiDelegate * );

  private:
    static Groupware *mInstance;
    KCalCore::ICalFormat mFormat;
    Calendar *mCalendar;
    GroupwareUiDelegate *mDelegate;
    bool mDoNotNotify;
};

}

#endif
