#ifndef AKONADIDUMPER_H
#define AKONADIDUMPER_H

#include <QtCore/QDir>
#include <QtCore/QList>
#include <QtCore/QObject>

class DumpRunner : public QObject
{
  Q_OBJECT

public:
  DumpRunner( const QDir &path, QObject *parent = 0 );

public slots:
  void dump();
  void restore();

private slots:
  void resourceFinished();

signals:
  void finished();

private:
  int m_remainingResources;
  QDir m_path;
};

#endif // AKONADIDUMPER_H
