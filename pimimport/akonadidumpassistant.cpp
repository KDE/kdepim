#include "akonadidumpassistant.h"

#include <akonadi/agentmanager.h>
#include <kdialogjobuidelegate.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kpushbutton.h>
#include <QtCore/QEvent>
#include <QtCore/QString>
#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QHeaderView>
#include <QtGui/QTableView>

#include "resourcedumpjob.h"
#include "resourcerestorejob.h"

AkonadiDumpAssistant::AkonadiDumpAssistant( AssistantMode mode, QWidget *parent ) :
    KAssistantDialog( parent ), m_mode( mode ), m_remainingJobs( 0 )
{
  m_infoPage = addPage( new ADAssInfoPage( mode, this ), "Info page" );
  connect( m_infoPage->widget(), SIGNAL( pathSelected( QString ) ), this, SLOT( pathSelected( QString ) ) );
  setValid( m_infoPage, false );
  m_selectPage = addPage( new ADAssSelectResourcesPage( mode, this ), "Select resources" );

  showButton( KDialog::Help, false );
}

void AkonadiDumpAssistant::accept()
{
  // create list of selected resource items
  QList< ResourceItem* > selectedItems;
  foreach ( ResourceItem *item, m_items ) {
    if ( item->selected() )
      selectedItems.append( item );
  }

  // finish if no items were selected
  if ( selectedItems.isEmpty() )
    KDialog::accept();

  ResourceItemModel *model = new ResourceItemModel( selectedItems, this );
  qobject_cast< ADAssSelectResourcesPage* >( m_selectPage->widget() )->enterProgressMode( model );
  setValid( m_selectPage, false );
  enableButton( KDialog::Cancel, false );
  enableButton( KDialog::User3, false );

  foreach ( ResourceItem* item, selectedItems ) {
    m_dir.mkdir( item->id() );
    connect( item->job(), SIGNAL( result( KJob* ) ), this, SLOT( jobResult( KJob* ) ) );
    item->job()->start();
  }
}

void AkonadiDumpAssistant::jobResult( KJob *job )
{
  if ( --m_remainingJobs <= 0 )
    KDialog::accept();
}

void AkonadiDumpAssistant::pathSelected( const QString &path )
{
  m_dir.setPath( path );
  if ( !m_dir.exists() ) {
    setValid( m_infoPage, false );
    return;
  }

  // prepare list of resource items
  if ( m_mode == AkonadiDump ) {
    Akonadi::AgentManager *mgr = Akonadi::AgentManager::self();
    foreach ( const Akonadi::AgentInstance &instance, mgr->instances() ) {
      // check if instance is a resource
      if ( !instance.type().capabilities().contains( "Resource" ) )
        continue;

      ResourceDumpJob *job = new ResourceDumpJob( instance, m_dir.absoluteFilePath( instance.identifier() ), this );
      ResourceItem *item = new ResourceItem( instance.identifier(), this );
      item->setJob( job );
      m_items.append( item );
    }
  }
  else {
    foreach ( const QString &resource, m_dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) ) {
      ResourceRestoreJob *job = new ResourceRestoreJob( m_dir.absoluteFilePath( resource ), this );
      ResourceItem *item = new ResourceItem( resource, this );
      item->setJob( job );
      m_items.append( item );
    }
  }

  // update model-dependent pages
  ResourceItemModel *model = new ResourceItemModel( m_items, this );
  qobject_cast< ADAssSelectResourcesPage* >( m_selectPage->widget() )->enterSelectionMode( model );

  setValid( m_infoPage, true );
}


//_________________________________________________________


ResourceItem::ResourceItem( const QString &id, QObject *parent ) :
    QObject( parent ), m_id( id ), m_selected( false ), m_job( 0 ), m_percent( 0 )
{
}

QString ResourceItem::id() const
{
  return m_id;
}

bool ResourceItem::selected() const
{
  return m_selected;
}

KJob *ResourceItem::job() const
{
  return m_job;
}

int ResourceItem::percent() const
{
  return m_percent;
}

void ResourceItem::setSelected( bool selected )
{
  m_selected = selected;
}

void ResourceItem::setJob( KJob *job )
{
  m_job = job;
  connect( m_job, SIGNAL( percent( KJob*, unsigned long ) ), this, SLOT( jobProgress( KJob*, unsigned long ) ) );
  connect( m_job, SIGNAL( result( KJob* ) ), this, SLOT( jobResult( KJob* ) ) );
}

void ResourceItem::jobProgress( KJob *job, unsigned long value )
{
  m_percent = (int) value;
  emit changed( this );
}

void ResourceItem::jobResult( KJob *job )
{
  if ( job->error() ) {
    kDebug() << "Job error: " << job->errorText();
    m_percent = -1;
  }
  else {
    m_percent = 100;
  }

  emit changed( this );
}


//_________________________________________________________


ResourceItemModel::ResourceItemModel( const QList< ResourceItem* > &items, QObject *parent ) :
    QAbstractTableModel( parent ), m_items( items )
{
  foreach ( ResourceItem *item, items )
    connect( item, SIGNAL( changed( ResourceItem* ) ), this, SLOT( itemChanged( ResourceItem* ) ) );
}

int ResourceItemModel::rowCount( const QModelIndex& ) const
{
  return m_items.size();
}

int ResourceItemModel::columnCount( const QModelIndex& ) const
{
  return 3;
}

QVariant ResourceItemModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( index.row() >= m_items.size() )
    return QVariant();

  if ( index.column() >= 3 )
    return QVariant();

  if ( role == Qt::DisplayRole && index.column() == 0 )
      return m_items[ index.row() ]->id();
  else if ( role == Qt::CheckStateRole && index.column() == 1 )
    return  m_items[ index.row() ]->selected() ? Qt::Checked : Qt::Unchecked;
  else if ( role == Qt::DisplayRole && index.column() == 2 )
    return m_items[ index.row() ]->percent();
  else
    return QVariant();
}

Qt::ItemFlags ResourceItemModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemIsEnabled;

  if ( index.column() == 1 )
    return QAbstractTableModel::flags( index ) | Qt::ItemIsUserCheckable;
  else
    return QAbstractTableModel::flags( index );
}

bool ResourceItemModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( index.isValid() && role == Qt::CheckStateRole && index.column() == 1 ) {
    m_items[ index.row() ]->setSelected( value.toBool() );
    emit dataChanged( index, index );
    return true;
  }

  return false;
}

QList< ResourceItem* > &ResourceItemModel::items()
{
  return m_items;
}

void ResourceItemModel::itemChanged( ResourceItem *item )
{
  for ( unsigned i = 0; i < m_items.size(); ++i ) {
    if ( m_items[ i ] == item ) {
      emit dataChanged( index( i, 0 ), index( i, columnCount() ) );
      break;
    }
  }
}


//_________________________________________________________


ADAssInfoPage::ADAssInfoPage( AkonadiDumpAssistant::AssistantMode mode, QWidget *parent ) :
    QWidget( parent )
{
  setupGui( mode );
}

void ADAssInfoPage::showPathDialog()
{
  KUrl url;
  if ( !m_pathEdit->text().isEmpty() )
    url.setPath( m_pathEdit->text() );
  QString caption( "Select directory..." );
  QString path = KFileDialog::getExistingDirectory( url, this, caption );
  if ( !path.isEmpty() )
    m_pathEdit->setText( path );
}

void ADAssInfoPage::setupGui( AkonadiDumpAssistant::AssistantMode mode )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->addStretch();

  QLabel *infoText = new QLabel( this );
  infoText->setText( "This is info text, behold" );
  layout->addWidget( infoText );

  QHBoxLayout *pathLayout = new QHBoxLayout();
  QLabel *label = new QLabel( this );
  label->setText( "Path:" );
  label->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );
  pathLayout->addWidget( label );
  m_pathEdit = new QLineEdit( this );
  m_pathEdit->clear();
  m_pathEdit->setReadOnly( false );
  m_pathEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
  connect( m_pathEdit, SIGNAL( textChanged( QString ) ), this, SIGNAL( pathSelected( QString ) ) );
  pathLayout->addWidget( m_pathEdit );
  QPushButton *button = new QPushButton( this );
  button->setText( "..." );
  button->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );
  connect( button, SIGNAL( clicked() ), this, SLOT( showPathDialog() ) );
  pathLayout->addWidget( button );

  layout->addLayout( pathLayout );
  layout->addStretch();
}


//_________________________________________________________


ADAssSelectResourcesPage::ADAssSelectResourcesPage( AkonadiDumpAssistant::AssistantMode mode, QWidget *parent )  :
    QWidget( parent ), m_view( 0 )
{
  setupGui( mode );
}

void ADAssSelectResourcesPage::enterSelectionMode( ResourceItemModel *model )
{
  m_view->setModel( model );
  m_view->hideColumn( 2 ); // hide KJob* column
  m_view->resizeColumnsToContents();
  m_view->resizeRowsToContents();
  m_view->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  m_view->horizontalHeader()->moveSection( 1, 0 );
  m_view->updateGeometry();
}

void ADAssSelectResourcesPage::enterProgressMode( ResourceItemModel *model )
{
  m_view->setModel( model );
  m_view->hideColumn( 1 ); // hide selection column
  m_view->setItemDelegateForColumn( 2, new ResourceItemProgressDelegate( m_view ) );
  m_view->resizeColumnsToContents();
  m_view->resizeRowsToContents();
  m_view->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  m_view->updateGeometry();
}

void ADAssSelectResourcesPage::setupGui( AkonadiDumpAssistant::AssistantMode mode )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->addStretch();

  QHBoxLayout *hLayout = new QHBoxLayout();
  m_view = new QTableView( this );
  m_view->setShowGrid( false );
  m_view->verticalHeader()->hide();
  m_view->horizontalHeader()->hide();
  m_view->setSelectionMode( QAbstractItemView::NoSelection );
  hLayout->addWidget( m_view );

  layout->addLayout( hLayout );
  layout->addStretch();
}


//_________________________________________________________


ResourceItemProgressDelegate::ResourceItemProgressDelegate( QWidget *parent ) :
    QStyledItemDelegate( parent )
{
}

void ResourceItemProgressDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  //
  // Following code is taken from Qt "Torrent" example.
  //
  if (index.column() != 2) {
      QStyledItemDelegate::paint(painter, option, index);
      return;
  }

  // Set up a QStyleOptionProgressBar to precisely mimic the
  // environment of a progress bar.
  QStyleOptionProgressBar progressBarOption;
  progressBarOption.state = QStyle::State_Enabled;
  progressBarOption.direction = QApplication::layoutDirection();
  progressBarOption.rect = option.rect;
  progressBarOption.fontMetrics = QApplication::fontMetrics();
  progressBarOption.minimum = 0;
  progressBarOption.maximum = 100;
  progressBarOption.textAlignment = Qt::AlignCenter;
  progressBarOption.textVisible = false;

  // Set the progress and text values of the style option.
  progressBarOption.progress = index.data().toInt();

  // Draw the progress bar onto the view.
  QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
}
