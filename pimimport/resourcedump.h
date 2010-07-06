#ifndef RESOURCEDUMP_H
#define RESOURCEDUMP_H

#include "abstractdump.h"

#include <QtCore/QHash>
#include <QtCore/QSharedPointer>
#include <akonadi/agentinstance.h>
#include <akonadi/collection.h>
#include <ksharedconfig.h>

class KJob;

class ResourceDump : public AbstractDump
{
  Q_OBJECT

public:
  ResourceDump( const QDir &path, QObject *parent = 0 );
  ResourceDump( const QDir &path, const Akonadi::AgentInstance &instance, QObject *parent = 0 );
  ~ResourceDump();

  Akonadi::AgentInstance instance() const;

public slots:
  void dump();
  void restore();

private slots:
  void resourceCreated( KJob *job );
  void dumpSubCollectionsResult( KJob *job );

private:
  void restoreResource();
  void dumpSubCollections( const Akonadi::Collection &base );

  QString m_name;
  KSharedConfigPtr m_config;
  Akonadi::AgentInstance m_instance;
  unsigned m_remainingJobs;

  // used while dumping
  QHash< Akonadi::Entity::Id, QSharedPointer< KConfigGroup > > m_configGroups;
};

#endif // RESOURCEDUMP_H
