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

#ifndef QUERYDEBUGGER_H
#define QUERYDEBUGGER_H

#include <QWidget>
#include <QtCore/QMap>
#include <QtCore/QVariant>

#include "storagedebuggerinterface.h"

class KTextEdit;

class QueryDebugger : public QWidget
{
    Q_OBJECT

public:
    explicit QueryDebugger(QWidget *parent = 0);
    virtual ~QueryDebugger();

private Q_SLOTS:
    void contextMenu(const QPoint &pos);
    void addQuery(double sequence, uint duration, const QString &query,
                  const QMap<QString, QVariant> &values, int resultsCount,
                  const QList<QList<QVariant> > &result, const QString &error);

private:
    QString variantToString(const QVariant &val);

    org::freedesktop::Akonadi::StorageDebugger *mDebugger;

    KTextEdit *mView;
};

#endif // QUERYDEBUGGER_H
