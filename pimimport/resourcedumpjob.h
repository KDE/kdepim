#ifndef RESOURCEDUMPJOB_H
#define RESOURCEDUMPJOB_H

#include <kjob.h>
#include <akonadi/agentinstance.h>
#include <QtCore/QDir>

class ResourceDumpJob : public KJob
{
  Q_OBJECT

public:
  ResourceDumpJob(const Akonadi::AgentInstance &instance, const QDir &dumpPath, QObject *parent = 0 );

  virtual void start();

private slots:
  void fetchResult( KJob *job );
  void dumpResult( KJob *job );

private:
  Akonadi::AgentInstance m_instance;
  QDir m_path;
  unsigned m_collectionsNo;
};

#endif // RESOURCEDUMPJOB_H
