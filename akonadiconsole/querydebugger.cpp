/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 * Copyright (C) 2017  Daniel Vrátil <dvratil@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "querydebugger.h"
#include "ui_querydebugger.h"
#include "storagedebuggerinterface.h"

#include <QtGui/QMenu>
#include <QtGui/QApplication>

#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QDateTime>
#include <QIcon>
#include <QAbstractItemModel>
#include <QFileDialog>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusMetaType>
#ifndef Q_MOC_RUN
#include <boost/concept_check.hpp>
#endif
#include <QtCore/QDateTime>

#include <akonadi/servermanager.h>
#include <akonadi/control.h>

#include <KLocalizedString>
#include <KColorScheme>

#include <algorithm>

Q_DECLARE_METATYPE(QList< QList<QVariant> >)

QDBusArgument &operator<<(QDBusArgument &arg, const DbConnection &con)
{
    arg.beginStructure();
    arg << con.id
        << con.name
        << con.start
        << con.transactionStart;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, DbConnection &con)
{
    arg.beginStructure();
    arg >> con.id
        >> con.name
        >> con.start
        >> con.transactionStart;
    arg.endStructure();
    return arg;
}

struct QueryInfo {
    QString query;
    quint64 duration;
    quint64 calls;

    bool operator<(const QString &other) const
    {
        return query < other;
    }
};

Q_DECLARE_TYPEINFO(QueryInfo, Q_MOVABLE_TYPE);

class QueryTreeModel : public QAbstractItemModel
{
    Q_OBJECT

    class Node
    {
    public:
        virtual ~Node() {}

        Node *parent;
        enum Type {
            Connection, Transaction, Query
        };
        Type type;
        qint64 start;
        uint duration;
    };

    class Query : public Node
    {
    public:
        QString query;
        QString error;
    };

    class Transaction : public Query
    {
    public:
        ~Transaction()
        {
            qDeleteAll(queries);
        }

        enum TransactionType {
            Begin, Commit, Rollback
        };
        TransactionType transactionType;
        QVector<Query*> queries;
    };

    class Connection : public Node
    {
    public:
        ~Connection()
        {
            qDeleteAll(queries);
        }

        QString name;
        QVector<Node*> queries; // FIXME: Why can' I use QVector<Query*> here??
    };

public:
    QueryTreeModel(QObject *parent)
        : QAbstractItemModel(parent)
    {}

    ~QueryTreeModel()
    {
        qDeleteAll(mConnections);
    }

public Q_SLOTS:
    void clear()
    {
        beginResetModel();
        qDeleteAll(mConnections);
        mConnections.clear();
        endResetModel();
    }

    void addConnection(qlonglong id, const QString &name, qlonglong timestamp)
    {
        auto con = new Connection;
        con->parent = nullptr;
        con->type = Node::Connection;
        con->name = name.isEmpty() ? QLatin1String("<unnamed connection>") : name;
        con->start = timestamp;
        beginInsertRows(QModelIndex(), mConnections.count(), mConnections.count());
        mConnections << con;
        mConnectionById.insert(id, con);
        endInsertRows();
    }

    void updateConnection(qlonglong id, const QString &name)
    {
        auto con = mConnectionById.value(id);
        if (!con) {
            return;
        }

        con->name = name;
        const QModelIndex index = createIndex(mConnections.indexOf(con), columnCount() - 1, con);
        Q_EMIT dataChanged(index, index.sibling(index.row(), 5));
    }

    void addTransaction(qlonglong connectionId, qlonglong timestamp, uint duration, const QString &error)
    {
        auto con = mConnectionById.value(connectionId);
        if (!con) {
            return;
        }

        auto trx = new Transaction;
        trx->parent = con;
        trx->type = Node::Transaction;
        trx->start = timestamp;
        trx->duration = duration;
        trx->transactionType = Transaction::Begin;
        trx->error = error.trimmed();
        const QModelIndex conIdx = createIndex(mConnections.indexOf(con), 0, con);
        beginInsertRows(conIdx, con->queries.count(), con->queries.count());
        con->queries << trx;
        endInsertRows();
    }

    void closeTransaction(qlonglong connectionId, bool commit, qlonglong timestamp, uint,
                          const QString &error)
    {
        auto con = mConnectionById.value(connectionId);
        if (!con) {
            return;
        }

        // Find the last open transaction and change it to closed
        for (int i = con->queries.count() - 1; i >= 0; i--) {
            Node *node = con->queries[i];
            if (node->type == Node::Transaction) {
                Transaction *trx = static_cast<Transaction*>(node);
                if (trx->transactionType != Transaction::Begin) {
                    continue;
                }

                trx->transactionType = commit ? Transaction::Commit : Transaction::Rollback;
                trx->duration = timestamp - trx->start;
                trx->error = error.trimmed();

                const QModelIndex trxIdx = createIndex(i, 0, trx);
                Q_EMIT dataChanged(trxIdx, trxIdx.sibling(trxIdx.row(), columnCount() - 1));
                return;
            }
        }
    }

    void addQuery(qlonglong connectionId, const QString &queryStr, qlonglong timestamp,
                  uint duration, const QString &error)
    {
        auto con = mConnectionById.value(connectionId);
        if (!con) {
            return;
        }

        auto query = new Query;
        query->type = Node::Query;
        query->query = queryStr;
        query->start = timestamp;
        query->duration = duration;
        query->error = error.trimmed();

        if (con->queries.isEmpty() || con->queries.last()->type == Node::Query) {
            query->parent = con;
            beginInsertRows(createIndex(mConnections.indexOf(con), 0, con),
                            con->queries.count(), con->queries.count());
            con->queries << query;
            endInsertRows();
        } else {
            auto trx = static_cast<Transaction*>(con->queries.last());
            query->parent = trx;
            beginInsertRows(createIndex(con->queries.indexOf(trx), 0, trx),
                            trx->queries.count(), trx->queries.count());
            trx->queries << query;
            endInsertRows();
        }
    }

    void dumpRow(QFile &file, const QModelIndex &idx, int depth)
    {
        if (idx.isValid()) {
            QTextStream stream(&file);
            for (int i = 0; i < depth; ++i) {
                stream << QLatin1String("  |");
            }
            stream << QLatin1String("- ");

            Node *node = reinterpret_cast<Node*>(idx.internalPointer());
            switch (node->type) {
            case Node::Connection: {
                Connection *con = static_cast<Connection*>(node);
                stream << con->name << "    " << fromMSecsSinceEpoch(con->start);
                break;
            }
            case Node::Transaction: {
                Transaction *trx = static_cast<Transaction*>(node);
                switch (trx->transactionType) {
                case Transaction::Begin: stream << QLatin1String("BEGIN"); break;
                case Transaction::Commit: stream << QLatin1String("COMMIT"); break;
                case Transaction::Rollback: stream << QLatin1String("ROLLBACK"); break;
                }
                stream << "    " << fromMSecsSinceEpoch(trx->start);
                if (trx->transactionType > Transaction::Begin) {
                    stream << " - " << fromMSecsSinceEpoch(trx->start + trx->duration);
                }
                break;
            }
            case Node::Query: {
                Query *query = static_cast<Query*>(node);
                stream << query->query << "    " << fromMSecsSinceEpoch(query->start) << ", took " << query->duration << " ms";
                break;
            }
            }

            if (node->type >= Node::Transaction) {
                Query *query = static_cast<Query*>(node);
                if (!query->error.isEmpty()) {
                    stream << '\n';
                    for (int i = 0; i < depth; ++i) {
                        stream << QLatin1String("  |");
                    }
                    stream << QLatin1String("  Error: ") << query->error;
                }
            }

            stream << '\n';
        }

        for (int i = 0, c = rowCount(idx); i < c; ++i) {
            dumpRow(file, index(i, 0, idx), depth + 1);
        }
    }

public:
    int rowCount(const QModelIndex &parent) const
    {
        if (!parent.isValid()) {
            return mConnections.count();
        }

        Node *node = reinterpret_cast<Node*>(parent.internalPointer());
        switch (node->type) {
        case Node::Connection:
            return static_cast<Connection*>(node)->queries.count();
        case Node::Transaction:
            return static_cast<Transaction*>(node)->queries.count();
        case Node::Query:
            return 0;
        }

        Q_ASSERT(false);
        return 0;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        return 5;
    }

    QModelIndex parent(const QModelIndex &child) const
    {
        if (!child.isValid() || !child.internalPointer()) {
            return QModelIndex();
        }

        Node *childNode = reinterpret_cast<Node*>(child.internalPointer());
        // childNode is a Connection
        if (!childNode->parent) {
            return QModelIndex();
        }

        // childNode is a query in transaction
        if (childNode->parent->parent) {
            Connection *connection = static_cast<Connection*>(childNode->parent->parent);
            const int trxIdx = connection->queries.indexOf(childNode->parent);
            return createIndex(trxIdx, 0, childNode->parent);
        } else {
            // childNode is a query without transaction or a transaction
            return createIndex(mConnections.indexOf(static_cast<Connection*>(childNode->parent)),
                               0, childNode->parent);
        }
    }

    QModelIndex index(int row, int column, const QModelIndex &parent) const
    {
        if (!parent.isValid()) {
            if (row < mConnections.count()) {
                return createIndex(row, column, mConnections.at(row));
            } else {
                return QModelIndex();
            }
        }

        Node *parentNode = reinterpret_cast<Node*>(parent.internalPointer());
        switch (parentNode->type) {
        case Node::Connection:
            if (row < static_cast<Connection*>(parentNode)->queries.count()) {
                return createIndex(row, column, static_cast<Connection*>(parentNode)->queries.at(row));
            } else {
                return QModelIndex();
            }
        case Node::Transaction:
            if (row < static_cast<Transaction*>(parentNode)->queries.count()) {
                return createIndex(row, column, static_cast<Transaction*>(parentNode)->queries.at(row));
            } else {
                return QModelIndex();
            }
        case Node::Query:
            // Query can never have children
            return QModelIndex();
        }

        Q_ASSERT(false);
        return QModelIndex();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
            return QVariant();
        }

        switch (section) {
        case 0: return QLatin1String("Name / Query");
        case 1: return QLatin1String("Started");
        case 2: return QLatin1String("Ended");
        case 3: return QLatin1String("Duration");
        case 4: return QLatin1String("Error");
        }

        return QVariant();
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid()) {
            return QVariant();
        }

        Node *node = reinterpret_cast<Node*>(index.internalPointer());
        switch (node->type) {
        case Node::Connection:
            return connectionData(static_cast<Connection*>(node), index.column(), role);
        case Node::Transaction:
            return transactionData(static_cast<Transaction*>(node), index.column(), role);
        case Node::Query:
            return queryData(static_cast<Query*>(node), index.column(), role);
        }

        Q_ASSERT(false);
        return QVariant();
    }

private:
    QString fromMSecsSinceEpoch(qint64 msecs) const
    {
        return QDateTime::fromMSecsSinceEpoch(msecs).toString(QLatin1String("dd-MM-yyyy HH:mm:ss.zzz"));
    }


    QVariant connectionData(Connection *connection, int column, int role) const
    {
        if (role != Qt::DisplayRole) {
            return QVariant();
        }

        switch (column) {
        case 0: return connection->name;
        case 1: return fromMSecsSinceEpoch(connection->start);
        }

        return QVariant();
    }

    QVariant transactionData(Transaction *transaction, int column, int role) const
    {
        if (role == Qt::DisplayRole && column == 0) {
            switch (transaction->transactionType) {
            case Transaction::Begin: return QLatin1String("BEGIN");
            case Transaction::Commit: return QLatin1String("COMMIT");
            case Transaction::Rollback: return QLatin1String("ROLLBACK");
            }
            Q_ASSERT(false);
        } else {
            return queryData(transaction, column, role);
        }
        return QVariant();
    }

    QVariant queryData(Query *query, int column, int role) const
    {
        if (role == Qt::BackgroundRole) {
            if (!query->error.isEmpty()) {
                return KColorScheme(QPalette::Normal).background(KColorScheme::NegativeBackground).color();
            }
        } else if (role == Qt::DisplayRole) {
            switch (column) {
            case 0: return query->query;
            case 1: return fromMSecsSinceEpoch(query->start);
            case 2: return fromMSecsSinceEpoch(query->start + query->duration);
            case 3: return QTime(0, 0, 0).addMSecs(query->duration).toString(QLatin1String("HH:mm:ss.zzz"));
            case 4: return query->error;
            }
            Q_ASSERT(false);
        }

        return QVariant();
    }

    QVector<Connection *> mConnections;
    QHash<qint64, Connection *> mConnectionById;
};


class QueryDebuggerModel : public QAbstractListModel
{
  Q_OBJECT
public:
  QueryDebuggerModel(QObject* parent)
    : QAbstractListModel(parent)
  {
    mSpecialRows[TOTAL].query = "TOTAL";
    mSpecialRows[TOTAL].duration = 0;
    mSpecialRows[TOTAL].calls = 0;
  }

  enum SPECIAL_ROWS {
    TOTAL,
    NUM_SPECIAL_ROWS
  };
  enum Colums {
    DurationColumn,
    CallsColumn,
    AvgDurationColumn,
    QueryColumn,
    NUM_COLUMNS
  };

  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
  {
    if (orientation == Qt::Vertical || section < 0 || section >= NUM_COLUMNS || role != Qt::DisplayRole) {
      return QVariant();
    }

    if (section == QueryColumn) {
      return "Query";
    } else if (section == DurationColumn) {
      return "Duration [ms]";
    } else if (section == CallsColumn) {
      return "Calls";
    } else if (section == AvgDurationColumn) {
      return "Avg. Duration [ms]";
    }

    return QVariant();
  }

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
  {
    if (role != Qt::DisplayRole && role != Qt::ToolTipRole) {
      return QVariant();
    }

    const int row = index.row();
    if (row < 0 || row >= rowCount(index.parent())) {
      return QVariant();
    }
    const int column = index.column();
    if (column < 0 || column >= NUM_COLUMNS) {
      return QVariant();
    }

    const QueryInfo& info = (row < NUM_SPECIAL_ROWS)
      ? mSpecialRows[row]
      : mQueries.at(row - NUM_SPECIAL_ROWS);

    if (role == Qt::ToolTipRole) {
      return QString(QLatin1String("<qt>") + info.query + QLatin1String("</qt>"));
    }

    if (column == QueryColumn) {
      return info.query;
    } else if (column == DurationColumn) {
      return info.duration;
    } else if (column == CallsColumn) {
      return info.calls;
    } else if (column == AvgDurationColumn) {
      return float(info.duration) / info.calls;
    }

    return QVariant();
  }

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const
  {
    if (!parent.isValid()) {
      return mQueries.size() + NUM_SPECIAL_ROWS;
    } else {
      return 0;
    }
  }

  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const
  {
    if (!parent.isValid()) {
      return NUM_COLUMNS;
    } else {
      return 0;
    }
  }

  void addQuery(const QString& query, uint duration)
  {
    QVector<QueryInfo>::iterator it = std::lower_bound(mQueries.begin(), mQueries.end(), query);

    const int row = std::distance(mQueries.begin(), it) + NUM_SPECIAL_ROWS;

    if (it != mQueries.end() && it->query == query) {
      ++(it->calls);
      it->duration += duration;

      emit dataChanged(index(row, DurationColumn), index(row, AvgDurationColumn));
    } else {
      beginInsertRows(QModelIndex(), row, row);
      QueryInfo info;
      info.query = query;
      info.duration = duration;
      info.calls = 1;
      mQueries.insert(it, info);
      endInsertRows();
    }

    mSpecialRows[TOTAL].duration += duration;
    ++mSpecialRows[TOTAL].calls;
    emit dataChanged(index(TOTAL, DurationColumn), index(TOTAL, AvgDurationColumn));
  }

  void clear()
  {
    beginResetModel();
    mQueries.clear();
    mSpecialRows[TOTAL].duration = 0;
    mSpecialRows[TOTAL].calls = 0;
    endResetModel();
  }

private:
  QVector<QueryInfo> mQueries;
  QueryInfo mSpecialRows[NUM_SPECIAL_ROWS];
};

QueryDebugger::QueryDebugger( QWidget *parent ):
  QWidget( parent ),
  mUi( new Ui::QueryDebugger )
{
  qDBusRegisterMetaType< QList< QList<QVariant> > >();
  qDBusRegisterMetaType<DbConnection>();
  qDBusRegisterMetaType<QVector<DbConnection>>();

  QString service = QLatin1String("org.freedesktop.Akonadi");
  if ( Akonadi::ServerManager::hasInstanceIdentifier() ) {
    service += QLatin1String(".") + Akonadi::ServerManager::instanceIdentifier();
  }
  mDebugger = new org::freedesktop::Akonadi::StorageDebugger( service,
            QLatin1String("/storageDebug"), QDBusConnection::sessionBus(), this );

  connect( mDebugger, SIGNAL( queryExecuted(double,qlonglong,qlonglong,uint,QString,QMap<QString,QVariant>,int,QList<QList<QVariant> >,QString) ),
           this, SLOT( addQuery(double,qlonglong,qlonglong,uint,QString,QMap<QString,QVariant>,int,QList<QList<QVariant> >,QString) ) );

  mUi->setupUi( this );
  connect( mUi->enableDebuggingChkBox, SIGNAL( toggled(bool) ),
           this, SLOT( debuggerToggled(bool) ) );

  mQueryList = new QueryDebuggerModel(this);
  QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
  proxy->setSourceModel(mQueryList);
  proxy->setDynamicSortFilter(true);
  mUi->queryListView->setModel(proxy);
  mUi->queryListView->header()->setResizeMode(QueryDebuggerModel::CallsColumn, QHeaderView::Fixed);
  mUi->queryListView->header()->setResizeMode(QueryDebuggerModel::DurationColumn, QHeaderView::Fixed);
  mUi->queryListView->header()->setResizeMode(QueryDebuggerModel::AvgDurationColumn, QHeaderView::Fixed);
  mUi->queryListView->header()->setResizeMode(QueryDebuggerModel::QueryColumn, QHeaderView::ResizeToContents);

  mQueryTree = new QueryTreeModel(this);
  mUi->queryTreeView->setModel(mQueryTree);
  connect( mDebugger, SIGNAL( connectionOpened(qlonglong,QString,qlonglong) ),
           mQueryTree, SLOT( addConnection(qlonglong,QString,qlonglong) ) );
  connect( mDebugger, SIGNAL( connectionChanged(qlonglong,QString) ),
           mQueryTree, SLOT( updateConnection(qlonglong,QString) ) );
  connect( mDebugger, SIGNAL( transactionStarted(qlonglong,qlonglong,uint,QString) ),
           mQueryTree, SLOT( addTransaction(qlonglong,qlonglong,uint,QString) ) );
  connect( mDebugger, SIGNAL( transactionFinished(qlonglong,bool,qlonglong,uint,QString) ),
           mQueryTree, SLOT( closeTransaction(qlonglong,bool,qlonglong,uint,QString) ) );

  connect( mUi->saveTreeBtn, SIGNAL( clicked(bool) ),
           this, SLOT( saveTreeToFile() ) );

  Akonadi::Control::widgetNeedsAkonadi( this );
}

QueryDebugger::~QueryDebugger()
{
  // Disable debugging when turning off Akonadi Console so that we don't waste
  // resources on server
  mDebugger->enableSQLDebugging( false );
}

void QueryDebugger::clear()
{
  mQueryList->clear();
}

void QueryDebugger::addQuery( double sequence, qlonglong connectionId, qlonglong timestamp,
                              uint duration, const QString &query, const QMap<QString, QVariant> &values,
                              int resultsCount, const QList<QList<QVariant> > &result,
                              const QString &error )
{
  mQueryList->addQuery( query, duration );
  mQueryTree->addQuery( connectionId, query, timestamp, duration, error );
}

void QueryDebugger::debuggerToggled( bool on )
{
  mDebugger->enableSQLDebugging(on);
  if ( on ) {
    mQueryTree->clear();

    const QVector<DbConnection> conns = mDebugger->connections();
    Q_FOREACH ( const DbConnection &con, conns ) {
      mQueryTree->addConnection( con.id, con.name, con.start );
      if ( con.transactionStart > 0 ) {
        mQueryTree->addTransaction( con.id, con.transactionStart,
                                    QDateTime::currentMSecsSinceEpoch() - con.transactionStart,
                                    QString() );
      }
    }
  }
}

void QueryDebugger::saveTreeToFile()
{
  const QString fileName = QFileDialog::getSaveFileName(this);
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    // show error
    return;
  }

  mQueryTree->dumpRow(file, QModelIndex(), 0);

  file.close();
}


#include "querydebugger.moc"
