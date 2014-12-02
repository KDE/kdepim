/*
  This file is part of KAddressBook.
  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef XXPORT_H
#define XXPORT_H

#include <KContacts/Addressee>

#include <QtCore/QMap>

/**
 * @short The base class for all import/export modules.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class XXPort
{
public:
    /**
     * Creates a new xxport object.
     *
     * @param parent The parent widget that shall be used as parent
     *               for GUI components.
     */
    explicit XXPort(QWidget *parent = Q_NULLPTR);

    /**
     * Destroys the xxport object.
     */
    virtual ~XXPort();

    /**
     * Imports a list of contacts.
     */
    virtual KContacts::Addressee::List importContacts() const = 0;

    /**
     * Exports the list of @p contacts.
     */
    virtual bool exportContacts(const KContacts::Addressee::List &contacts) const = 0;

    /**
     * Sets module specific options.
     *
     * @param key The option key.
     * @param value The option value.
     */
    void setOption(const QString &key, const QString &value);

    /**
     * Returns the module specific option value for the given @p key.
     */
    QString option(const QString &key) const;

protected:
    /**
     * Returns the parent widget that can be used as parent for
     * GUI components.
     */
    QWidget *parentWidget() const;

    /**
     * Returns a file name depending on the passed @p contact.
     */
    QString contactFileName(const KContacts::Addressee &contact) const;

private:
    QWidget *mParentWidget;
    QMap<QString, QString> mOptions;
};

#endif
