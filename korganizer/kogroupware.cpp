/*
  This file is part of the Groupware/KOrganizer integration.

  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.trolltech.com and http://www.kde.org respectively

  Copyright (c) 2002-2004 KlarÃ¤lvdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA  02110-1301, USA.

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

#include "kogroupware.h"
#include "freebusymanager.h"
#include "calendarview.h"
#include "mailscheduler.h"
#include "koprefs.h"
#include "koincidenceeditor.h"
#include <libemailfunctions/email.h>
#include <libkcal/attendee.h>
#include <libkcal/calhelper.h>
#include <libkcal/journal.h>
#include <libkcal/incidenceformatter.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdirwatch.h>
#include <qfile.h>
#include <qregexp.h>
#include <qdir.h>
#include <qtimer.h>

FreeBusyManager *KOGroupware::mFreeBusyManager = 0;

KOGroupware *KOGroupware::mInstance = 0;

KOGroupware *KOGroupware::create( CalendarView *view,
                                  KCal::CalendarResources *calendar )
{
  if( !mInstance )
    mInstance = new KOGroupware( view, calendar );
  return mInstance;
}

void KOGroupware::destroy()
{
  delete mInstance;
  mInstance = 0;
  mFreeBusyManager = 0; // mFreeBusyManager is already deleted when mInstance is deleted
}

KOGroupware *KOGroupware::instance()
{
  // Doesn't create, that is the task of create()
  Q_ASSERT( mInstance );
  return mInstance;
}


KOGroupware::KOGroupware( CalendarView* view, KCal::CalendarResources* cal )
   : QObject( 0, "kmgroupware_instance" ), mView( view ), mCalendar( cal ), mDoNotNotify( false )
{
  // Set up the dir watch of the three incoming dirs
  KDirWatch* watcher = KDirWatch::self();
  watcher->addDir( locateLocal( "data", "korganizer/income.accepted/" ) );
  watcher->addDir( locateLocal( "data", "korganizer/income.tentative/" ) );
  watcher->addDir( locateLocal( "data", "korganizer/income.counter/" ) );
  watcher->addDir( locateLocal( "data", "korganizer/income.cancel/" ) );
  watcher->addDir( locateLocal( "data", "korganizer/income.reply/" ) );
  watcher->addDir( locateLocal( "data", "korganizer/income.delegated/" ) );
  connect( watcher, SIGNAL( dirty( const QString& ) ),
           this, SLOT( incomingDirChanged( const QString& ) ) );
  // Now set the ball rolling
  QTimer::singleShot( 0, this, SLOT(initialCheckForChanges()) );

  // Initialize
  lastUsedDialogAnswer = KMessageBox::Yes;
}

void KOGroupware::initialCheckForChanges()
{
  incomingDirChanged( locateLocal( "data", "korganizer/income.accepted/" ) );
  incomingDirChanged( locateLocal( "data", "korganizer/income.tentative/" ) );
  incomingDirChanged( locateLocal( "data", "korganizer/income.counter/" ) );
  incomingDirChanged( locateLocal( "data", "korganizer/income.cancel/" ) );
  incomingDirChanged( locateLocal( "data", "korganizer/income.reply/" ) );
  incomingDirChanged( locateLocal( "data", "korganizer/income.delegated/" ) );
}

void KOGroupware::slotViewNewIncidenceChanger( IncidenceChangerBase* changer )
{
    // Call slot perhapsUploadFB if an incidence was added, changed or removed
    connect( changer, SIGNAL( incidenceAdded( Incidence* ) ),
             mFreeBusyManager, SLOT( slotPerhapsUploadFB() ) );
    connect( changer, SIGNAL( incidenceChanged( Incidence*, Incidence*, KOGlobals::WhatChanged ) ),
             mFreeBusyManager, SLOT( slotPerhapsUploadFB() ) );
    connect( changer, SIGNAL( incidenceDeleted( Incidence * ) ),
             mFreeBusyManager, SLOT( slotPerhapsUploadFB() ) );
}

FreeBusyManager *KOGroupware::freeBusyManager()
{
  if ( !mFreeBusyManager ) {
    mFreeBusyManager = new FreeBusyManager( this, "freebusymanager" );
    mFreeBusyManager->setCalendar( mCalendar );
    connect( mCalendar, SIGNAL( calendarChanged() ),
             mFreeBusyManager, SLOT( slotPerhapsUploadFB() ) );
    connect( mView, SIGNAL( newIncidenceChanger( IncidenceChangerBase* ) ),
             this, SLOT( slotViewNewIncidenceChanger( IncidenceChangerBase* ) ) );
    slotViewNewIncidenceChanger( mView->incidenceChanger() );
  }

  return mFreeBusyManager;
}

void KOGroupware::incomingDirChanged( const QString& path )
{
  const QString incomingDirName = locateLocal( "data","korganizer/" )
                                  + "income.";
  if ( !path.startsWith( incomingDirName ) ) {
    kdDebug(5850) << "incomingDirChanged: Wrong dir " << path << endl;
    return;
  }
  QString action = path.mid( incomingDirName.length() );
  while ( action.length() > 0 && action[ action.length()-1 ] == '/' )
    // Strip slashes at the end
    action.truncate( action.length()-1 );

  // Handle accepted invitations
  QDir dir( path );
  const QStringList files = dir.entryList( QDir::Files );
  if ( files.isEmpty() )
    // No more files here
    return;

  // Read the file and remove it
  QFile f( path + "/" + files[0] );
  if (!f.open(IO_ReadOnly)) {
    kdError(5850) << "Can't open file '" << files[0] << "'" << endl;
    return;
  }
  QTextStream t(&f);
  t.setEncoding( QTextStream::UnicodeUTF8 );
  QString receiver = KPIM::getFirstEmailAddress( t.readLine() );
  QString iCal = t.read();

  f.remove();

  ScheduleMessage *message = mFormat.parseScheduleMessage( mCalendar, iCal );
  if ( !message ) {
    QString errorMessage;
    if (mFormat.exception())
      errorMessage = i18n( "Error message: %1" ).arg( mFormat.exception()->message() );
    kdDebug(5850) << "MailScheduler::retrieveTransactions() Error parsing "
                  << errorMessage << endl;
    KMessageBox::detailedError( mView,
        i18n("Error while processing an invitation or update."),
        errorMessage );
    return;
  }

  KCal::Scheduler::Method method =
    static_cast<KCal::Scheduler::Method>( message->method() );
  KCal::ScheduleMessage::Status status = message->status();
  KCal::Incidence* incidence =
    dynamic_cast<KCal::Incidence*>( message->event() );
  if(!incidence) {
    delete message;
    return;
  }
  KCal::MailScheduler scheduler( mCalendar );
  if ( action.startsWith( "accepted" ) || action.startsWith( "tentative" ) ||
       action.startsWith( "delegated" ) || action.startsWith( "counter" ) ) {
    // Find myself and set my status. This can't be done in the scheduler,
    // since this does not know the choice I made in the KMail bpf
    KCal::Attendee::List attendees = incidence->attendees();
    KCal::Attendee::List::ConstIterator it;
    for ( it = attendees.begin(); it != attendees.end(); ++it ) {
      if( (*it)->email() == receiver ) {
        if ( action.startsWith( "accepted" ) )
          (*it)->setStatus( KCal::Attendee::Accepted );
        else if ( action.startsWith( "tentative" ) )
          (*it)->setStatus( KCal::Attendee::Tentative );
        else if ( KOPrefs::instance()->outlookCompatCounterProposals() && action.startsWith( "counter" ) )
          (*it)->setStatus( KCal::Attendee::Tentative );
        else if ( action.startsWith( "delegated" ) )
          (*it)->setStatus( KCal::Attendee::Delegated );
        break;
      }
    }
    if ( KOPrefs::instance()->outlookCompatCounterProposals() || !action.startsWith( "counter" ) ) {
      if ( scheduler.acceptTransaction( incidence, method, status, receiver ) ) {
        mCalendar->save();
      }
    }
  } else if ( action.startsWith( "cancel" ) ) {
    // Delete the old incidence, if one is present
    if ( scheduler.acceptTransaction( incidence, KCal::Scheduler::Cancel, status, receiver ) ) {
      mCalendar->save();
    }
  } else if ( action.startsWith( "reply" ) ) {
    if ( method != Scheduler::Counter ) {
      if ( scheduler.acceptTransaction( incidence, method, status ) ) {
        mCalendar->save();
      }
    } else {
      // accept counter proposal
      if ( scheduler.acceptCounterProposal( incidence ) ) {
        mCalendar->save();
        // send update to all attendees
        sendICalMessage( mView, Scheduler::Request, incidence, 0,
                         KOGlobals::INCIDENCEEDITED, false );
      }
    }
  } else {
    kdError(5850) << "Unknown incoming action " << action << endl;
  }

  if ( action.startsWith( "counter" ) ) {
    Incidence *possiblyNewIncidence = NULL;
    KOIncidenceEditor *tmp = NULL;

    mView->editIncidence( incidence, QDate(), true, &possiblyNewIncidence );
    if ( possiblyNewIncidence ) {
      // If a new incidence was created we need to use that to find
      // the corresponding dialog.
      tmp = mView->editorDialog( possiblyNewIncidence );
    }
    if ( !tmp ) {
      tmp = mView->editorDialog( incidence );
    }
    if ( tmp ) {
      tmp->selectInvitationCounterProposal( true );
    } else {
      kdError(5850) << "Failed to find counter proposal editor." << endl;
    }
  }
  mView->updateView();
}

class KOInvitationFormatterHelper : public InvitationFormatterHelper
{
  public:
    virtual QString generateLinkURL( const QString &id ) { return "kmail:groupware_request_" + id; }
};

// A string comparison that considers that null and empty are the same
static bool stringCompare( const QString &s1, const QString &s2 )
{
  return ( s1.isEmpty() && s2.isEmpty() ) || ( s1 == s2 );
}

static bool compareIncsExceptAttendees( Incidence *i1, Incidence *i2 )
{
  if( i1->alarms().count() != i2->alarms().count() ) {
    return false; // no need to check further
  }

  Alarm::List::ConstIterator a1 = i1->alarms().begin();
  Alarm::List::ConstIterator a2 = i2->alarms().begin();
  for( ; a1 != i1->alarms().end() && a2 != i2->alarms().end(); ++a1, ++a2 ) {
    if( **a1 == **a2 ) {
      continue;
    } else {
      return false;
    }
  }

  bool recurrenceEqual = ( i1->recurrence() == 0 && i2->recurrence() == 0 );
  if ( !recurrenceEqual ) {
    recurrenceEqual = i1->recurrence() != 0 &&
                      i2->recurrence() != 0 &&
                      i2->recurrence() == i2->recurrence();
  }

  return
    recurrenceEqual &&
    i1->dtStart() == i2->dtStart() &&
    i1->organizer() == i2->organizer() &&
    i1->doesFloat() == i2->doesFloat() &&
    i1->duration() == i2->duration() &&
    i2->hasDuration() == i2->hasDuration() &&
    stringCompare( i1->description(), i2->description() ) &&
    stringCompare( i1->summary(), i2->summary() ) &&
    i1->categories() == i2->categories() &&
    i1->attachments() == i2->attachments() &&
    i1->secrecy() == i2->secrecy() &&
    i1->priority() == i2->priority() &&
    stringCompare( i1->location(), i2->location() );
}

static QStringList recipients( Attendee::List attendees )
{
  QStringList attStrList;
  for ( Attendee::List::ConstIterator it = attendees.begin(); it != attendees.end(); ++it ) {
    attStrList << (*it)->fullName();
  }
  return attStrList;
}

/* This function sends mails if necessary, and makes sure the user really
 * want to change his calendar.
 *
 * Return true means accept the changes
 * Return false means revert the changes
 */
bool KOGroupware::sendICalMessage( QWidget* parent,
                                   KCal::Scheduler::Method method,
                                   Incidence *incidence,
                                   Incidence *oldincidence,
                                   KOGlobals::HowChanged action,
                                   bool attendeeStatusChanged,
                                   bool useLastDialogAnswer )
{
  // If there are no attendees, don't bother
  if ( incidence->attendees().isEmpty() )
    return true;

  bool isOrganizer = KOPrefs::instance()->thatIsMe( incidence->organizer().email() );
  int rc = 0;
  /*
   * There are two scenarios:
   * o "we" are the organizer, where "we" means any of the identities or mail
   *   addresses known to Kontact/PIM. If there are attendees, we need to mail
   *   them all, even if one or more of them are also "us". Otherwise there
   *   would be no way to invite a resource or our boss, other identities we
   *   also manage.
   * o "we: are not the organizer, which means we changed the completion status
   *   of a todo, or we changed our attendee status from, say, tentative to
   *   accepted. In both cases we only mail the organizer. All other changes
   *   bring us out of sync with the organizer, so we won't mail, if the user
   *   insists on applying them.
   */

  if ( isOrganizer ) {
    // We are the organizer. If there is more than one attendee, or if there is
    // only one, and it's not the same as the organizer, ask the user to send mail.
    // Also send a mail if there were more then one attendees in the previous version
    // of the incidence.
    if ( incidence->attendees().count() > 1
        || ( oldincidence && oldincidence->attendees().count() > 1 )
        || incidence->attendees().first()->email() != incidence->organizer().email() ) {

      QString txt;
      switch( action ) {
      case KOGlobals::INCIDENCEEDITED:
      {
        bool sendUpdate = true;

        Attendee::List attendees = incidence->attendees();
        Attendee::List::ConstIterator it;

        // create the list of attendees from the new incidence
        Attendee::List sameAtts;
        for ( it = attendees.begin(); it != attendees.end(); ++it ) {
          if ( (*it)->email() != incidence->organizer().email() ) { //skip organizer
            sameAtts << *it;
          }
        }

        if ( incidence->summary().isEmpty() ) {
          incidence->setSummary( i18n( "<No summary given>" ) );
        }

        if ( oldincidence ) {
          // First we need to determine if the old and new incidences differ in
          // the attendee list only. If that's the case, then we need to get the
          // list of new attendees and the list of removed attendees and deal
          // with those lists separately.

          Attendee::List oldattendees = oldincidence->attendees();
          Attendee::List::ConstIterator ot;

          Attendee::List newAtts;  // newly added attendees
          Attendee::List remAtts;  // newly removed attendees
          sameAtts.clear();

          // make a list of newly added attendees and attendees in both old and new incidence.
          for ( it = attendees.begin(); it != attendees.end(); ++it ) {
            bool found = false;
            for ( ot = oldattendees.begin(); ot != oldattendees.end(); ++ot ) {
              if ( (*it)->email() == (*ot)->email() ) {
                found = true;
                break;
              }
            }
            if ( !found ) {
              newAtts << *it;
            } else {
              if ( (*it)->email() != incidence->organizer().email() ) { //skip organizer
                sameAtts << *it;
              }
            }
          }

          // make a list of newly removed attendees
          for ( ot = oldattendees.begin(); ot != oldattendees.end(); ++ot ) {
            bool found = false;
            for ( it = attendees.begin(); it != attendees.end(); ++it ) {
              if ( (*it)->email() == (*ot)->email() ) {
                found = true;
                break;
              }
            }
            if ( !found ) {
              remAtts << *ot;
            }
          }

          // let's see if any else changed from the old incidence
          if ( compareIncsExceptAttendees( incidence, oldincidence ) ) {
            // no change, so no need to send an update to the original attendees list.
            sendUpdate = false;
          }

          // For new attendees, send them the new incidence as a new invitation.
          if ( newAtts.count() > 0 ) {
            const QStringList rList = recipients( newAtts );
            int newMail = KMessageBox::questionYesNoList(
              parent,
              i18n( "You are adding new attendees to the invitation \"%1\".\n"
                    "Do you want to email an invitation to these new attendees?" ).
              arg( incidence->summary() ),
              rList,
              i18n( "New Attendees" ),
              KGuiItem( i18n( "Send Email" ) ), KGuiItem( i18n( "Do Not Send" ) ) );
            if ( newMail == KMessageBox::Yes ) {
              KCal::MailScheduler scheduler( mCalendar );
              incidence->setRevision( 0 );
              scheduler.performTransaction( incidence, Scheduler::Request, rList.join( "," ) );
            }
          }

          // For removed attendees, tell them they are toast and send them a cancel.
          if ( remAtts.count() > 0 ) {
            const QStringList rList = recipients( remAtts );
            int newMail = KMessageBox::questionYesNoList(
              parent,
              i18n( "You removed attendees from the invitation \"%1\".\n"
                    "Do you want to email a cancellation message to these attendees?" ).
              arg( incidence->summary() ),
              rList,
              i18n( "Removed Attendees" ),
              KGuiItem( i18n( "Send Email" ) ), KGuiItem( i18n( "Do Not Send" ) ) );
            if ( newMail == KMessageBox::Yes ) {
              KCal::MailScheduler scheduler( mCalendar );
              scheduler.performTransaction( incidence, Scheduler::Cancel, rList.join( "," ) );
            }
          }
        }

        // For existing attendees, skip the update if there are no other changes except attendees
        if ( sameAtts.count() > 0 ) {
          const QStringList rList = recipients( sameAtts );
          int newMail;
          if ( sendUpdate ) {
            newMail = KMessageBox::questionYesNoList(
              parent,
              i18n( "You changed the invitation \"%1\".\n"
                    "Do you want to email an updated invitation to these attendees?" ).
              arg( incidence->summary() ),
              rList,
              i18n( "Send Invitation Update" ),
              KGuiItem( i18n( "Send Email" ) ), KGuiItem( i18n( "Do Not Send" ) ) );
          } else {
            newMail = KMessageBox::questionYesNoList(
              parent,
              i18n( "You changed the invitation attendee list only.\n"
                    "Do you want to email an updated invitation showing the new attendees?" ),
              rList,
              i18n( "Send Invitation Update" ),
              KGuiItem( i18n( "Send Email" ) ), KGuiItem( i18n( "Do Not Send" ) ) );
          }
          if ( newMail == KMessageBox::Yes ) {
            KCal::MailScheduler scheduler( mCalendar );
            incidence->setRevision( incidence->revision() + 1 );
            scheduler.performTransaction( incidence, Scheduler::Request, rList.join( "," ) );
          }
        }
        return true;
      }

      case KOGlobals::INCIDENCEDELETED:
        Q_ASSERT( incidence->type() == "Event" || incidence->type() == "Todo" );
        if ( incidence->type() == "Event" ) {
          txt = i18n( "You removed the invitation \"%1\".\n"
                      "Do you want to email the attendees that the event is canceled?" ).
                arg( incidence->summary() );
        } else if ( incidence->type() == "Todo" ) {
          txt = i18n( "You removed the invitation \"%1\".\n"
                      "Do you want to email the attendees that the todo is canceled?" ).
                arg( incidence->summary() );
        }
        break;

      case KOGlobals::INCIDENCEADDED:
        if ( incidence->type() == "Event" ) {
          txt = i18n( "The event \"%1\" includes other people.\n"
                      "Do you want to email the invitation to the attendees?" ).
                arg( incidence->summary() );
        } else if ( incidence->type() == "Todo" ) {
          txt = i18n( "The todo \"%1\" includes other people.\n"
                      "Do you want to email the invitation to the attendees?" ).
                arg( incidence->summary() );
        } else {
          txt = i18n( "This incidence includes other people. "
                      "Should an email be sent to the attendees?" );
        }
        break;

      default:
        kdError() << "Unsupported HowChanged action" << int( action ) << endl;
        break;
      }

      if ( useLastDialogAnswer ) {
        rc = lastUsedDialogAnswer;
      } else {
        lastUsedDialogAnswer = rc = KMessageBox::questionYesNo(
          parent, txt, i18n( "Group Scheduling Email" ),
          KGuiItem( i18n( "Send Email" ) ), KGuiItem( i18n( "Do Not Send" ) ) );
      }
    } else {
      return true;
    }
  } else if( incidence->type() == "Todo" ) {
    QString txt;
    Todo *todo = static_cast<Todo *>( incidence );
    Todo *oldtodo = static_cast<Todo *>( oldincidence );
    if ( todo && oldtodo &&
         todo->percentComplete() != oldtodo->percentComplete() &&
         method == Scheduler::Request ) {
      txt = i18n( "Your completion status in this task has been changed. "
                  "Do you want to send a status update to the task organizer?" );
      method = Scheduler::Reply;
      if ( useLastDialogAnswer ) {
        rc = lastUsedDialogAnswer;
      } else {
        lastUsedDialogAnswer = rc = KMessageBox::questionYesNo(
          parent, txt, i18n( "Group Scheduling Email" ),
          KGuiItem( i18n( "Send Update" ) ), KGuiItem( i18n( "Do Not Send" ) ) );
      }
    } else if ( attendeeStatusChanged && method == Scheduler::Request ) {
      txt = i18n( "Your status as a participant in this task changed. "
                  "Do you want to send a status update to the task organizer?" );
      method = Scheduler::Reply;
      if ( useLastDialogAnswer ) {
        rc = lastUsedDialogAnswer;
      } else {
        lastUsedDialogAnswer = rc = KMessageBox::questionYesNo(
          parent, txt, i18n( "Group Scheduling Email" ),
          KGuiItem( i18n( "Send Update" ) ), KGuiItem( i18n( "Do Not Send" ) ) );
      }
    } else {
      if ( action == KOGlobals::INCIDENCEDELETED ) {
        const QStringList myEmails = KOPrefs::instance()->allEmails();
        bool askConfirmation = false;
        for ( QStringList::ConstIterator it = myEmails.begin(); it != myEmails.end(); ++it ) {
          QString email = *it;
          Attendee *me = incidence->attendeeByMail(email);
          if ( me &&
               ( me->status() == KCal::Attendee::Accepted ||
                 me->status() == KCal::Attendee::Delegated ) ) {
            askConfirmation = true;
            break;
          }
        }

        if ( !askConfirmation ) {
          return true;
        }

        txt = i18n( "You had previously accepted your participation in this task. "
                    "Do you want to send an updated response to the organizer "
                    "removing yourself from the task?" );
        if ( useLastDialogAnswer ) {
          rc = lastUsedDialogAnswer;
        } else {
          lastUsedDialogAnswer = rc = KMessageBox::questionYesNo(
            parent, txt, i18n( "Group Scheduling Email" ),
            KGuiItem( i18n( "Send Update" ) ), KGuiItem( i18n( "Do Not Send" ) ) );
          setDoNotNotify( rc == KMessageBox::No );
        }
        return true;
      } else {
        if ( useLastDialogAnswer ) {
          rc = lastUsedDialogAnswer;
        } else {
          if ( CalHelper::incOrganizerOwnsCalendar( mCalendar, incidence ) ) {
            txt = i18n( "<qt>"
                        "You are modifying the organizer's task. "
                        "Do you really want to edit it?<p>"
                        "If \"yes\", all the attendees will be emailed the updated invitation."
                        "</qt>" );
            lastUsedDialogAnswer = rc = KMessageBox::warningYesNo( parent, txt );
            if ( rc == KMessageBox::Yes ) {
              KCal::MailScheduler scheduler( mCalendar );
              scheduler.performTransaction( incidence, Scheduler::Request,
                                            recipients( incidence->attendees() ).join( "," ) );
            }
          } else {
            txt = i18n( "<qt>"
                        "You are not the organizer of this task. Editing it will "
                        "bring your invitation out of sync with the organizer's invitation. "
                        "Do you really want to edit it?<p>"
                        "If \"yes\", your local copy of the invitation will be different than "
                        "the organizer and any other task participants."
                        "</qt>" );
            lastUsedDialogAnswer = rc = KMessageBox::warningYesNo( parent, txt );
          }
        }
        return ( rc == KMessageBox::Yes );
      }
    }
  } else if ( incidence->type() == "Event" ) {
    QString txt;
    if ( attendeeStatusChanged && method == Scheduler::Request ) {
      txt = i18n( "Your status as an attendee of this event changed. "
                  "Do you want to send a status update to the event organizer?" );
      method = Scheduler::Reply;
      if ( useLastDialogAnswer ) {
        rc = lastUsedDialogAnswer;
      } else {
        lastUsedDialogAnswer = rc = KMessageBox::questionYesNo(
          parent, txt, i18n( "Group Scheduling Email" ),
          KGuiItem( i18n( "Send Update" ) ), KGuiItem( i18n( "Do Not Send" ) ) );
      }
    } else {
      if ( action == KOGlobals::INCIDENCEDELETED ) {
        const QStringList myEmails = KOPrefs::instance()->allEmails();
        bool askConfirmation = false;
        for ( QStringList::ConstIterator it = myEmails.begin(); it != myEmails.end(); ++it ) {
          QString email = *it;
          Attendee *me = incidence->attendeeByMail(email);
          if ( me &&
               ( me->status() == KCal::Attendee::Accepted ||
                 me->status() == KCal::Attendee::Delegated ) ) {
            askConfirmation = true;
            break;
          }
        }

        if ( !askConfirmation ) {
          return true;
        }

        txt = i18n( "You had previously accepted an invitation to this event. "
                    "Do you want to send an updated response to the organizer "
                    "declining the invitation?" );
        if ( useLastDialogAnswer ) {
          rc = lastUsedDialogAnswer;
        } else {
          lastUsedDialogAnswer = rc = KMessageBox::questionYesNo(
            parent, txt, i18n( "Group Scheduling Email" ),
            KGuiItem( i18n( "Send Update" ) ), KGuiItem( i18n( "Do Not Send" ) ) );
          setDoNotNotify( rc == KMessageBox::No );
        }
        return true;
      } else {
        if ( useLastDialogAnswer ) {
          rc = lastUsedDialogAnswer;
        } else {
          if ( CalHelper::incOrganizerOwnsCalendar( mCalendar, incidence ) ) {
            txt = i18n( "<qt>"
                        "You are modifying the organizer's event. "
                        "Do you really want to edit it?<p>"
                        "If \"yes\", all the attendees will be emailed the updated invitation."
                        "</qt>" );
            lastUsedDialogAnswer = rc = KMessageBox::warningYesNo( parent, txt );
            if ( rc == KMessageBox::Yes ) {
              KCal::MailScheduler scheduler( mCalendar );
              scheduler.performTransaction( incidence, Scheduler::Request,
                                            recipients( incidence->attendees() ).join( "," ) );
            }
          } else {
            txt = i18n( "<qt>"
                        "You are not the organizer of this event. Editing it will "
                        "bring your invitation out of sync with the organizer's invitation. "
                        "Do you really want to edit it?<p>"
                        "If \"yes\", your local copy of the invitation will be different than "
                        "the organizer and any other event participants."
                        "</qt>" );
            lastUsedDialogAnswer = rc = KMessageBox::warningYesNo( parent, txt );
          }
        }
        return ( rc == KMessageBox::Yes );
      }
    }
  } else {
    kdWarning(5850) << "Groupware messages for Journals are not implemented yet!" << endl;
    return true;
  }

  if ( rc == KMessageBox::Yes ) {
    // We will be sending out a message here. Now make sure there is
    // some summary
    if( incidence->summary().isEmpty() )
      incidence->setSummary( i18n("<No summary given>") );

    // Send the mail
    KCal::MailScheduler scheduler( mCalendar );
    scheduler.performTransaction( incidence, method );

    return true;
  } else if ( rc == KMessageBox::No ) {
    return true;
  } else {
    return false;
  }
}

void KOGroupware::sendCounterProposal(KCal::Calendar *calendar, KCal::Event * oldEvent, KCal::Event * newEvent) const
{
  if ( !oldEvent || !newEvent || *oldEvent == *newEvent || !KOPrefs::instance()->mUseGroupwareCommunication )
    return;
  if ( KOPrefs::instance()->outlookCompatCounterProposals() ) {
    Incidence* tmp = oldEvent->clone();
    tmp->setSummary( i18n("Counter proposal: %1").arg( newEvent->summary() ) );
    tmp->setDescription( newEvent->description() );
    tmp->addComment( i18n("Proposed new meeting time: %1 - %2").
                     arg( IncidenceFormatter::dateToString( newEvent->dtStart() ),
                          IncidenceFormatter::dateToString( newEvent->dtEnd() ) ) );
    KCal::MailScheduler scheduler( calendar );
    scheduler.performTransaction( tmp, Scheduler::Reply );
    delete tmp;
  } else {
    KCal::MailScheduler scheduler( calendar );
    scheduler.performTransaction( newEvent, Scheduler::Counter );
  }
}

#include "kogroupware.moc"
