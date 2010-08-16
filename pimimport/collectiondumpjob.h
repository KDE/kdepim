#ifndef COLLECTIONDUMPJOB_H
#define COLLECTIONDUMPJOB_H

#include <kjob.h>
#include <akonadi/collection.h>
#include <ksharedconfig.h>
#include <QtCore/QDir>

class KConfigGroup;

class CollectionDumpJob : public KJob
{
  Q_OBJECT

public:
  CollectionDumpJob(const Akonadi::Collection &collection, QDir dumpPath, QObject *parent = 0);

  virtual void start();

private slots:
  void fetchResult( KJob* job );
  void dumpResult( KJob* job );
  void itemFetchResult( KJob *job );

private:
  void dumpItems();

  Akonadi::Collection m_collection;
  QDir m_path;
  unsigned m_subjobs;
  KSharedConfigPtr m_config;
};

#endif // COLLECTIONDUMPJOB_H
