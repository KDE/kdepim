#ifndef KONNECTORPAIR_H
#define KONNECTORPAIR_H

#include <qmap.h>
#include <qvaluelist.h>

#include "konnectormanager.h"

class KonnectorPair
{
  public:
    typedef QValueList<KonnectorPair*> List;
    typedef QMap<QString, KonnectorPair*> Map;

    enum ResolveStrategy
    {
      ResolveManually,
      ResolveFirst,
      ResolveSecond,
      ResolveBoth
    };

    KonnectorPair();
    ~KonnectorPair();

    QString uid() const;
    void setUid( const QString &uid );

    QString name() const;
    void setName( const QString &name );

    int resolveStrategy() const;
    void setResolveStrategy( int strategy );

    void load();
    void save();

    KonnectorManager* manager();

  private:
    QString configFile() const;

    QString mUid;
    QString mName;
    int mStrategy;

    KonnectorManager *mManager;
    KConfig *mConfig;
};

#endif
