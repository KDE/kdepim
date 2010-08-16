#ifndef RESOURCERESTOREJOB_H
#define RESOURCERESTOREJOB_H

#include <kjob.h>
#include <akonadi/agentinstance.h>
#include <QtCore/QDir>

class ResourceRestoreJob : public KJob
{
  Q_OBJECT

public:
  ResourceRestoreJob( const QDir &path, QObject *parent = 0);

  virtual void start();

private slots:
  void createResult( KJob *job );
  void restoreResult( KJob *job );

private:
  Akonadi::AgentInstance m_instance;
  QDir m_path;
};

#endif // RESOURCERESTOREJOB_H
