#ifndef __AGENT_H
#define __AGENT_H
#include <qstring.h>
#include <qobject.h>

/**
   Class representing a Certification Agent (CA)
*/
class Agent :public QObject
{
public:
  Agent( const QString& agentName, Agent* provider, QObject* parent, const char* name = 0 );
  QString tree() const;
  QString shortName() const;
private:
  QString tree( int* level ) const;

  Agent* _provider;
  QString _agentName;
};

#endif //__AGENT_H
