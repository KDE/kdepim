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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "incidenceeditor.h"
#include "editorconfig.h"
#ifdef HAVE_QT3SUPPORT
#include "editordetails.h"
#endif
#include "designerfields.h"
#include "embeddedurlpage.h"
#include "templatemanagementdialog.h"

#include <akonadi/kcal/utils.h> //krazy:exclude=camelcase since kdepim/akonadi
#include <akonadi/kcal/kcalprefs.h> //krazy:exclude=camelcase since kdepim/akonadi

#include <libkdepimdbusinterfaces/urihandler.h>

#include <Akonadi/CollectionComboBox>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Monitor>

#include <KABC/Addressee>

#include <KCal/CalendarLocal>
#include <KCal/ICalFormat>

#include <KLocale>
#include <KMessageBox>
#include <KStandardDirs>
#include <KSystemTimeZones>

#include <QCloseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QTabWidget>
#include <QVBoxLayout>

using namespace Akonadi;
using namespace KCal;
using namespace IncidenceEditors;

IncidenceEditor::IncidenceEditor( const QString &caption, const QStringList& mimetypes, QWidget *parent )
  : KDialog( parent ), mAttendeeEditor( 0 ), mIsCounter( false ),
    mIsCreateTask( false ), mApplyClicked( false ), mMonitor( 0 )
{
  setCaption( caption );
  setButtons( Ok | Apply | Cancel | Default );
  setDefaultButton( Ok );
  enableButton( Ok, false );
  enableButton( Apply, false );
  setModal( false );
  showButtonSeparator( false );

  // Set this to be the group leader for all subdialogs - this means
  // modal subdialogs will only affect this dialog, not the other windows
  setAttribute( Qt::WA_GroupLeader );

  setButtonText( Default, i18nc( "@action:button", "Manage &Templates..." ) );
  setButtonToolTip( Default,
                    i18nc( "@info:tooltip",
                           "Apply or create templates for this item" ) );
  setButtonWhatsThis( Default,
                      i18nc( "@info:whatsthis",
                             "Push this button to run a tool that helps "
                             "you manage a set of templates. Templates "
                             "can make creating new items easier and faster "
                             "by putting your favorite default values into "
                             "the editor automatically." ) );

  QVBoxLayout *layout = new QVBoxLayout( mainWidget() );
  layout->setMargin( 0 );
  layout->setSpacing( 0 );
  mainWidget()->setLayout( layout );

  QHBoxLayout *callayout = new QHBoxLayout;
  callayout->setSpacing( KDialog::spacingHint() );
  mCalSelector = new Akonadi::CollectionComboBox( mainWidget() );
  mCalSelector->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
  //mCalSelector->setDefaultCollection( KCalPrefs::instance()->defaultCollection() );
  //mCalSelector->addExcludeResourcesType(QStringList()<<"akonadi_search_resource");
  mCalSelector->setMimeTypeFilter( QStringList() << mimetypes );
  connect( mCalSelector, SIGNAL(currentChanged(Akonadi::Collection)),
           SLOT(slotSelectedCollectionChanged()) );
  connect( mCalSelector->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
           SLOT(slotSelectedCollectionChanged()) );
  connect( mCalSelector->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
           SLOT(slotSelectedCollectionChanged()) );
  connect( mCalSelector->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
           SLOT(slotSelectedCollectionChanged()) );
  connect( mCalSelector->model(), SIGNAL(modelReset()),
           SLOT(slotSelectedCollectionChanged()) );

  QLabel *callabel = new QLabel( i18n( "Calendar:" ), mainWidget() );
  callabel->setBuddy( mCalSelector );
  callayout->addWidget( callabel );
  callayout->addWidget( mCalSelector, 1 );
  layout->addLayout( callayout );

  mTabWidget = new QTabWidget( mainWidget() );
  layout->addWidget( mTabWidget );

  connect( this, SIGNAL(defaultClicked()), SLOT(slotManageTemplates()) );
  connect( this, SIGNAL(finished()), SLOT(delayedDestruct()) );
}

IncidenceEditor::~IncidenceEditor()
{
}

void IncidenceEditor::readIncidence( const Akonadi::Item &item, const QDate &date, bool tmpl )
{
  if ( !read( item, date, tmpl ) ) {
    return;
  }

  Akonadi::Entity::Id colId( item.storageCollectionId() );
  Akonadi::Collection col( colId );
  if ( col.isValid() ) {
    mCalSelector->setDefaultCollection( col );
  }

  //TODO read-only ATM till we support moving of existing incidences from
  // one collection to another.
  mCalSelector->setEnabled( false );
}

void IncidenceEditor::editIncidence( const Akonadi::Item &item, const QDate &date )
{
  Incidence::Ptr incidence = Akonadi::incidence( item );
  Q_ASSERT( incidence );
  Q_ASSERT( incidence->type() == type() );

  init();

  if ( mIncidence.isValid() ) {
    Q_ASSERT( mMonitor );
    mMonitor->setItemMonitored( mIncidence, false );
  } else {
    Q_ASSERT( !mIncidence.hasPayload<Incidence::Ptr>() ); // not possible, right?
  }

  readIncidence( item, date, false );
  mIncidence = item;

  if ( !mMonitor ) {
    mMonitor = new Akonadi::Monitor( this );
    mMonitor->itemFetchScope().fetchFullPayload();
    //mMonitor->itemFetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
    connect( mMonitor, SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)),
             this, SLOT(slotItemChanged(Akonadi::Item)) );
    connect( mMonitor, SIGNAL(itemRemoved(Akonadi::Item)),
             this, SLOT(slotItemRemoved(Akonadi::Item)) );
    //connect( mMonitor, SIGNAL(itemMoved(Akonadi::Item,Akonadi::Collection,Akonadi::Collection)),
    //         this, SLOT(readIncidence(Akonadi::Item)) );
  }
  mMonitor->setItemMonitored( item, true );

  setCaption( i18nc( "@title:window",
                     "Edit %1: %2", QString( incidence->type() ), incidence->summary() ) );
}

bool IncidenceEditor::incidenceModified() {
  return true;
}

void IncidenceEditor::reload() {
  readIncidence( mIncidence, QDate(), true );
}

void IncidenceEditor::slotButtonClicked( int button )
{
  bool dontQuit = false;
  switch( button ) {
  case KDialog::Ok:
  {
    // "this" can be deleted before processInput() returns (processInput()
    // opens a non-modal dialog when Kolab is used). So accept should only
    // be executed when "this" is still valid
    QPointer<QWidget> ptr( this );
    if ( processInput() && ptr ) {
      KDialog::accept();
    }
    break;
  }
  case KDialog::Apply:
  {
    mApplyClicked = true;
    processInput();
    break;
  }
  case KDialog::Cancel:
    dontQuit = incidenceModified() &&
               KMessageBox::questionYesNo(
                 this,
                 i18nc( "@info", "Do you really want to cancel?" ),
                 i18nc( "@title:window", "KOrganizer Confirmation" ) ) == KMessageBox::No;

    if ( !dontQuit ) {
       processCancel();
       KDialog::reject();
     }
    break;
  default:
    KDialog::slotButtonClicked( button );
    break;
  }
}

void IncidenceEditor::setupAttendeesTab()
{
  QFrame *topFrame = new QFrame( this );
  mTabWidget->addTab( topFrame, i18nc( "@title:tab", "Atte&ndees" ) );
  topFrame->setWhatsThis(
    i18nc( "@info:whatsthis",
           "The Attendees tab allows you to Add or Remove "
           "Attendees to/from this event or to-do." ) );

  QVBoxLayout *topLayout = new QVBoxLayout( topFrame );
  topLayout->setMargin(0);

#ifdef HAVE_QT3SUPPORT
  mAttendeeEditor = mDetails = new EditorDetails( spacingHint(), topFrame );
  topLayout->addWidget( mDetails );
#endif
}

void IncidenceEditor::accept()
{
}

void IncidenceEditor::reject()
{
}

void IncidenceEditor::closeEvent( QCloseEvent *event )
{
  event->ignore();
  slotButtonClicked( KDialog::Cancel );
}

void IncidenceEditor::cancelRemovedAttendees( const Akonadi::Item &item )
{
  const Incidence::Ptr incidence = Akonadi::incidence( item );
  if ( !incidence ) {
    return;
  }

  // cancelAttendeeIncidence removes all attendees from the incidence,
  // and then only adds those that need to be canceled (i.e. a mail needs to be sent to them).
  const bool thatIsMe = EditorConfig::instance()->thatIsMe( incidence->organizer().email() );
  if ( thatIsMe ) {
    Incidence::Ptr inc( incidence->clone() );
    inc->registerObserver( 0 );
#ifdef HAVE_QT3SUPPORT
    mAttendeeEditor->cancelAttendeeIncidence( inc.get() );
#endif
    if ( inc->attendeeCount() > 0 ) {
      emit deleteAttendee( item );
    }
  }
}

void IncidenceEditor::slotSelectedCollectionChanged()
{
  const bool enabled = mCalSelector->currentCollection().isValid();
  enableButton( Ok, enabled );
  enableButton( Apply, enabled );
}

void IncidenceEditor::slotItemChanged( const Akonadi::Item &item )
{
  kDebug();
  Q_ASSERT( item == mIncidence );
  //Don't display messagebox when we click on apply, you know what you do
  //Bug #228526
  if ( !mApplyClicked ) {
    KMessageBox::information(
      this,
      i18nc( "@info", "The incidence got changed. Reloading editor now." ) );
  }
  else
    mApplyClicked = false;
  readIncidence( item, QDate() );
}

void IncidenceEditor::slotItemRemoved( const Akonadi::Item &item )
{
  kDebug();
  Q_ASSERT( item == mIncidence );
  KMessageBox::information(
    this,
    i18nc( "@info", "The incidence got removed. Closing editor now." ) );
  KDialog::reject();
}

void IncidenceEditor::slotManageTemplates()
{
  QStringList &templates = EditorConfig::instance()->templates( type() );
  TemplateManagementDialog *const d = new TemplateManagementDialog( this, templates, type() );
  connect( d, SIGNAL( loadTemplate( const QString& ) ),
           this, SLOT( slotLoadTemplate( const QString& ) ) );
  connect( d, SIGNAL( templatesChanged( const QStringList& ) ),
           this, SLOT( slotTemplatesChanged( const QStringList& ) ) );
  connect( d, SIGNAL( saveTemplate( const QString& ) ),
           this, SLOT( slotSaveTemplate( const QString& ) ) );
  d->exec();
  delete d;
}

void IncidenceEditor::slotLoadTemplate( const QString &templateName )
{
  CalendarLocal cal( KSystemTimeZones::local() );
  QString fileName = KStandardDirs::locateLocal( "data",
                       "korganizer/templates/" + type() + '/' + templateName );

  if ( fileName.isEmpty() ) {
    KMessageBox::error( this,
                        i18nc( "@info", "Unable to find template '%1'.", fileName ) );
    return;
  }

  ICalFormat format;
  if ( !format.load( &cal, fileName ) ) {
    KMessageBox::error( this,
                        i18nc( "@info", "Error loading template file '%1'.", fileName ) );
    return;
  }

  Incidence::List incidences = cal.incidences();
  if ( incidences.isEmpty() ) {
    KMessageBox::error( this,
                        i18nc( "@info", "Template does not contain a valid incidence." ) );
    return;
  }

  Incidence *incidence = incidences.first();
  Akonadi::Item incidenceItem;
  incidenceItem.setPayload( Incidence::Ptr( incidence->clone() ) );
  readIncidence( incidenceItem, QDate(), true );
}

void IncidenceEditor::slotSaveTemplate( const QString &templateName )
{
  Q_ASSERT( ! templateName.isEmpty() );
  Q_ASSERT( mIncidence.isValid() );
  Q_ASSERT( mIncidence.hasPayload<Incidence::Ptr>() );
  Incidence::Ptr incidence = mIncidence.payload<Incidence::Ptr>();

  QString fileName = "templates/" + incidence->type();
  fileName.append( '/' + templateName );
  fileName = KStandardDirs::locateLocal( "data", "korganizer/" + fileName );

  CalendarLocal cal( KSystemTimeZones::local() );
  cal.addIncidence( incidence->clone() );
  ICalFormat format;
  format.save( &cal, fileName );
}

void IncidenceEditor::slotTemplatesChanged( const QStringList &templateNames )
{
  QStringList &templates = EditorConfig::instance()->templates( type() );
  templates = templateNames;
}

void IncidenceEditor::setupDesignerTabs( const QString &type )
{
  QStringList activePages =  EditorConfig::instance()->activeDesignerFields();

  QStringList list = KGlobal::dirs()->findAllResources(
    "data", "korganizer/designer/" + type + "/*.ui",
    KStandardDirs::Recursive |KStandardDirs::NoDuplicates );

  for ( QStringList::iterator it = list.begin(); it != list.end(); ++it ) {
    const QString &fn = (*it).mid( (*it).lastIndexOf( '/' ) + 1 );
    if ( activePages.contains( fn ) ) {
      addDesignerTab( *it );
    }
  }
}

QWidget *IncidenceEditor::addDesignerTab( const QString &uifile )
{
  DesignerFields *wid = new DesignerFields( uifile, 0 );
  mDesignerFields.append( wid );

  QFrame *topFrame = new QFrame();
  mTabWidget->addTab( topFrame, wid->title() );

  QVBoxLayout *topLayout = new QVBoxLayout( topFrame );
  topLayout->setMargin(0);

  wid->setParent( topFrame );
  topLayout->addWidget( wid );
  mDesignerFieldForWidget[topFrame] = wid;

  return topFrame;
}

class KCalStorage : public DesignerFields::Storage
{
  public:
    KCalStorage( Incidence *incidence )
      : mIncidence( incidence )
    {
    }

    QStringList keys()
    {
      QStringList keys;

      QMap<QByteArray, QString> props = mIncidence->customProperties();
      QMap<QByteArray, QString>::ConstIterator it;
      for ( it = props.constBegin(); it != props.constEnd(); ++it ) {
        QString customKey = it.key();
        QStringList parts = customKey.split( '-', QString::SkipEmptyParts );
        if ( parts.count() != 4 ) {
          continue;
        }
        if ( parts[2] != "KORGANIZER" ) {
          continue;
        }
        keys.append( parts[3] );
      }

      return keys;
    }

    QString read( const QString &key )
    {
      return mIncidence->customProperty( "KORGANIZER", key.toUtf8() );
    }

    void write( const QString &key, const QString &value )
    {
      mIncidence->setCustomProperty( "KORGANIZER", key.toUtf8(), value );
    }

  private:
    Incidence *mIncidence;
};

void IncidenceEditor::readDesignerFields( const Item &i )
{
  KCalStorage storage( Akonadi::incidence( i ).get() );
  foreach ( DesignerFields *fields, mDesignerFields ) {
    if ( fields ) {
      fields->load( &storage );
    }
  }
}

void IncidenceEditor::writeDesignerFields( Incidence *i )
{
  KCalStorage storage( i );
  foreach ( DesignerFields *fields, mDesignerFields ) {
    if ( fields ) {
      fields->save( &storage );
    }
  }
}

void IncidenceEditor::setupEmbeddedURLPage( const QString &label,
                                              const QString &url,
                                              const QString &mimetype )
{
  QFrame *topFrame = new QFrame();
  mTabWidget->addTab( topFrame, label );

  QVBoxLayout *topLayout = new QVBoxLayout( topFrame );
  topLayout->setMargin(0);

  EmbeddedURLPage *wid = new EmbeddedURLPage( url, mimetype, topFrame );
  topLayout->addWidget( wid );
  mEmbeddedURLPages.append( topFrame );
  connect( wid, SIGNAL(openURL(const KUrl &)),
           this, SLOT(openURL(const KUrl &)) );
  // TODO: Call this method only when the tab is actually activated!
  wid->loadContents();
}

void IncidenceEditor::createEmbeddedURLPages( const Incidence *i )
{
  if ( !i ) return;
  if ( !mEmbeddedURLPages.isEmpty() ) {
    qDeleteAll( mEmbeddedURLPages );
    mEmbeddedURLPages.clear();
  }
  if ( !mAttachedDesignerFields.isEmpty() ) {
    for ( QList<QWidget*>::Iterator it = mAttachedDesignerFields.begin();
          it != mAttachedDesignerFields.end(); ++it ) {
      if ( mDesignerFieldForWidget.contains( *it ) ) {
        mDesignerFields.removeAll( mDesignerFieldForWidget[*it] );
      }
    }
    qDeleteAll( mAttachedDesignerFields );
    mAttachedDesignerFields.clear();
  }

  Attachment::List att = i->attachments();
  for ( Attachment::List::Iterator it = att.begin(); it != att.end(); ++it ) {
    Attachment *a = (*it);
    if ( a->showInline() && a->isUri() ) {
      // TODO: Allow more mime-types, but add security checks!
//       if ( a->mimeType() == QLatin1String("application/x-designer") ) {
//         QString tmpFile;
//         if ( KIO::NetAccess::download( a->uri(), tmpFile, this ) ) {
//           mAttachedDesignerFields.append( addDesignerTab( tmpFile ) );
//           KIO::NetAccess::removeTempFile( tmpFile );
//         }
//       } else
      // TODO: Enable that check again!
      if ( a->mimeType() == QLatin1String( "text/html" ) ) {
        setupEmbeddedURLPage( a->label(), a->uri(), a->mimeType() );
      }
    }
  }
}

void IncidenceEditor::openURL( const KUrl &url )
{
  QString uri = url.url();
  UriHandler::process( uri );
}

void IncidenceEditor::addAttachments( const QStringList &attachments,
                                        const QStringList &mimeTypes,
                                        bool inlineAttachments )
{
  emit signalAddAttachments( attachments, mimeTypes, inlineAttachments );
}

void IncidenceEditor::addAttendees( const QStringList &attendees )
{
  QStringList::ConstIterator it;
  for ( it = attendees.begin(); it != attendees.end(); ++it ) {
    QString name, email;
    KABC::Addressee::parseEmailAddress( *it, name, email );
#ifdef HAVE_QT3SUPPORT
    mAttendeeEditor->insertAttendee( new Attendee( name, email, true, Attendee::NeedsAction ) );
#endif
  }
}

void IncidenceEditor::selectCreateTask( bool enable )
{
  mIsCreateTask = enable;
  if ( mIsCreateTask ) {
    setCaption( i18nc( "@title:window", "Create to-do" ) );
    setButtonText( KDialog::Ok, i18nc( "@action:button", "Create to-do" ) );
    showButton( KDialog::Apply, false );
  }
}

void IncidenceEditor::selectInvitationCounterProposal( bool enable )
{
  mIsCounter = enable;
  if ( mIsCounter ) {
    setCaption( i18nc( "@title:window", "Counter proposal" ) );
    setButtonText( KDialog::Ok, i18nc( "@action:button", "Counter proposal" ) );
    showButton( KDialog::Apply, false );
  }
}

void IncidenceEditor::selectCollection( const Akonadi::Collection &collection )
{
  mCalSelector->setDefaultCollection( collection );
}

#include "incidenceeditor.moc"
