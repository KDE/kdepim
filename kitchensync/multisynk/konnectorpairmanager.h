#ifndef KONNECTORPAIRMANAGER_H
#define KONNECTORPAIRMANAGER_H

#include <qobject.h>

#include "konnectorpair.h"

class KonnectorPairManager : public QObject
{
  Q_OBJECT

  public:
    KonnectorPairManager( QObject *parent );
    ~KonnectorPairManager();

    void load();
    void save();

    void add( KonnectorPair *pair );
    void change( KonnectorPair *pair );
    void remove( const QString &uid );

    KonnectorPair* pair( const QString &uid ) const;
    KonnectorPair::List pairs() const;

  signals:
    void changed();

  private:
    QString configFile() const;

    KonnectorPair::Map mPairs;
};

#endif
