/*
    This file is part of KOrganizer.
    Copyright (c) 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqpopupmenu.h>
#include <tqcursor.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>

#include <libkcal/calendar.h>


#include "kocore.h"
#include "koeventview.h"
#include "koeventpopupmenu.h"

using namespace KOrg;
#include "koeventview.moc"

//---------------------------------------------------------------------------

KOEventView::KOEventView(Calendar *cal,TQWidget *parent,const char *name)
  : KOrg::BaseView(cal,parent,name)
{
}

//---------------------------------------------------------------------------

KOEventView::~KOEventView()
{
}

//---------------------------------------------------------------------------

KOEventPopupMenu *KOEventView::eventPopup()
{
  KOEventPopupMenu *eventPopup = new KOEventPopupMenu;

  connect( eventPopup, TQT_SIGNAL(editIncidenceSignal(Incidence *,const TQDate &)),
           TQT_SIGNAL(editIncidenceSignal(Incidence *,const TQDate &)) );
  connect( eventPopup, TQT_SIGNAL(showIncidenceSignal(Incidence *,const TQDate &)),
           TQT_SIGNAL(showIncidenceSignal(Incidence *,const TQDate &)) );
  connect( eventPopup, TQT_SIGNAL(deleteIncidenceSignal(Incidence *)),
           TQT_SIGNAL(deleteIncidenceSignal(Incidence *)) );
  connect( eventPopup, TQT_SIGNAL(cutIncidenceSignal(Incidence *)),
           TQT_SIGNAL(cutIncidenceSignal(Incidence *)) );
  connect( eventPopup, TQT_SIGNAL(copyIncidenceSignal(Incidence *)),
           TQT_SIGNAL(copyIncidenceSignal(Incidence *)) );
  connect( eventPopup, TQT_SIGNAL(pasteIncidenceSignal()),
           TQT_SIGNAL(pasteIncidenceSignal()) );
  connect( eventPopup, TQT_SIGNAL(toggleAlarmSignal(Incidence *)),
           TQT_SIGNAL(toggleAlarmSignal(Incidence*)) );
  connect( eventPopup, TQT_SIGNAL(dissociateOccurrenceSignal(Incidence *,const TQDate &)),
           TQT_SIGNAL(dissociateOccurrenceSignal(Incidence *,const TQDate &)) );
  connect( eventPopup, TQT_SIGNAL(dissociateFutureOccurrenceSignal(Incidence *,const TQDate &)),
           TQT_SIGNAL(dissociateFutureOccurrenceSignal(Incidence *,const TQDate &)) );

  return eventPopup;
}

TQPopupMenu *KOEventView::newEventPopup()
{
  KXMLGUIClient *client = KOCore::self()->xmlguiClient( this );
  if ( !client ) {
    kdError() << "KOEventView::newEventPopup(): no xmlGuiClient." << endl;
    return 0;
  }
  if ( !client->factory() ) {
    kdError() << "KOEventView::newEventPopup(): no factory" << endl;
    return 0; // can happen if called too early
  }

  return static_cast<TQPopupMenu*>
      ( client->factory()->container( "rmb_selection_popup", client ) );
}
//---------------------------------------------------------------------------

void KOEventView::popupShow()
{
  emit showIncidenceSignal(mCurrentIncidence,  TQDate() );
}

//---------------------------------------------------------------------------

void KOEventView::popupEdit()
{
  emit editIncidenceSignal( mCurrentIncidence, TQDate() );
}

//---------------------------------------------------------------------------

void KOEventView::popupDelete()
{
  emit deleteIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupCut()
{
  emit cutIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupCopy()
{
  emit copyIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::showNewEventPopup()
{
  if ( !readOnly() ) {
    TQPopupMenu *popup = newEventPopup();
    if ( !popup ) {
      kdError() << "KOEventView::showNewEventPopup(): popup creation failed"
                << endl;
      return;
    }

    popup->popup( TQCursor::pos() );
  }
}

//---------------------------------------------------------------------------

void KOEventView::defaultAction( Incidence *incidence )
{
  kdDebug(5850) << "KOEventView::defaultAction()" << endl;

  if ( !incidence ) return;

  kdDebug(5850) << "  type: " << incidence->type() << endl;

  if ( incidence->isReadOnly() ) {
    emit showIncidenceSignal( incidence, TQDate() );
  } else {
    emit editIncidenceSignal( incidence, TQDate() );
  }
}

//---------------------------------------------------------------------------

#include "baseview.moc"

