#ifndef ABSTRACTDUMP_H
#define ABSTRACTDUMP_H

#include <QtCore/QObject>
#include <QtCore/QDir>

/**
 Abstract base class for all dumps.
 */
class AbstractDump : public QObject
{
  Q_OBJECT

  public:
    AbstractDump( const QDir &path, QObject *parent = 0 );
    virtual ~AbstractDump();

    /**
      Path to the dump.
      */
    QDir path() const;

  public slots:
    /** Performes dump to disk. */
    virtual void dump() = 0;

    /** Restores a dump from disk. */
    virtual void restore() = 0;

  signals:
    void finished();

  private:
    QDir m_path; // path to the dump
};

#endif // ABSTRACTDUMP_H
