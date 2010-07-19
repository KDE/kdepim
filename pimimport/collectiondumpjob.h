#ifndef COLLECTIONDUMPJOB_H
#define COLLECTIONDUMPJOB_H

#include <kjob.h>
#include <akonadi/collection.h>
#include <QtCore/QDir>

class CollectionDumpJob : public KJob
{
  Q_OBJECT

public:
  CollectionDumpJob(const Akonadi::Collection &collection, QDir dumpPath, QObject *parent = 0);

  virtual void start();

private slots:
  void fetchResult( KJob* job );
  void dumpResult( KJob* job );

private:
  void dumpSubCollections();

  Akonadi::Collection m_collection;
  QDir m_path;
  unsigned m_subjobs;
};

#endif // COLLECTIONDUMPJOB_H
