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

#ifndef QCSVMODEL_H
#define QCSVMODEL_H

#include <QtCore/QAbstractTableModel>

class QCsvModel : public QAbstractTableModel
{
  Q_OBJECT

  public:
    /**
     * Creates a new csv model.
     */
    QCsvModel( QObject *parent );

    /**
     * Destroys the csv model.
     */
    ~QCsvModel();

    /**
     * Loads the data from the @p device into the model.
     */
    bool load( QIODevice *device );

    /**
     * Sets the character that is used for quoting. The default is '"'.
     */
    void setTextQuote( const QChar &textQuote );

    /**
     * Returns the character that is used for quoting.
     */
    QChar textQuote() const;

    /**
     * Sets the character that is used as delimiter for fields.
     * The default is ' '.
     */
    void setDelimiter( const QChar &delimiter );

    /**
     * Returns the delimiter that is used as delimiter for fields.
     */
    QChar delimiter() const;

    /**
     * Sets the row from where the parsing shall be started.
     *
     * Some csv files have some kind of header in the first line with
     * the column titles. To retrieve only the real data, set the start row
     * to '1' in this case.
     *
     * The default start row is 0.
     */
    void setStartRow( uint startRow );

    /**
     * Returns the start row.
     */
    uint startRow() const;

    /**
     * Sets the text codec that shall be used for parsing the csv list.
     *
     * The default is the system locale.
     */
    void setTextCodec( QTextCodec *textCodec );

    /**
     * Returns the text codec that is used for parsing the csv list.
     */
    QTextCodec *textCodec() const;

    /**
     * Inherited from QAbstractTableModel.
     */
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    /**
     * Inherited from QAbstractTableModel.
     */
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    /**
     * Inherited from QAbstractTableModel.
     */
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    /**
     * Inherited from QAbstractTableModel.
     */
    virtual bool setData( const QModelIndex &index, const QVariant &data, int role = Qt::EditRole );

    /**
     * Inherited from QAbstractTableModel.
     */
    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

  Q_SIGNALS:
    /**
     * This signal is emitted whenever the model has loaded all data.
     */
    void finishedLoading();

  private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void columnCountChanged( int ) )
    Q_PRIVATE_SLOT( d, void rowCountChanged( int ) )
    Q_PRIVATE_SLOT( d, void fieldChanged( const QString&, int, int ) )
    Q_PRIVATE_SLOT( d, void finishedLoading() )
};

#endif
