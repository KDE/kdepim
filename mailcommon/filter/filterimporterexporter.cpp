/*
  Copyright (c) 2007 Till Adam <adam@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "filterimporterexporter.h"
#include "filterimporterexporter_p.h"
#include "filteraction.h"
#include "filtermanager.h"
#include "mailfilter.h"

#include "filterimporter/filterimporterthunderbird_p.h"
#include "filterimporter/filterimporterevolution_p.h"
#include "filterimporter/filterimportersylpheed_p.h"
#include "filterimporter/filterimporterprocmail_p.h"

#include <messageviewer/autoqpointer.h>
#include <messageviewer/util.h>

#include <KConfig>
#include <KDebug>
#include <KFileDialog>
#include <KListWidgetSearchLine>
#include <KMessageBox>
#include <KPushButton>

#include <QListWidget>
#include <QRegExp>
#include <QVBoxLayout>

using namespace MailCommon;

FilterSelectionDialog::FilterSelectionDialog( QWidget *parent )
  : KDialog( parent )
{
  setObjectName( "filterselection" );
  setModal( true );
  setCaption( i18n( "Select Filters" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );

  QVBoxLayout *const top = new QVBoxLayout( mainWidget() );

  filtersListWidget = new QListWidget();
  KListWidgetSearchLine *searchLine = new KListWidgetSearchLine( this, filtersListWidget );
  searchLine->setClickMessage(
    i18nc( "@info/plain Displayed grayed-out inside the textbox, verb to search",
           "Search" ) );

  top->addWidget( searchLine );
  top->addWidget( filtersListWidget );
  filtersListWidget->setAlternatingRowColors( true );
  filtersListWidget->setSortingEnabled( false );
  filtersListWidget->setSelectionMode( QAbstractItemView::NoSelection );

  QHBoxLayout *const buttonLayout = new QHBoxLayout();
  top->addLayout( buttonLayout );
  selectAllButton = new KPushButton( i18n( "Select All" ) );
  buttonLayout->addWidget( selectAllButton );
  unselectAllButton = new KPushButton( i18n( "Unselect All" ) );
  buttonLayout->addWidget( unselectAllButton );

  connect( selectAllButton, SIGNAL(clicked()), this, SLOT(slotSelectAllButton()) );
  connect( unselectAllButton, SIGNAL(clicked()), this, SLOT(slotUnselectAllButton()) );

  resize( 300, 350 );
}

FilterSelectionDialog::~FilterSelectionDialog()
{
}

void FilterSelectionDialog::reject()
{
   qDeleteAll(originalFilters);
   QDialog::reject();
}

void FilterSelectionDialog::setFilters( const QList<MailFilter *> &filters )
{
  if ( filters.isEmpty() ) {
    enableButtonOk( false );
    return;
  }

  originalFilters = filters;
  filtersListWidget->clear();

  foreach ( const MailFilter *filter, filters ) {
    QListWidgetItem *item = new QListWidgetItem( filter->name(), filtersListWidget );
    item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
    item->setCheckState( Qt::Checked );
  }
}

QList<MailFilter*> FilterSelectionDialog::selectedFilters() const
{
  QList<MailFilter*> filters;

  const int filterCount = filtersListWidget->count();
  for ( int i = 0; i < filterCount; ++i ) {
    const QListWidgetItem *item = filtersListWidget->item( i );
    if ( item->checkState() == Qt::Checked ) {
      filters << originalFilters[ i ];
    } else {
      delete originalFilters[ i ];
    }
  }

  return filters;
}

void FilterSelectionDialog::slotUnselectAllButton()
{
  const int filterCount = filtersListWidget->count();
  for ( int i = 0; i < filterCount; ++i ) {
    QListWidgetItem * const item = filtersListWidget->item( i );
    item->setCheckState( Qt::Unchecked );
  }
}

void FilterSelectionDialog::slotSelectAllButton()
{
  const int filterCount = filtersListWidget->count();
  for ( int i = 0; i < filterCount; ++i ) {
    QListWidgetItem * const item = filtersListWidget->item( i );
    item->setCheckState( Qt::Checked );
  }
}

QList<MailFilter*> FilterImporterExporter::readFiltersFromConfig(
  const KSharedConfig::Ptr config, QStringList &emptyFilters )
{
  const KConfigGroup group = config->group( "General" );

  const int numFilters = group.readEntry( "filters", 0 );

  bool filterNeedUpdate = false;
  QList<MailFilter*> filters;
  for ( int i = 0; i < numFilters; ++i ) {
    const QString groupName = QString::fromLatin1( "Filter #%1" ).arg( i );

    const KConfigGroup group = config->group( groupName );
    bool update = false;
    MailFilter *filter = new MailFilter( group, true/*interactive*/, update );
    filter->purify();
    if ( update ) {
      filterNeedUpdate = true;
    }
    if ( filter->isEmpty() ) {
#ifndef NDEBUG
      kDebug() << "Filter" << filter->asString() << "is empty!";
#endif
      emptyFilters << filter->name();
      delete filter;
    } else {
      filters.append( filter );
    }
  }
  if( filterNeedUpdate ) {
     KSharedConfig::Ptr config = KSharedConfig::openConfig( "akonadi_mailfilter_agentrc" );

     // Now, write out the new stuff:
     FilterImporterExporter::writeFiltersToConfig( filters, config );
     KConfigGroup group = config->group( "General" );
     group.sync();
  }
  return filters;
}

void FilterImporterExporter::writeFiltersToConfig( const QList<MailFilter*> &filters,
                                                   KSharedConfig::Ptr config,
                                                   bool exportFiler )
{
  // first, delete all filter groups:
  const QStringList filterGroups =
    config->groupList().filter( QRegExp( "Filter #\\d+" ) );

  foreach ( const QString &group, filterGroups ) {
    config->deleteGroup( group );
  }

  int i = 0;
  foreach ( const MailFilter *filter, filters ) {
    if ( !filter->isEmpty() ) {
      const QString groupName = QString::fromLatin1( "Filter #%1" ).arg( i );

      KConfigGroup group = config->group( groupName );
      filter->writeConfig( group, exportFiler );
      ++i;
    }
  }

  KConfigGroup group = config->group( "General" );
  group.writeEntry( "filters", i );

  config->sync();
}

class FilterImporterExporter::Private
{
  public:
    Private( QWidget *parent )
     : mParent( parent )
    {
    }
    void warningInfoAboutInvalidFilter( const QStringList &emptyFilters ) const;
    QWidget *mParent;
};

void FilterImporterExporter::Private::warningInfoAboutInvalidFilter(
  const QStringList &emptyFilters ) const
{
  if ( !emptyFilters.isEmpty() ) {
    KMessageBox::informationList(
      mParent,
      i18n( "The following filters have not been saved because they were invalid "
            "(e.g. containing no actions or no search rules)." ),
      emptyFilters,
      QString(),
      "ShowInvalidFilterWarning" );
  }
}

FilterImporterExporter::FilterImporterExporter( QWidget *parent )
  : d( new Private( parent ) )
{
}

FilterImporterExporter::~FilterImporterExporter()
{
  delete d;
}

QList<MailFilter *> FilterImporterExporter::importFilters(
  bool &canceled, FilterImporterExporter::FilterType type, const QString& filename )
{
  QString fileName( filename );

  if ( fileName.isEmpty() ) {
    QString title;
    QString defaultPath;
    switch(type){
    case KMailFilter:
      title = i18n( "Import KMail Filters" );
      defaultPath = QDir::homePath();
      break;
    case ThunderBirdFilter:
      title = i18n( "Import Thunderbird Filters" );
      defaultPath = MailCommon::FilterImporterThunderbird::defaultPath();
      break;
    case EvolutionFilter:
      title = i18n( "Import Evolution Filters" );
      defaultPath = MailCommon::FilterImporterEvolution::defaultPath();
      break;
    case SylpheedFilter:
      title = i18n( "Import Sylpheed Filters" );
      defaultPath = MailCommon::FilterImporterSylpheed::defaultPath();
      break;
    case ProcmailFilter:
      title = i18n( "Import Procmail Filters" );
      break;
    }

    fileName = KFileDialog::getOpenFileName(
      defaultPath, QString(), d->mParent, title );
    if ( fileName.isEmpty() ) {
      canceled = true;
      return QList<MailFilter*>(); // cancel
    }
  }
  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    KMessageBox::error(
      d->mParent,
      i18n( "The selected file is not readable. "
            "Your file access permissions might be insufficient." ) );
    return QList<MailFilter*>();
  }

  QList<MailFilter*> imported;
  QStringList emptyFilter;

  switch(type){
  case KMailFilter:
  {
    const KSharedConfig::Ptr config = KSharedConfig::openConfig( fileName );
    imported = readFiltersFromConfig( config, emptyFilter );
    break;
  }
  case ThunderBirdFilter:
  {
    MailCommon::FilterImporterThunderbird *thunderBirdFilter =
      new MailCommon::FilterImporterThunderbird( &file );

    imported = thunderBirdFilter->importFilter();
    emptyFilter = thunderBirdFilter->emptyFilter();
    delete thunderBirdFilter;
    break;
  }
  case EvolutionFilter:
  {
    MailCommon::FilterImporterEvolution *filter =
      new MailCommon::FilterImporterEvolution( &file );

    imported = filter->importFilter();
    emptyFilter = filter->emptyFilter();
    delete filter;
    break;
  }
  case SylpheedFilter:
  {
    MailCommon::FilterImporterSylpheed *filter =
      new MailCommon::FilterImporterSylpheed( &file );

    imported = filter->importFilter();
    emptyFilter = filter->emptyFilter();
    delete filter;
    break;
  }
  case ProcmailFilter:
  {
    MailCommon::FilterImporterProcmail *filter =
      new MailCommon::FilterImporterProcmail( &file );

    imported = filter->importFilter();
    emptyFilter = filter->emptyFilter();
    delete filter;
    break;
  }
  }
  d->warningInfoAboutInvalidFilter( emptyFilter );
  file.close();

  FilterSelectionDialog dlg( d->mParent );
  dlg.setFilters( imported );
  if ( dlg.exec() == QDialog::Accepted ) {
    return dlg.selectedFilters();
  }

  return QList<MailFilter*>();
}

void FilterImporterExporter::exportFilters( const QList<MailFilter*> &filters, const KUrl&fileName, bool saveAll )
{
  KUrl saveUrl;
  if(fileName.isEmpty()) {
    saveUrl = KFileDialog::getSaveUrl(
          QDir::homePath(), QString(), d->mParent, i18n( "Export Filters" ) );

    if ( saveUrl.isEmpty() ||
         !MessageViewer::Util::checkOverwrite( saveUrl, d->mParent ) ) {
      return;
    }
  } else {
    saveUrl= fileName;
  }

  KSharedConfig::Ptr config = KSharedConfig::openConfig( saveUrl.toLocalFile() );
  if(saveAll) {
    writeFiltersToConfig( filters, config, true );
  } else {
    MessageViewer::AutoQPointer<FilterSelectionDialog> dlg( new FilterSelectionDialog( d->mParent ) );
    dlg->setFilters( filters );
    if ( dlg->exec() == QDialog::Accepted && dlg ) {
      QList<MailFilter*> lst = dlg->selectedFilters();
      writeFiltersToConfig( lst, config, true );
      qDeleteAll(lst);
    }
  }
}

#include "filterimporterexporter_p.moc"
