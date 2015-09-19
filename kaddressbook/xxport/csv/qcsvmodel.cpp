/*
  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "qcsvmodel.h"
#include "qcsvmodel_p.h"
#include "qcsvreader.h"

#include <QMap>
#include <QPair>
#include <QStringList>
#include <QVector>

CsvParser::CsvParser(QObject *parent)
    : QThread(parent), mDevice(Q_NULLPTR), mRowCount(0), mColumnCount(0), mCacheCounter(0)
{
    mReader = new QCsvReader(this);
}

CsvParser::~CsvParser()
{
    delete mReader;
}

void CsvParser::load(QIODevice *device)
{
    mDevice = device;

    start();
}

void CsvParser::begin()
{
    mCacheCounter = 0;
    mRowCount = 0;
    mColumnCount = 0;
}

void CsvParser::beginLine()
{
    mRowCount++;
}

void CsvParser::field(const QString &data, uint row, uint column)
{
    const int tmp = qMax(mColumnCount, (int)column + 1);
    if (tmp != mColumnCount) {
        mColumnCount = tmp;
        Q_EMIT columnCountChanged(tmp);
    }

    Q_EMIT dataChanged(data, row, column);
}

void CsvParser::endLine()
{
    mCacheCounter++;
    if (mCacheCounter == 50) {
        Q_EMIT rowCountChanged(mRowCount);
        mCacheCounter = 0;
    }
}

void CsvParser::end()
{
    Q_EMIT rowCountChanged(mRowCount);
    Q_EMIT ended();
}

void CsvParser::error(const QString &)
{
}

void CsvParser::run()
{
    if (!mDevice->isOpen()) {
        mDevice->open(QIODevice::ReadOnly);
    }

    mDevice->reset();
    mReader->read(mDevice);
}

class QCsvModel::Private
{
public:
    Private(QCsvModel *model)
        : mParent(model), mParser(Q_NULLPTR),
          mDevice(Q_NULLPTR), mRowCount(0), mColumnCount(0)
    {
    }

    void columnCountChanged(int columns);
    void rowCountChanged(int rows);
    void fieldChanged(const QString &data, int row, int column);
    void finishedLoading();

    QCsvModel *mParent;
    CsvParser *mParser;
    QVector<QString> mFieldIdentifiers;
    QMap< QPair<int, int>, QString> mFields;
    QIODevice *mDevice;

    int mRowCount;
    int mColumnCount;
};

void QCsvModel::Private::columnCountChanged(int columns)
{
    mColumnCount = columns;
    mFieldIdentifiers.resize(columns);
    mFieldIdentifiers[ columns - 1 ] = QStringLiteral("0");
    Q_EMIT mParent->layoutChanged();
}

void QCsvModel::Private::rowCountChanged(int rows)
{
    mRowCount = rows;
    Q_EMIT mParent->layoutChanged();
}

void QCsvModel::Private::fieldChanged(const QString &data, int row, int column)
{
    mFields.insert(QPair<int, int>(row, column), data);
}

void QCsvModel::Private::finishedLoading()
{
    Q_EMIT mParent->finishedLoading();
}

QCsvModel::QCsvModel(QObject *parent)
    : QAbstractTableModel(parent), d(new Private(this))
{
    d->mParser = new CsvParser(this);

    connect(d->mParser, SIGNAL(columnCountChanged(int)),
            this, SLOT(columnCountChanged(int)), Qt::QueuedConnection);
    connect(d->mParser, SIGNAL(rowCountChanged(int)),
            this, SLOT(rowCountChanged(int)), Qt::QueuedConnection);
    connect(d->mParser, SIGNAL(dataChanged(QString,int,int)),
            this, SLOT(fieldChanged(QString,int,int)), Qt::QueuedConnection);
    connect(d->mParser, &CsvParser::ended, this, &QCsvModel::finishedLoading);
}

QCsvModel::~QCsvModel()
{
    delete d;
}

bool QCsvModel::load(QIODevice *device)
{
    d->mDevice = device;
    d->mRowCount = 0;
    d->mColumnCount = 0;

    Q_EMIT layoutChanged();

    d->mParser->load(device);

    return true;
}

void QCsvModel::setTextQuote(const QChar &textQuote)
{
    const bool isRunning = d->mParser->isRunning();

    if (isRunning) {
        d->mParser->reader()->terminate();
        d->mParser->wait();
    }

    d->mParser->reader()->setTextQuote(textQuote);

    if (isRunning) {
        load(d->mDevice);
    }
}

QChar QCsvModel::textQuote() const
{
    return d->mParser->reader()->textQuote();
}

void QCsvModel::setDelimiter(const QChar &delimiter)
{
    const bool isRunning = d->mParser->isRunning();

    if (isRunning) {
        d->mParser->reader()->terminate();
        d->mParser->wait();
    }

    d->mParser->reader()->setDelimiter(delimiter);

    if (isRunning) {
        load(d->mDevice);
    }
}

QChar QCsvModel::delimiter() const
{
    return d->mParser->reader()->delimiter();
}

void QCsvModel::setStartRow(uint startRow)
{
    const bool isRunning = d->mParser->isRunning();

    if (isRunning) {
        d->mParser->reader()->terminate();
        d->mParser->wait();
    }

    d->mParser->reader()->setStartRow(startRow);

    if (isRunning) {
        load(d->mDevice);
    }
}

uint QCsvModel::startRow() const
{
    return d->mParser->reader()->startRow();
}

void QCsvModel::setTextCodec(QTextCodec *textCodec)
{
    const bool isRunning = d->mParser->isRunning();

    if (isRunning) {
        d->mParser->reader()->terminate();
        d->mParser->wait();
    }

    d->mParser->reader()->setTextCodec(textCodec);

    if (isRunning) {
        load(d->mDevice);
    }
}

QTextCodec *QCsvModel::textCodec() const
{
    return d->mParser->reader()->textCodec();
}

int QCsvModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return d->mColumnCount;
    } else {
        return 0;
    }
}

int QCsvModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return d->mRowCount + 1; // +1 for the header row
    } else {
        return 0;
    }
}

QVariant QCsvModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() == 0) {
        if (index.column() >= d->mFieldIdentifiers.count()) {
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return d->mFieldIdentifiers.at(index.column());
        }

        return QVariant();
    }

    const QPair<int, int> pair(index.row() - 1, index.column());
    if (!d->mFields.contains(pair)) {
        return QVariant();
    }

    const QString data = d->mFields.value(pair);

    if (role == Qt::DisplayRole) {
        return data;
    } else {
        return QVariant();
    }
}

bool QCsvModel::setData(const QModelIndex &index, const QVariant &data, int role)
{
    if (role == Qt::EditRole && index.row() == 0 &&
            index.column() <= d->mFieldIdentifiers.count()) {
        d->mFieldIdentifiers[ index.column() ] = data.toString();

        Q_EMIT dataChanged(index, index);
        return true;
    }

    return false;
}

Qt::ItemFlags QCsvModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.row() == 0) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

#include "moc_qcsvmodel.cpp"
#include "moc_qcsvmodel_p.cpp"
