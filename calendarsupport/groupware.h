/*
  Copyright (c) 2002-2004 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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
#include "nepomukcalendar.h"
#include <akonadi/calendar/etmcalendar.h>
#include <KCalCore/ICalFormat>

namespace Akonadi {
  class Item;
}

namespace CalendarSupport {

class MailScheduler;

class CALENDARSUPPORT_EXPORT GroupwareUiDelegate
{
  public:
    virtual ~GroupwareUiDelegate();
    virtual void requestIncidenceEditor( const Akonadi::Item &item ) = 0;

    virtual void setCalendar( const Akonadi::ETMCalendar::Ptr &calendar ) = 0;
    virtual void createCalendar() = 0;
};

class CALENDARSUPPORT_EXPORT Groupware : public QObject
{
  Q_OBJECT
  public:
    /**
       @see sendICalMessage
     */
    struct SendICalMessageDialogAnswers {
      SendICalMessageDialogAnswers() : sendEmailAnswer( 0 ),
                                       notOrganizerAnswer( 0 ),
                                       sendingErrorAnswer( 0 ) { }
      int sendEmailAnswer;
      int notOrganizerAnswer;
      int sendingErrorAnswer;
    };

    static Groupware *create( GroupwareUiDelegate * );
    static Groupware *instance();

    ~Groupware();

    /** Send iCal messages after asking the user
         Returns false if the user cancels the dialog, and true if the
         user presses Yes or No. ( This last afirmation seems false, i don't see a
         cancel button, and there's more than one dialog ).

         @param dialogAnswers will contain user answers to all dialogs that
                were presented.

                If @p reuseDialogAnswers is true, this variable is also read,
                otherwise only written to.

                This is useful so the user isn't presented with the same dialog
                twice on the same atomic operations like dissociating occurrences
                ( which are composed of an "add incidence" and a "change incidence" ).

        @param reuseDialogAnswers, if true, sendICalMessage() will only
               ask the user if it didn't find the answer in @p dialogResults
    */
    bool sendICalMessage( QWidget *parent,
                          KCalCore::iTIPMethod method,
                          const KCalCore::Incidence::Ptr &incidence,
                          IncidenceChanger::HowChanged action,
                          bool attendeeStatusChanged,
                          SendICalMessageDialogAnswers &dialogAnswers,
                          MailScheduler &scheduler,
                          bool reuseDialogAnswers = false );

    // DoNotNotify is a flag indicating that the user does not want
    // updates sent back to the organizer.
    void setDoNotNotify( bool notify ) { mDoNotNotify = notify; }
    bool doNotNotify() { return mDoNotNotify; }
    void handleInvitation( const QString &receiver,
                           const QString &iCal,
                           const QString &type );
  private Q_SLOTS:
    void finishHandlingInvitation();

    // Frees calendar if it doesn't have jobs running
    void calendarJobFinished( bool success, const QString &errorString );

  Q_SIGNALS:
    void handleInvitationFinished( bool success, const QString &errorMessage );

  protected:
    Groupware( GroupwareUiDelegate * );

  private:
    static Groupware *mInstance;
    KCalCore::ICalFormat mFormat;
    GroupwareUiDelegate *mDelegate;
    bool mDoNotNotify;
    class Private;
    Private *const d;
};

}

//@cond PRIVATE
inline uint qHash( const QSharedPointer<CalendarSupport::NepomukCalendar> &key )
{
  return qHash<CalendarSupport::NepomukCalendar>( key.data() );
}
//@endcond

#endif
