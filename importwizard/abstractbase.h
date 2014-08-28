/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ABSTRACTBASE_H
#define ABSTRACTBASE_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVariant>

namespace PimCommon
{
class CreateResource;
}

class AbstractBase : public QObject
{
    Q_OBJECT
public:
    explicit AbstractBase();
    virtual ~AbstractBase();

    QString createResource(const QString &resources , const QString &name, const QMap<QString, QVariant> &settings);

private Q_SLOTS:
    void slotCreateResourceError(const QString &);
    void slotCreateResourceInfo(const QString &);

protected:
    virtual void addImportInfo(const QString &log) = 0;
    virtual void addImportError(const QString &log) = 0;

private:
    PimCommon::CreateResource *mCreateResource;
};

#endif // ABSTRACTBASE_H
