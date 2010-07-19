#ifndef COLLECTIONRESTOREJOB_H
#define COLLECTIONRESTOREJOB_H

#include <kjob.h>
#include <akonadi/collection.h>
#include <QtCore/QDir>

class CollectionRestoreJob : public KJob
{
    Q_OBJECT

public:
  CollectionRestoreJob(const Akonadi::Collection& parentCollection, const QDir &path, QObject *parent = 0);

  virtual void start();

private slots:
  void createResult( KJob *job );
  void restoreResult( KJob *job );

private:
  Akonadi::Collection m_parent;
  QDir m_path;
  unsigned m_subcollectionsNo;
};

#endif // COLLECTIONRESTOREJOB_H
