#ifndef ABSTRACTDUMP_H
#define ABSTRACTDUMP_H

#include <QtCore/QObject>
#include <QtCore/QString>

/**
 Abstract base class for all dumps.
 */
class AbstractDump : public QObject
{
  Q_OBJECT

  public:
    AbstractDump( QString path, QObject *parent = 0 );
    virtual ~AbstractDump();

    /**
      Path to the dump.
      */
    QString path() const;

    /** Enum specifying whether we are dumping or restoring. */
    enum Action {
      action_dump,
      action_restore
    };
    Q_ENUMS ( Action )

  public slots:
    /** Performes dump to disk. */
    virtual void dump() = 0;

    /** Restores a dump from disk. */
    virtual void restore() = 0;

  signals:
    void finished();

  private:
    QString m_path; // path to the dump
};

#endif // ABSTRACTDUMP_H
