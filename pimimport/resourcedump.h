#ifndef RESOURCEDUMP_H
#define RESOURCEDUMP_H

#include "abstractdump.h"

#include <akonadi/agentinstance.h>

class ResourceDump : public AbstractDump
{
  Q_OBJECT

public:
  ResourceDump( QString path, QObject *parent = 0 );
  ResourceDump( QString path, Akonadi::AgentInstance instance, QObject *parent = 0 );

  Akonadi::AgentInstance instance() const;

public slots:
  void dump();
  void restore();

private:
  Akonadi::AgentInstance m_instance;
};

#endif // RESOURCEDUMP_H
