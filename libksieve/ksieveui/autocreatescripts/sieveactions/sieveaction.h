/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef SIEVEACTION_H
#define SIEVEACTION_H

#include <QObject>
class QDomElement;
namespace KSieveUi
{
class SieveAction : public QObject
{
    Q_OBJECT
public:
    SieveAction(const QString &name, const QString &label, QObject *parent = 0);
    virtual ~SieveAction();

    QString name() const;
    QString label() const;

    /**
     * Static function that creates a filter action of this type.
     */
    static SieveAction *newAction();

    virtual QWidget *createParamWidget(QWidget *parent) const;

    virtual bool setParamWidgetValue(const QDomElement &element, QWidget *parent, QString &error);

    virtual QString code(QWidget *) const;

    virtual QStringList needRequires(QWidget *parent) const;

    virtual bool needCheckIfServerHasCapability() const;

    virtual QString serverNeedsCapability() const;

    virtual QString help() const;
    virtual QString href() const;

    QString comment() const;

    void setComment(const QString &comment);

    void unknownTag(const QString &tag, QString &error);
    void unknowTagValue(const QString &tagValue, QString &error);
    void tooManyArgument(const QString &tagName, int index, int maxValue, QString &error);
    void serverDoesNotSupportFeatures(const QString &feature, QString &error);

Q_SIGNALS:
    void valueChanged();

private:
    QString mName;
    QString mLabel;
    QString mComment;
};
}

#endif // SIEVEACTION_H
