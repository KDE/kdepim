/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
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

#include <QVBoxLayout>
#include <QCheckBox>
#include <QMenu>

#include <QToolButton>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QHeaderView>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusMetaType>
#include <boost/concept_check.hpp>
#include <QtCore/QDateTime>

#include <AkonadiCore/servermanager.h>
#include <AkonadiCore/control.h>

#include <KTextEdit>
#include <KLocalizedString>
#include <QFontDatabase>

#include <QIcon>

#include <algorithm>

Q_DECLARE_METATYPE(QList< QList<QVariant> >)

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

class QueryDebuggerModel : public QAbstractListModel
{
    Q_OBJECT
public:
    QueryDebuggerModel(QObject *parent)
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

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE
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

        const QueryInfo &info = (row < NUM_SPECIAL_ROWS)
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

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        if (!parent.isValid()) {
            return mQueries.size() + NUM_SPECIAL_ROWS;
        } else {
            return 0;
        }
    }

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const
    {
        if (!parent.isValid()) {
            return NUM_COLUMNS;
        } else {
            return 0;
        }
    }

    void addQuery(const QString &query, uint duration)
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

QueryDebugger::QueryDebugger(QWidget *parent):
    QWidget(parent)
{
    qDBusRegisterMetaType< QList< QList<QVariant> > >();

    QString service = QLatin1String("org.freedesktop.Akonadi");
    if (Akonadi::ServerManager::hasInstanceIdentifier()) {
        service += "." + Akonadi::ServerManager::instanceIdentifier();
    }
    mDebugger = new org::freedesktop::Akonadi::StorageDebugger(service,
            QLatin1String("/storageDebug"), QDBusConnection::sessionBus(), this);

    connect(mDebugger, SIGNAL(queryExecuted(double,uint,QString,QMap<QString,QVariant>,int,QList<QList<QVariant> >,QString)),
            this, SLOT(addQuery(double,uint,QString,QMap<QString,QVariant>,int,QList<QList<QVariant> >,QString)));

    QVBoxLayout *layout = new QVBoxLayout(this);

    QHBoxLayout *checkBoxLayout = new QHBoxLayout(this);

    QCheckBox *enableCB = new QCheckBox(this);
    enableCB->setText("Enable query debugger (slows down server!)");
    enableCB->setChecked(mDebugger->isSQLDebuggingEnabled());
    connect(enableCB, SIGNAL(toggled(bool)), mDebugger, SLOT(enableSQLDebugging(bool)));
    checkBoxLayout->addWidget(enableCB);

    mOnlyAggregate = new QCheckBox(this);
    mOnlyAggregate->setText("Only Aggregate data");
    mOnlyAggregate->setChecked(true);
    checkBoxLayout->addWidget(mOnlyAggregate);

    QToolButton *clearButton = new QToolButton;
    clearButton->setText("clear");
    clearButton->setIcon(QIcon::fromTheme("edit-clear-list"));
    clearButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    connect(clearButton, SIGNAL(clicked()), SLOT(clear()));
    checkBoxLayout->addWidget(clearButton);

    layout->addLayout(checkBoxLayout);

    QTreeView *queryList = new QTreeView(this);
    mModel = new QueryDebuggerModel(this);
    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(mModel);
    proxy->setDynamicSortFilter(true);
    queryList->setModel(proxy);
    queryList->setRootIsDecorated(false);
    queryList->setSortingEnabled(true);
    queryList->setUniformRowHeights(true);
    queryList->header()->setResizeMode(QueryDebuggerModel::CallsColumn, QHeaderView::Fixed);
    queryList->header()->setResizeMode(QueryDebuggerModel::DurationColumn, QHeaderView::Fixed);
    queryList->header()->setResizeMode(QueryDebuggerModel::AvgDurationColumn, QHeaderView::Fixed);
    queryList->header()->setResizeMode(QueryDebuggerModel::QueryColumn, QHeaderView::ResizeToContents);

    layout->addWidget(queryList);

    mView = new KTextEdit(this);
    mView->setReadOnly(true);
    mView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    layout->addWidget(mView);

    Akonadi::Control::widgetNeedsAkonadi(this);
}

QueryDebugger::~QueryDebugger()
{
    // Disable debugging when turning off Akonadi Console so that we don't waste
    // resources on server
    mDebugger->enableSQLDebugging(false);
}

void QueryDebugger::clear()
{
    mView->clear();
    mModel->clear();
}

void QueryDebugger::addQuery(double sequence, uint duration, const QString &query,
                             const QMap<QString, QVariant> &values,
                             int resultsCount, const QList<QList<QVariant> > &result,
                             const QString &error)
{
    mModel->addQuery(query, duration);

    if (mOnlyAggregate->isChecked()) {
        return;
    }

    QString q = query;
    const QStringList keys = values.uniqueKeys();
    Q_FOREACH (const QString &key, keys) {
        int pos = q.indexOf(QLatin1String("?"));
        const QVariant val = values.value(key);
        q.replace(pos, 1, variantToString(val));
    }

    mView->append(QString::fromLatin1("%1: <font color=\"blue\">%2</font>") .arg(sequence).arg(q));

    if (!error.isEmpty()) {
        mView->append(QString::fromLatin1("<font color=\"red\">Error: %1</font>\n").arg(error));
        return;
    }

    mView->append(QString::fromLatin1("<font color=\"green\">Success</font>: Query took %1 msecs ").arg(duration));
    if (query.startsWith(QLatin1String("SELECT"))) {
        mView->append(QString::fromLatin1("Fetched %1 results").arg(resultsCount));
    } else {
        mView->append(QString::fromLatin1("Affected %1 rows").arg(resultsCount));
    }

    if (!result.isEmpty()) {
        const QVariantList headerRow = result.first();
        QString header;
        for (int i = 0; i < headerRow.size(); ++i) {
            if (i > 0) {
                header += QLatin1String(" | ");
            }
            header += headerRow.at(i).toString();
        }
        mView->append(header);

        QString sep;
        mView->append(sep.fill(QLatin1Char('-'), header.length()));

        for (int row = 1; row < result.count(); ++row) {
            const QVariantList columns = result.at(row);
            QString rowStr;
            for (int column = 0; column < columns.count(); ++column) {
                if (column > 0) {
                    rowStr += QLatin1String(" | ");
                }
                rowStr += variantToString(columns.at(column));
            }
            mView->append(rowStr);
        }
    }

    mView->append(QLatin1String("\n"));
}

QString QueryDebugger::variantToString(const QVariant &val)
{
    if (val.canConvert(QVariant::String)) {
        return val.toString();
    } else if (val.canConvert(QVariant::QVariant::DateTime)) {
        return val.toDateTime().toString(Qt::ISODate);
    }

    QDBusArgument arg = val.value<QDBusArgument>();
    if (arg.currentType() == QDBusArgument::StructureType) {
        QDateTime t = qdbus_cast<QDateTime>(arg);
        if (t.isValid()) {
            return t.toString(Qt::ISODate);
        }
    }

    return QString();
}
#include "querydebugger.moc"
