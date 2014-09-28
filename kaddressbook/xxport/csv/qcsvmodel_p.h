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

#ifndef QCSVMODEL_P_H
#define QCSVMODEL_P_H

#include "qcsvreader.h"

#include <QtCore/QThread>

class CsvParser : public QThread, public QCsvBuilderInterface
{
    Q_OBJECT

public:
    explicit CsvParser(QObject *parent);
    ~CsvParser();

    void load(QIODevice *device);

    void begin();
    void beginLine();
    void field(const QString &data, uint row, uint column);
    void endLine();
    void end();
    void error(const QString &errorMsg);

    QCsvReader *reader()
    {
        return mReader;
    }

Q_SIGNALS:
    void columnCountChanged(int columns);
    void rowCountChanged(int rows);
    void dataChanged(const QString &data, int row, int column);
    void ended();

protected:
    virtual void run();

private:
    QCsvReader *mReader;
    QIODevice *mDevice;
    int mRowCount;
    int mColumnCount;
    int mCacheCounter;
};

#endif
