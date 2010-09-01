/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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

#include <tqtooltip.h>
#include <tqframe.h>
#include <tqguardedptr.h>
#include <tqpixmap.h>
#include <tqlayout.h>
#include <tqwidgetstack.h>
#include <tqdatetime.h>
#include <tqwhatsthis.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kio/netaccess.h>
#include <kabc/addressee.h>

#include <libkdepim/designerfields.h>
#include <libkdepim/embeddedurlpage.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/incidence.h>
#include <libkcal/icalformat.h>
#include <libkcal/resourcecalendar.h>

#include "koprefs.h"
#include "koglobals.h"
#include "koeditordetails.h"
#include "koeditoralarms.h"
#include "urihandler.h"
#include "koincidenceeditor.h"
#include "templatemanagementdialog.h"

KOIncidenceEditor::KOIncidenceEditor( const TQString &caption,
                                      Calendar *calendar, TQWidget *parent )
  : KDialogBase( Tabbed, caption, Ok | Apply | Cancel | Default, Ok,
                 parent, 0, false, false ),
    mAttendeeEditor( 0 ), mResource( 0 ), mIsCounter( false ), mIsCreateTask( false ),
    mRecurIncidence( 0 ), mRecurIncidenceAfterDissoc( 0 )
{
  // Set this to be the group leader for all subdialogs - this means
  // modal subdialogs will only affect this dialog, not the other windows
  setWFlags( getWFlags() | WGroupLeader );

  mCalendar = calendar;

  if ( KOPrefs::instance()->mCompactDialogs ) {
    showButton( Apply, false );
    showButton( Default, false );
  } else {
    setButtonText( Default, i18n("&Templates...") );
  }

  connect( this, TQT_SIGNAL( defaultClicked() ), TQT_SLOT( slotManageTemplates() ) );
  connect( this, TQT_SIGNAL( finished() ), TQT_SLOT( delayedDestruct() ) );
}

KOIncidenceEditor::~KOIncidenceEditor()
{
}

void KOIncidenceEditor::setupAttendeesTab()
{
  TQFrame *topFrame = addPage( i18n("Atte&ndees") );
  TQWhatsThis::add( topFrame,
                   i18n("The Attendees tab allows you to Add or Remove "
                        "Attendees to/from this event or to-do.") );

  TQBoxLayout *topLayout = new TQVBoxLayout( topFrame );

  mAttendeeEditor = mDetails = new KOEditorDetails( spacingHint(), topFrame );
  topLayout->addWidget( mDetails );
}

void KOIncidenceEditor::slotApply()
{
  processInput();
}

void KOIncidenceEditor::slotOk()
{
  // "this" can be deleted before processInput() returns (processInput() opens
  // a non-modal dialog when Kolab is used). So accept should only be executed
  // when "this" is still valid
  TQGuardedPtr<TQWidget> ptr( this );
  if ( processInput() && ptr ) accept();
}

void KOIncidenceEditor::slotCancel()
{
  processCancel();
  reject();
}

void KOIncidenceEditor::cancelRemovedAttendees( Incidence *incidence )
{
  if ( !incidence ) return;

  // cancelAttendeeEvent removes all attendees from the incidence,
  // and then only adds those that need to be cancelled (i.e. a mail needs to be sent to them).
  if ( KOPrefs::instance()->thatIsMe( incidence->organizer().email() ) ) {
    Incidence *ev = incidence->clone();
    ev->registerObserver( 0 );
    mAttendeeEditor->cancelAttendeeEvent( ev );
    if ( ev->attendeeCount() > 0 ) {
      emit deleteAttendee( ev );
    }
    delete( ev );
  }

}

void KOIncidenceEditor::slotManageTemplates()
{
  kdDebug(5850) << "KOIncidenceEditor::manageTemplates()" << endl;

  TemplateManagementDialog * const d = new TemplateManagementDialog( this, templates() );
  connect( d, TQT_SIGNAL( loadTemplate( const TQString& ) ),
           this, TQT_SLOT( slotLoadTemplate( const TQString& ) ) );
  connect( d, TQT_SIGNAL( templatesChanged( const TQStringList& ) ),
           this, TQT_SLOT( slotTemplatesChanged( const TQStringList& ) ) );
  connect( d, TQT_SIGNAL( saveTemplate( const TQString& ) ),
           this, TQT_SLOT( slotSaveTemplate( const TQString& ) ) );
  d->exec();
  return;
}

void KOIncidenceEditor::saveAsTemplate( Incidence *incidence,
                                        const TQString &templateName )
{
  if ( !incidence || templateName.isEmpty() ) return;

  TQString fileName = "templates/" + incidence->type();
  fileName.append( "/" + templateName );
  fileName = locateLocal( "data", "korganizer/" + fileName );

  CalendarLocal cal( KOPrefs::instance()->mTimeZoneId );
  cal.addIncidence( incidence );
  ICalFormat format;
  format.save( &cal, fileName );
}

void KOIncidenceEditor::slotLoadTemplate( const TQString& templateName )
{
  CalendarLocal cal( KOPrefs::instance()->mTimeZoneId );
  TQString fileName = locateLocal( "data", "korganizer/templates/" + type() + "/" +
      templateName );

  if ( fileName.isEmpty() ) {
    KMessageBox::error( this, i18n("Unable to find template '%1'.")
        .arg( fileName ) );
  } else {
    ICalFormat format;
    if ( !format.load( &cal, fileName ) ) {
      KMessageBox::error( this, i18n("Error loading template file '%1'.")
          .arg( fileName ) );
      return;
    }
  }
  loadTemplate( cal );
}

void KOIncidenceEditor::slotTemplatesChanged( const TQStringList& newTemplates )
{
  templates() = newTemplates;
}

void KOIncidenceEditor::setupDesignerTabs( const TQString &type )
{
  TQStringList activePages = KOPrefs::instance()->activeDesignerFields();

  TQStringList list = KGlobal::dirs()->findAllResources( "data",
    "korganizer/designer/" + type + "/*.ui", true, true );
  for ( TQStringList::iterator it = list.begin(); it != list.end(); ++it ) {
    const TQString &fn = (*it).mid( (*it).findRev('/') + 1 );
    if ( activePages.find( fn ) != activePages.end() ) {
      addDesignerTab( *it );
    }
  }
}

TQWidget *KOIncidenceEditor::addDesignerTab( const TQString &uifile )
{
  kdDebug(5850) << "Designer tab: " << uifile << endl;

  KPIM::DesignerFields *wid = new KPIM::DesignerFields( uifile, 0 );
  mDesignerFields.append( wid );

  TQFrame *topFrame = addPage( wid->title() );

  TQBoxLayout *topLayout = new TQVBoxLayout( topFrame );

  wid->reparent( topFrame, 0, TQPoint() );
  topLayout->addWidget( wid );
  mDesignerFieldForWidget[ topFrame ] = wid;

  return topFrame;
}

class KCalStorage : public KPIM::DesignerFields::Storage
{
  public:
    KCalStorage( Incidence *incidence )
      : mIncidence( incidence )
    {
    }

    TQStringList keys()
    {
      TQStringList keys;

      TQMap<TQCString, TQString> props = mIncidence->customProperties();
      TQMap<TQCString, TQString>::ConstIterator it;
      for( it = props.begin(); it != props.end(); ++it ) {
        TQString customKey = it.key();
        TQStringList parts = TQStringList::split( "-", customKey );
        if ( parts.count() != 4 ) continue;
        if ( parts[ 2 ] != "KORGANIZER" ) continue;
        keys.append( parts[ 3 ] );
      }

      return keys;
    }

    TQString read( const TQString &key )
    {
      return mIncidence->customProperty( "KORGANIZER", key.utf8() );
    }

    void write( const TQString &key, const TQString &value )
    {
      mIncidence->setCustomProperty( "KORGANIZER", key.utf8(), value );
    }

  private:
    Incidence *mIncidence;
};

void KOIncidenceEditor::readDesignerFields( Incidence *i )
{
  KCalStorage storage( i );
  KPIM::DesignerFields *fields;
  for( fields = mDesignerFields.first(); fields;
       fields = mDesignerFields.next() ) {
    fields->load( &storage );
  }
}

void KOIncidenceEditor::writeDesignerFields( Incidence *i )
{
  kdDebug(5850) << "KOIncidenceEditor::writeDesignerFields()" << endl;

  KCalStorage storage( i );
  KPIM::DesignerFields *fields;
  for( fields = mDesignerFields.first(); fields;
       fields = mDesignerFields.next() ) {
    kdDebug(5850) << "Write Field " << fields->title() << endl;
    fields->save( &storage );
  }
}


void KOIncidenceEditor::setupEmbeddedURLPage( const TQString &label,
                                 const TQString &url, const TQString &mimetype )
{
  kdDebug(5850) << "KOIncidenceEditor::setupEmbeddedURLPage()" << endl;
  kdDebug(5850) << "label=" << label << ", url=" << url << ", mimetype=" << mimetype << endl;
  TQFrame *topFrame = addPage( label );
  TQBoxLayout *topLayout = new TQVBoxLayout( topFrame );

  KPIM::EmbeddedURLPage *wid = new KPIM::EmbeddedURLPage( url, mimetype,
                                                          topFrame );
  topLayout->addWidget( wid );
  mEmbeddedURLPages.append( topFrame );
  connect( wid, TQT_SIGNAL( openURL( const KURL & ) ) ,
           this, TQT_SLOT( openURL( const KURL & ) ) );
  // TODO: Call this method only when the tab is actually activated!
  wid->loadContents();
}

void KOIncidenceEditor::createEmbeddedURLPages( Incidence *i )
{
  kdDebug(5850) << "KOIncidenceEditor::createEmbeddedURLPages()" << endl;

  if ( !i ) return;
  if ( !mEmbeddedURLPages.isEmpty() ) {
    kdDebug(5850) << "mEmbeddedURLPages are not empty, clearing it!" << endl;
    mEmbeddedURLPages.setAutoDelete( true );
    mEmbeddedURLPages.clear();
    mEmbeddedURLPages.setAutoDelete( false );
  }
  if ( !mAttachedDesignerFields.isEmpty() ) {
    for ( TQPtrList<TQWidget>::Iterator it = mAttachedDesignerFields.begin();
          it != mAttachedDesignerFields.end(); ++it ) {
      if ( mDesignerFieldForWidget.contains( *it ) ) {
        mDesignerFields.remove( mDesignerFieldForWidget[ *it ] );
      }
    }
    mAttachedDesignerFields.setAutoDelete( true );
    mAttachedDesignerFields.clear();
    mAttachedDesignerFields.setAutoDelete( false );
  }

  Attachment::List att = i->attachments();
  for ( Attachment::List::Iterator it = att.begin(); it != att.end(); ++it ) {
    Attachment *a = (*it);
    kdDebug(5850) << "Iterating over the attachments " << endl;
    kdDebug(5850) << "label=" << a->label() << ", url=" << a->uri() << ", mimetype=" << a->mimeType() << endl;
    if ( a && a->showInline() && a->isUri() ) {
      // TODO: Allow more mime-types, but add security checks!
/*      if ( a->mimeType() == "application/x-designer" ) {
        TQString tmpFile;
        if ( KIO::NetAccess::download( a->uri(), tmpFile, this ) ) {
          mAttachedDesignerFields.append( addDesignerTab( tmpFile ) );
          KIO::NetAccess::removeTempFile( tmpFile );
        }
      } else*/
      // TODO: Enable that check again!
      if ( a->mimeType() == "text/html" )
      {
        setupEmbeddedURLPage( a->label(), a->uri(), a->mimeType() );
      }
    }
  }
}

void KOIncidenceEditor::openURL( const KURL &url )
{
  TQString uri = url.url();
  UriHandler::process( this, uri );
}

void KOIncidenceEditor::addAttachments( const TQStringList &attachments,
                                        const TQStringList &mimeTypes,
                                        bool inlineAttachments )
{
  emit signalAddAttachments( attachments, mimeTypes, inlineAttachments );
}

void KOIncidenceEditor::addAttendees( const TQStringList &attendees )
{
  TQStringList::ConstIterator it;
  for ( it = attendees.begin(); it != attendees.end(); ++it ) {
    TQString name, email;
    KABC::Addressee::parseEmailAddress( *it, name, email );
    mAttendeeEditor->insertAttendee( new Attendee( name, email, true, Attendee::NeedsAction ) );
  }
}

void KOIncidenceEditor::setResource( ResourceCalendar *res, const TQString &subRes )
{
  TQString label;
  if ( res ) {
    if ( !res->subresources().isEmpty() && !subRes.isEmpty() ) {
      label = res->labelForSubresource( subRes );
    } else {
      label = res->resourceName();
    }
  }

  mResource = res;
  mSubResource = subRes;
}


void KOIncidenceEditor::selectCreateTask( bool enable )
{
  mIsCreateTask = enable;
  if ( mIsCreateTask ) {
    setCaption( i18n( "Create to-do" ) );
    setButtonOK( i18n( "Create to-do" ) );
    showButtonApply( false );
  }
}

void KOIncidenceEditor::selectInvitationCounterProposal(bool enable)
{
  mIsCounter = enable;
  if ( mIsCounter ) {
    setCaption( i18n( "Counter proposal" ) );
    setButtonOK( i18n( "Counter proposal" ) );
    showButtonApply( false );
  }
}

void KOIncidenceEditor::setRecurringIncidence ( Incidence *originalIncidence,
                                                Incidence *incAfterDissociation )
{
  mRecurIncidence = originalIncidence;
  mRecurIncidenceAfterDissoc = incAfterDissociation;
}


#include "koincidenceeditor.moc"
