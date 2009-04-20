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

#include "qcsvreader.h"

#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>

QCsvBuilderInterface::~QCsvBuilderInterface()
{
}

class QCsvReader::Private
{
  public:
    Private( QCsvBuilderInterface *builder )
      : mBuilder( builder )
    {
      mTextQuote = QChar( '"' );
      mDelimiter = QChar( ' ' );
      mStartRow = 0;
      mCodec = QTextCodec::codecForLocale();

      mIgnoreDuplicates = true;
    }

    void emitBeginLine( uint row );
    void emitEndLine( uint row );
    void emitField( const QString &data, int row, int column );

    QCsvBuilderInterface *mBuilder;
    QTextCodec *mCodec;
    QChar mTextQuote;
    QChar mDelimiter;

    uint mStartRow;
    bool mIgnoreDuplicates;
};

void QCsvReader::Private::emitBeginLine( uint row )
{
  if ( (row - mStartRow) > 0 )
    mBuilder->beginLine();
}

void QCsvReader::Private::emitEndLine( uint row )
{
  if ( (row - mStartRow) > 0 )
    mBuilder->endLine();
}

void QCsvReader::Private::emitField( const QString &data, int row, int column )
{
  if ( (row - mStartRow) > 0 )
    mBuilder->field( data, row - mStartRow - 1, column - 1 );
}

QCsvReader::QCsvReader( QCsvBuilderInterface *builder )
  : d( new Private( builder ) )
{
  Q_ASSERT( builder );
}

QCsvReader::~QCsvReader()
{
  delete d;
}

bool QCsvReader::read( QIODevice *device )
{
  int row, column;
  bool lastCharDelimiter = false;
  bool ignoreDups = d->mIgnoreDuplicates;
  enum { S_START, S_QUOTED_FIELD, S_MAYBE_END_OF_QUOTED_FIELD, S_END_OF_QUOTED_FIELD,
         S_MAYBE_NORMAL_FIELD, S_NORMAL_FIELD } state = S_START;

  QChar x;
  QString field;

  row = column = 1;

  d->mBuilder->begin();
  d->emitBeginLine( row );

  if ( !device->isOpen() ) {
    d->mBuilder->error( "Device is not open" );
    d->emitEndLine( row );
    d->mBuilder->end();
    return false;
  }

  QTextStream inputStream( device );
  inputStream.setCodec( d->mCodec );

  int maxColumn = 0;
  while ( !inputStream.atEnd() ) {
    inputStream >> x; // read one char

    if ( x == '\r' ) inputStream >> x; // eat '\r', to handle DOS/LOSEDOWS files correctly

    switch ( state ) {
      case S_START:
        if ( x == d->mTextQuote ) {
          state = S_QUOTED_FIELD;
        } else if ( x == d->mDelimiter ) {
          if ( ( ignoreDups == false ) || ( lastCharDelimiter == false ) )
            ++column;
          lastCharDelimiter = true;
        } else if ( x == '\n' ) {
          d->emitEndLine( row );
          ++row;
          column = 1;
          d->emitBeginLine( row );
        } else {
          field += x;
          state = S_MAYBE_NORMAL_FIELD;
        }
        break;
      case S_QUOTED_FIELD:
        if ( x == d->mTextQuote ) {
          state = S_MAYBE_END_OF_QUOTED_FIELD;
        } else if ( x == '\n' &&  d->mTextQuote.isNull() ) {
          d->emitField( field, row, column );
          field.clear();
          if ( x == '\n' ) {
            d->emitEndLine( row );
            ++row;
            column = 1;
            d->emitBeginLine( row );
          } else {
            if ( ( ignoreDups == false ) || ( lastCharDelimiter == false ) )
              ++column;
            lastCharDelimiter = true;
          }
          state = S_START;
        } else {
          field += x;
        }
        break;
      case S_MAYBE_END_OF_QUOTED_FIELD:
        if ( x == d->mTextQuote ) {
          field += x;
          state = S_QUOTED_FIELD;
        } else if ( x == d->mDelimiter || x == '\n' ) {
          d->emitField( field, row, column );
          field.clear();
          if ( x == '\n' ) {
            d->emitEndLine( row );
            ++row;
            column = 1;
            d->emitBeginLine( row );
          } else {
            if ( ( ignoreDups == false ) || ( lastCharDelimiter == false ) )
              ++column;
            lastCharDelimiter = true;
          }
          state = S_START;
        } else {
          state = S_END_OF_QUOTED_FIELD;
        }
        break;
      case S_END_OF_QUOTED_FIELD:
        if ( x == d->mDelimiter || x == '\n' ) {
          d->emitField( field, row, column );
          field.clear();
          if ( x == '\n' ) {
            d->emitEndLine( row );
            ++row;
            column = 1;
            d->emitBeginLine( row );
          } else {
            if ( ( ignoreDups == false ) || ( lastCharDelimiter == false ) )
              ++column;
            lastCharDelimiter = true;
          }
          state = S_START;
        } else {
          state = S_END_OF_QUOTED_FIELD;
        }
        break;
      case S_MAYBE_NORMAL_FIELD:
        if ( x == d->mTextQuote ) {
          field.clear();
          state = S_QUOTED_FIELD;
          break;
        }
      case S_NORMAL_FIELD:
        if ( x == d->mDelimiter || x == '\n' ) {
          d->emitField( field, row, column );
          field.clear();
          if ( x == '\n' ) {
            d->emitEndLine( row );
            ++row;
            column = 1;
            d->emitBeginLine( row );
          } else {
            if ( ( ignoreDups == false ) || ( lastCharDelimiter == false ) )
              ++column;
            lastCharDelimiter = true;
          }
          state = S_START;
        } else {
          field += x;
        }
    }
    if ( x != d->mDelimiter )
      lastCharDelimiter = false;

    if ( column > maxColumn )
      maxColumn = column;
  }

  // file with only one line without '\n'
  if ( field.length() > 0 ) {
    d->emitField( field, row, column );
    ++row;
    field.clear();
  }

  d->emitEndLine( row );
  d->mBuilder->end();

  return true;
}

void QCsvReader::setTextQuote( const QChar &textQuote )
{
  d->mTextQuote = textQuote;
}

QChar QCsvReader::textQuote() const
{
  return d->mTextQuote;
}

void QCsvReader::setDelimiter( const QChar &delimiter )
{
  d->mDelimiter = delimiter;
}

QChar QCsvReader::delimiter() const
{
  return d->mDelimiter;
}

void QCsvReader::setStartRow( uint startRow )
{
  d->mStartRow = startRow;
}

uint QCsvReader::startRow() const
{
  return d->mStartRow;
}

void QCsvReader::setTextCodec( QTextCodec *textCodec )
{
  d->mCodec = textCodec;
}

QTextCodec *QCsvReader::textCodec() const
{
  return d->mCodec;
}


class QCsvStandardBuilder::Private
{
  public:
    Private()
    {
      init();
    }

    void init();

    QString mLastErrorString;
    uint mRowCount;
    uint mColumnCount;
    QList<QStringList> mRows;
};

void QCsvStandardBuilder::Private::init()
{
  mRows.clear();
  mRowCount = 0;
  mColumnCount = 0;
  mLastErrorString.clear();
}

QCsvStandardBuilder::QCsvStandardBuilder()
  : d( new Private )
{
}

QCsvStandardBuilder::~QCsvStandardBuilder()
{
  delete d;
}

QString QCsvStandardBuilder::lastErrorString() const
{
  return d->mLastErrorString;
}

uint QCsvStandardBuilder::rowCount() const
{
  return d->mRowCount;
}

uint QCsvStandardBuilder::columnCount() const
{
  return d->mColumnCount;
}

QString QCsvStandardBuilder::data( uint row, uint column ) const
{
  if ( row > d->mRowCount || column > d->mColumnCount || column >= (uint)d->mRows[ row ].count() )
    return QString();

  return d->mRows[ row ][ column ];
}

void QCsvStandardBuilder::begin()
{
  d->init();
}

void QCsvStandardBuilder::beginLine()
{
  d->mRows.append( QStringList() );
  d->mRowCount++;
}

void QCsvStandardBuilder::field( const QString &data, uint row, uint column )
{
  const uint size = d->mRows[ row ].size();
  if ( column >= size ) {
    for ( uint i = column; i < size + 1; ++i )
      d->mRows[ row ].append( QString() );
  }

  d->mRows[ row ][ column ] = data;

  d->mColumnCount = qMax( d->mColumnCount, column + 1 );
}

void QCsvStandardBuilder::endLine()
{
}

void QCsvStandardBuilder::end()
{
}

void QCsvStandardBuilder::error( const QString &errorMsg )
{
  d->mLastErrorString = errorMsg;
}

#include "qcsvreader.moc"
