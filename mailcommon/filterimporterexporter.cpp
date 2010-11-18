/*
    This file is part of KMail.
    Copyright (c) 2007 Till Adam <adam@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
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
#include "mailfilter.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <messageviewer/autoqpointer.h>
#include <messageviewer/util.h>

#include <QtCore/QRegExp>
#include <QtGui/QListWidget>
#include <QtGui/QVBoxLayout>

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

  QVBoxLayout* const top = new QVBoxLayout( mainWidget() );
  filtersListWidget = new QListWidget();
  top->addWidget( filtersListWidget );
  filtersListWidget->setAlternatingRowColors( true );
  filtersListWidget->setSortingEnabled( false );
  filtersListWidget->setSelectionMode( QAbstractItemView::NoSelection );

  QHBoxLayout* const buttonLayout = new QHBoxLayout();
  top->addLayout( buttonLayout );
  selectAllButton = new KPushButton( i18n( "Select All" ) );
  buttonLayout->addWidget( selectAllButton );
  unselectAllButton = new KPushButton( i18n( "Unselect All" ) );
  buttonLayout->addWidget( unselectAllButton );

  connect( selectAllButton, SIGNAL( clicked() ), this, SLOT( slotSelectAllButton() ) );
  connect( unselectAllButton, SIGNAL( clicked() ), this, SLOT( slotUnselectAllButton() ) );

  resize( 300, 350 );
}

FilterSelectionDialog::~FilterSelectionDialog()
{
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
    if ( item->checkState() == Qt::Checked )
      filters << originalFilters[ i ];
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

QList<MailFilter*> FilterImporterExporter::readFiltersFromConfig( const KSharedConfig::Ptr config )
{
  const KConfigGroup group = config->group( "General" );

  const int numFilters = group.readEntry( "filters", 0 );

  QList<MailFilter*> filters;
  for ( int i = 0; i < numFilters; ++i ) {
    const QString groupName = QString::fromLatin1( "Filter #%1" ).arg( i );

    const KConfigGroup group = config->group( groupName );
    MailFilter *filter = new MailFilter( group );
    filter->purify();
    if ( filter->isEmpty() ) {
#ifndef NDEBUG
      kDebug() << "Filter" << filter->asString() << "is empty!";
#endif
      delete filter;
    } else
      filters.append( filter );
  }

  return filters;
}

void FilterImporterExporter::writeFiltersToConfig( const QList<MailFilter*> &filters, KSharedConfig::Ptr config )
{
  // first, delete all filter groups:
  const QStringList filterGroups =
    config->groupList().filter( QRegExp( "Filter #\\d+" ) );

  foreach ( const QString &group, filterGroups )
    config->deleteGroup( group );

  int i = 0;
  foreach ( const MailFilter *filter, filters ) {
    if ( !filter->isEmpty() ) {
      const QString groupName = QString::fromLatin1( "Filter #%1" ).arg( i );

      KConfigGroup group = config->group( groupName );
      filter->writeConfig( group );
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
     : mParent( parent)
    {
    }

    QWidget *mParent;
};


FilterImporterExporter::FilterImporterExporter( QWidget *parent )
  : d( new Private( parent ) )
{
}

FilterImporterExporter::~FilterImporterExporter()
{
  delete d;
}

QList<MailFilter *> FilterImporterExporter::importFilters()
{
  const QString fileName = KFileDialog::getOpenFileName( QDir::homePath(), QString(),
                                                         d->mParent, i18n( "Import Filters" ) );
  if ( fileName.isEmpty() )
    return QList<MailFilter*>(); // cancel

  {
    QFile file( fileName );
    if ( !file.open( QIODevice::ReadOnly ) ) {
      KMessageBox::error( d->mParent,
                          i18n( "The selected file is not readable. "
                                "Your file access permissions might be insufficient.") );
      return QList<MailFilter*>();
    }
  }

  const KSharedConfig::Ptr config = KSharedConfig::openConfig( fileName );
  const QList<MailFilter*> imported = readFiltersFromConfig( config );

  FilterSelectionDialog dlg( d->mParent );
  dlg.setFilters( imported );

  return (dlg.exec() == QDialog::Accepted ? dlg.selectedFilters() : QList<MailFilter*>());
}

void FilterImporterExporter::exportFilters( const QList<MailFilter*> &filters )
{
  const KUrl saveUrl = KFileDialog::getSaveUrl( QDir::homePath(), QString(),
                                                d->mParent, i18n( "Export Filters" ) );

  if ( saveUrl.isEmpty() || !MessageViewer::Util::checkOverwrite( saveUrl, d->mParent ) )
      return;

  KSharedConfig::Ptr config = KSharedConfig::openConfig( saveUrl.toLocalFile() );
  MessageViewer::AutoQPointer<FilterSelectionDialog> dlg( new FilterSelectionDialog( d->mParent ) );
  dlg->setFilters( filters );
  if ( dlg->exec() == QDialog::Accepted && dlg )
    writeFiltersToConfig( dlg->selectedFilters(), config );
}

#include "filterimporterexporter_p.moc"
