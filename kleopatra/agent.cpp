#include "agent.h"

Agent::Agent( const QString& agentName, Agent* provider, QObject* parent, const char* name ) 
  :QObject( parent, name ), _provider( provider), _agentName( agentName )
{
}

/**
   Return a QString with a representation of the CA, and all its parent CA's
*/
QString Agent::tree() const
{
  int dummy;
  return tree( &dummy );
}

QString Agent::tree( int* level ) const 
{
  QString res;
  if ( _provider ) {
    res += _provider->tree(level);
    (*level)++;
  }
  else
    *level = 0;

  for ( int i = 0; i < *level; ++i )
    res += "\t";
  
  res += _agentName + "\n"; // KHZ: Add more attributes for agents here.
  
  return res;
}

/**
   This method return the string, which is shown in the listview.
*/
QString Agent::shortName() const
{
  return _agentName; // KHZ: Here you can change the encoding for the ListView.
}
