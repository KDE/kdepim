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

#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QStringList>

CsvParser::CsvParser( QObject *parent )
  : QThread( parent ), mDevice( 0 )
{
  mReader = new QCsvReader( this );
}

CsvParser::~CsvParser()
{
  delete mReader;
}

void CsvParser::load( QIODevice *device )
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

void CsvParser::field( const QString &data, uint row, uint column )
{
  const int tmp = qMax( mColumnCount, (int)column + 1 );
  if ( tmp != mColumnCount ) {
    mColumnCount = tmp;
    emit columnCountChanged( tmp );
  }

  emit dataChanged( data, row, column );
}

void CsvParser::endLine()
{
  mCacheCounter++;
  if ( mCacheCounter == 50 ) {
    emit rowCountChanged( mRowCount );
    mCacheCounter = 0;
  }
}

void CsvParser::end()
{
  emit rowCountChanged( mRowCount );
}

void CsvParser::error( const QString& )
{
}

void CsvParser::run()
{
  if ( !mDevice->isOpen() )
    mDevice->open( QIODevice::ReadOnly );

  mDevice->reset();
  mReader->read( mDevice );
}

class QCsvModel::Private
{
  public:
    Private( QCsvModel *model )
      : mParent( model ),
        mDevice( 0 ), mRowCount( 0 ), mColumnCount( 0 )
    {
    }

    void columnCountChanged( int columns );
    void rowCountChanged( int rows );
    void fieldChanged( const QString &data, int row, int column );

    QCsvModel *mParent;
    CsvParser *mParser;
    QMap< QPair<int, int>, QString> mFields;
    QIODevice *mDevice;

    int mRowCount;
    int mColumnCount;
};

void QCsvModel::Private::columnCountChanged( int columns )
{
  mColumnCount = columns;
  emit mParent->layoutChanged();
}

void QCsvModel::Private::rowCountChanged( int rows )
{
  mRowCount = rows;
  emit mParent->layoutChanged();
}

void QCsvModel::Private::fieldChanged( const QString &data, int row, int column )
{
  mFields.insert( QPair<int, int>( row, column ), data );
}

QCsvModel::QCsvModel( QObject *parent )
  : QAbstractTableModel( parent ), d( new Private( this ) )
{
  d->mParser = new CsvParser( this );

  connect( d->mParser, SIGNAL( columnCountChanged( int ) ),
           this, SLOT( columnCountChanged( int ) ), Qt::QueuedConnection );
  connect( d->mParser, SIGNAL( rowCountChanged( int ) ),
           this, SLOT( rowCountChanged( int ) ), Qt::QueuedConnection );
  connect( d->mParser, SIGNAL( dataChanged( const QString&, int, int ) ),
           this, SLOT( fieldChanged( const QString&, int, int ) ), Qt::QueuedConnection );
  connect( d->mParser, SIGNAL( finished() ), this, SIGNAL( finished() ) );
}

QCsvModel::~QCsvModel()
{
}

bool QCsvModel::load( QIODevice *device )
{
  d->mDevice = device;
  d->mRowCount = 0;
  d->mColumnCount = 0;
  d->mFields.clear();

  emit layoutChanged();

  d->mParser->load( device );

  return true;
}

void QCsvModel::setTextQuote( const QChar &textQuote )
{
  const bool isRunning = d->mParser->isRunning();

  if ( isRunning ) {
    d->mParser->terminate();
    d->mParser->wait();
  }

  d->mParser->reader()->setTextQuote( textQuote );

  if ( isRunning )
    load( d->mDevice );
}

QChar QCsvModel::textQuote() const
{
  return d->mParser->reader()->textQuote();
}

void QCsvModel::setDelimiter( const QChar &delimiter )
{
  const bool isRunning = d->mParser->isRunning();

  if ( isRunning ) {
    d->mParser->terminate();
    d->mParser->wait();
  }

  d->mParser->reader()->setDelimiter( delimiter );

  if ( isRunning )
    load( d->mDevice );
}

QChar QCsvModel::delimiter() const
{
  return d->mParser->reader()->delimiter();
}

void QCsvModel::setStartRow( uint startRow )
{
  const bool isRunning = d->mParser->isRunning();

  if ( isRunning ) {
    d->mParser->terminate();
    d->mParser->wait();
  }

  d->mParser->reader()->setStartRow( startRow );

  if ( isRunning )
    load( d->mDevice );
}

uint QCsvModel::startRow() const
{
  return d->mParser->reader()->startRow();
}

void QCsvModel::setTextCodec( QTextCodec *textCodec )
{
  const bool isRunning = d->mParser->isRunning();

  if ( isRunning ) {
    d->mParser->terminate();
    d->mParser->wait();
  }

  d->mParser->reader()->setTextCodec( textCodec );

  if ( isRunning )
    load( d->mDevice );
}

QTextCodec *QCsvModel::textCodec() const
{
  return d->mParser->reader()->textCodec();
}

int QCsvModel::columnCount( const QModelIndex& ) const
{
  return d->mColumnCount;
}

int QCsvModel::rowCount( const QModelIndex& ) const
{
  return d->mRowCount;
}

QVariant QCsvModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  const QPair<int, int> pair( index.row(), index.column() );
  if ( !d->mFields.contains( pair ) )
    return QVariant();

  const QString data = d->mFields.value( pair );

  if ( role == Qt::DisplayRole )
    return data;
  else
    return QVariant();
}

#include "qcsvmodel.moc"
#include "qcsvmodel_p.moc"
