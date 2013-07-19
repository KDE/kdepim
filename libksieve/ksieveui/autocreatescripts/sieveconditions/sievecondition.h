/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#ifndef SIEVECONDITION_H
#define SIEVECONDITION_H

#include <QObject>
class QDomDocument;
namespace KSieveUi {
class SieveCondition : public QObject
{
    Q_OBJECT
public:
    SieveCondition(const QString &name, const QString &label, QObject *parent = 0);
    virtual ~SieveCondition();

    QString name() const;
    QString label() const;

    /**
     * Static function that creates a filter action of this type.
     */
    static SieveCondition *newAction();

    virtual QWidget *createParamWidget( QWidget *parent ) const;

    virtual QString code(QWidget *parent) const;

    virtual QStringList needRequires(QWidget *parent) const;

    virtual bool needCheckIfServerHasCapability() const;

    virtual QString serverNeedsCapability() const;

    virtual QString help() const;
    virtual void setParamWidgetValue(const QDomDocument &doc, QWidget *parent ) const;

private:
    QString mName;
    QString mLabel;
};
}

#endif // SIEVECONDITION_H
