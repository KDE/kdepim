#ifndef AKONADIDUMPASSISTANT_H
#define AKONADIDUMPASSISTANT_H

#include <kassistantdialog.h>

#include <akonadi/agentinstance.h>
#include <QtCore/QAbstractTableModel>
#include <QtCore/QDir>
#include <QtGui/QStyledItemDelegate>

class KJob;
class ResourceItem;
class ResourceItemModel;
class ResourceItemProgressDelegate;
class ADAssInfoPage;
class ADAssSelectResourcesPage;
class QTableView;
class QLabel;
class QLineEdit;
class QProgressBar;

class AkonadiDumpAssistant : public KAssistantDialog
{
    Q_OBJECT

public:
  enum AssistantMode { AkonadiDump, AkonadiRestore };

  AkonadiDumpAssistant( AssistantMode mode, QWidget *parent = 0);

  virtual void accept();

private slots:
  void pathSelected( const QString &path );
  void jobResult( KJob *job );

private:
  AssistantMode m_mode;
  QList< ResourceItem* > m_items;
  int m_remainingJobs;
  KPageWidgetItem *m_infoPage;
  KPageWidgetItem *m_selectPage;
  QDir m_dir;
};


//_________________________________________________________


class ResourceItem : public QObject
{
  Q_OBJECT

public:
  ResourceItem( const QString &id, QObject *parent = 0 );

  QString id() const;
  bool selected() const;
  KJob *job() const;
  int percent() const;

  void setSelected( bool selected );
  void setJob( KJob *job );

private slots:
  void jobProgress( KJob *job, unsigned long value );
  void jobResult( KJob *job );

signals:
  void changed( ResourceItem* item );

private:
  QString m_id;
  bool m_selected;
  KJob *m_job;
  int m_percent;
};


//_________________________________________________________


class ResourceItemModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  ResourceItemModel( const QList< ResourceItem* > &items, QObject *parent = 0 );

  int rowCount( const QModelIndex &parent = QModelIndex() ) const;
  int columnCount( const QModelIndex &parent = QModelIndex() ) const;
  QVariant data( const QModelIndex &index, int role ) const;
  Qt::ItemFlags flags( const QModelIndex &index ) const;
  bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

  QList< ResourceItem* > &items();

private slots:
  void itemChanged( ResourceItem *item );

private:
  QList< ResourceItem* > m_items;
};


//_________________________________________________________


class ADAssInfoPage : public QWidget
{
  Q_OBJECT

public:
  ADAssInfoPage( AkonadiDumpAssistant::AssistantMode mode, QWidget *parent = 0 );

private slots:
  void showPathDialog();

signals:
  void pathSelected( QString path );

private:
  void setupGui( AkonadiDumpAssistant::AssistantMode mode );

  QLineEdit *m_pathEdit;
};


//_________________________________________________________


class ADAssSelectResourcesPage : public QWidget
{
  Q_OBJECT

public:
  ADAssSelectResourcesPage( AkonadiDumpAssistant::AssistantMode mode, QWidget *parent = 0 );

public slots:
  void enterSelectionMode(  ResourceItemModel *model );
  void enterProgressMode( ResourceItemModel *model );

private:
  void setupGui( AkonadiDumpAssistant::AssistantMode mode );

  QTableView *m_view;
};


//_________________________________________________________


class ResourceItemProgressDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit ResourceItemProgressDelegate( QWidget *parent = 0 );
  void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

#endif // AKONADIDUMPASSISTANT_H
