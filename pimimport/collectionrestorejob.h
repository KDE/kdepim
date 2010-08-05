#ifndef COLLECTIONRESTOREJOB_H
#define COLLECTIONRESTOREJOB_H

#include <kjob.h>
#include <akonadi/collection.h>
#include <akonadi/session.h>
#include <QtCore/QDir>

class CollectionRestoreJob : public KJob
{
    Q_OBJECT

public:
  CollectionRestoreJob(const Akonadi::Collection& parentCollection, const QDir &path, QObject *parent = 0);
  CollectionRestoreJob(const Akonadi::Collection& parentCollection, const QDir &path, Akonadi::Session *session);

  virtual void start();

private slots:
  void createResult( KJob *job );
  void checkJobResult( KJob *job );

private:
  void restoreItems();
  void decreaseSubjobs();

  Akonadi::Collection m_parent;
  Akonadi::Collection m_collection;
  QDir m_path;
  Akonadi::Session *m_session;
  unsigned m_subjobsNo;
};

#endif // COLLECTIONRESTOREJOB_H
