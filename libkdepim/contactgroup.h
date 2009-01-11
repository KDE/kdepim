/*
  This file is part of libkabc.
  Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KABC_CONTACTGROUP_H
#define KABC_CONTACTGROUP_H

#include <QtCore/QList>
#include <QtCore/QSharedDataPointer>

#include "kdepim_export.h"

class QString;

namespace KPIM {

/**
 * This class represents a group of contacts.
 *
 * It can contain two types of contacts, either a reference
 * or data.
 * The reference entry is just an unique identifier which
 * identifies the real contact in the system.
 * The data entry contains a name and an email address.
 *
 * @since 4.2
 */
class KDEPIM_EXPORT ContactGroup
{
  public:

    /**
     * This class represents a contact reference
     */
    class KDEPIM_EXPORT Reference
    {
      public:
        /**
         * A list of contact references.
         */
        typedef QList<Reference> List;

        /**
         * Creates an empty contact reference.
         */
        Reference();

        /**
         * Creates a contact reference from an @p other reference.
         */
        Reference( const Reference &other );

        /**
         * Creates a contact reference for the given contact @p uid.
         */
        Reference( const QString &uid );

        /**
         * Destroys the contact reference.
         */
        ~Reference();

        /**
         * Sets the contact uid of the contact reference.
         */
        void setUid( const QString &uid );

        /**
         * Returns the contact uid of the contact reference.
         */
        QString uid() const;

        /**
         * Sets the preferred email address.
         */
        void setPreferredEmail( const QString &email );

        /**
         * Returns the preferred email address, or an empty string
         * if no preferred email address is set.
         */
        QString preferredEmail() const;

        /**
         * Inserts a custom entry.
         * If an entry with the same @p key already exists, it is
         * overwritten.
         *
         * @param key The unique key.
         * @param value The value.
         */
        void insertCustom( const QString &key, const QString &value );

        /**
         * Removes the custom entry with the given @p key.
         */
        void removeCustom( const QString &key );

        /**
         * Returns the value for the given @p key, or an empty string
         * if the entry for that key does not exists.
         */
        QString custom( const QString &key ) const;

        /**
         * @internal
         */
        Reference &operator=( const Reference & );

        /**
         * @internal
         */
        bool operator==( const Reference & ) const;

      private:
        class ReferencePrivate;
        QSharedDataPointer<ReferencePrivate> d;
    };

    /**
     * This class represents a contact data object
     */
    class KDEPIM_EXPORT Data
    {
      public:
        /**
         * A list of contact data.
         */
        typedef QList<Data> List;

        /**
         * Creates an empty contact data object.
         */
        Data();

        /**
         * Creates a contact data object from an @p other data object.
         */
        Data( const Data &other );

        /**
         * Creates a contact data object with the given @p name and @p email address.
         */
        Data( const QString &name, const QString &email );

        /**
         * Destroys the contact data object.
         */
        ~Data();

        /**
         * Sets the @p name of the contact data object.
         */
        void setName( const QString &name );

        /**
         * Returns the name of the contact data object.
         */
        QString name() const;

        /**
         * Sets the @p email address of the contact data object.
         */
        void setEmail( const QString &email );

        /**
         * Returns the email address of the contact data object.
         */
        QString email() const;

        /**
         * Inserts a custom entry.
         * If an entry with the same @p key already exists, it is
         * overwritten.
         *
         * @param key The unique key.
         * @param value The value.
         */
        void insertCustom( const QString &key, const QString &value );

        /**
         * Removes the custom entry with the given @p key.
         */
        void removeCustom( const QString &key );

        /**
         * Returns the value for the given @p key, or an empty string
         * if the entry for that key does not exists.
         */
        QString custom( const QString &key ) const;

        /**
         * @internal
         */
        Data &operator=( const Data & );

        /**
         * @internal
         */
        bool operator==( const Data & ) const;

      private:
        class DataPrivate;
        QSharedDataPointer<DataPrivate> d;
    };

    /**
     * A list of contact groups.
     */
    typedef QList<ContactGroup> List;

    /**
     * Creates an empty contact group.
     */
    ContactGroup();

    /**
     * Creates a contact group from an @p other group.
     */
    ContactGroup( const ContactGroup &other );

    /**
     * Creates a contact group with the given name.
     */
    ContactGroup( const QString &name );

    /**
     * Destroys the contact group.
     */
    ~ContactGroup();

    /**
     * Sets the unique @p id of the contact group.
     */
    void setId( const QString &id );

    /**
     * Returns the unique id of the contact group.
     */
    QString id() const;

    /**
     * Sets the i18n'd @p name of the contact group.
     */
    void setName( const QString &name );

    /**
     * Returns the i18n'd name of the contact group.
     */
    QString name() const;

    /**
     * Returns the number of contacts in this group.
     * That includes the contact references and contact data.
     */
    unsigned int count() const;

    /**
     * Returns the number of contact references in this group.
     */
    unsigned int referencesCount() const;

    /**
     * Returns the number of contact data objects in this group.
     */
    unsigned int dataCount() const;

    /**
     * Returns the contact reference at the given @p index.
     */
    Reference &reference( unsigned int index );

    /**
     * Returns the contact reference at the given @p index.
     */
    const Reference &reference( unsigned int index ) const;

    /**
     * Returns the contact data object at the given @p index.
     */
    Data &data( unsigned int index );

    /**
     * Returns the contact data object at the given @p index.
     */
    const Data &data( unsigned int index ) const;

    /**
     * Appends a new contact @p reference to the contact group.
     */
    void append( const Reference &reference );

    /**
     * Appends a new contact @p data object to the contact group.
     */
    void append( const Data &data );

    /**
     * Removes the given contact @p reference from the contact group.
     */
    void remove( const Reference &reference );

    /**
     * Removes the given contact @p data object from the contact group.
     */
    void remove( const Data &data );

    /**
     * @internal
     */
    ContactGroup &operator=( const ContactGroup & );

    /**
     * @internal
     */
    bool operator==( const ContactGroup & ) const;

    /**
     * Returns the MIME type used for Contact Groups
     */
    static QString mimeType();

  private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
