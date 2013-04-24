/*
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

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

#ifndef KPIMIDENTITIES_IDENTITYMANAGER_H
#define KPIMIDENTITIES_IDENTITYMANAGER_H

#include <kpimidentities/kpimidentities_export.h>
#include <kconfiggroup.h>
#include <QtCore/QObject>

class KConfigBase;
class KConfig;
class QStringList;

namespace KPIMIdentities
{

  class Identity;
  /**
   * @short Manages the list of identities.
   * @author Marc Mutz <mutz@kde.org>
   **/
class KPIMIDENTITIES_EXPORT IdentityManager : public QObject
{
    Q_OBJECT
    public:
      /**
       * Create an identity manager, which loads the emailidentities file
       * to create identities.
       * @param readonly if true, no changes can be made to the identity manager
       * This means in particular that if there is no identity configured,
       * the default identity created here will not be saved.
       * It is assumed that a minimum of one identity is always present.
       */
      explicit IdentityManager( bool readonly = false, QObject *parent=0,
                                const char *name=0 );
      virtual ~IdentityManager();

    public:
      typedef QList<Identity>::Iterator Iterator;
      typedef QList<Identity>::ConstIterator ConstIterator;

      /**
       * Typedef for STL style iterator
       */
      typedef Iterator iterator;

      /**
       * Typedef for STL style iterator
       */
      typedef ConstIterator const_iterator;

      /** @return a unique name for a new identity based on @p name
       *  @param name the name of the base identity
       */
      QString makeUnique( const QString &name ) const;

      /** @return whether the @p name is unique
       *  @param name the name to be examined
       */
      bool isUnique( const QString &name ) const;

      /** Commit changes to disk and emit changed() if necessary. */
      void commit();

      /** Re-read the config from disk and forget changes. */
      void rollback();

      /** Check whether there are any unsaved changes. */
      bool hasPendingChanges() const;

      /** @return the list of identities */
      QStringList identities() const;

      /** Convenience method.

          @return the list of (shadow) identities, ie. the ones currently
          under configuration.
      */
      QStringList shadowIdentities() const;

      /** Sort the identities by name (the default is always first). This
          operates on the @em shadow list, so you need to @ref commit for
          the changes to take effect.
      **/
      void sort();

      /** @return an identity whose address matches any in @p addresses
                  or @ref Identity::null if no such identity exists.
          @param addresses the string of addresses to scan for matches
      **/
      const Identity &identityForAddress( const QString &addresses ) const;

      /** @return true if @p addressList contains any of our addresses,
                  false otherwise.
          @param addressList the addressList to examine
          @see #identityForAddress
      **/
      bool thatIsMe( const QString &addressList ) const;

      /** @return the identity with Unique Object Identifier (UOID) @p
                  uoid or @ref Identity::null if not found.
          @param uoid the Unique Object Identifier to find identity with
       **/
      const Identity &identityForUoid( uint uoid ) const;

      /** Convenience menthod.

          @return the identity with Unique Object Identifier (UOID) @p
                  uoid or the default identity if not found.
          @param uoid the Unique Object Identifier to find identity with
      **/
      const Identity &identityForUoidOrDefault( uint uoid ) const;

      /** @return the default identity */
      const Identity &defaultIdentity() const;

      /** Sets the identity with Unique Object Identifier (UOID) @p uoid
          to be new the default identity. As usual, use @ref commit to
          make this permanent.
          
          @param uoid the default identity to set
          @return false if an identity with UOID @p uoid was not found
      **/
      bool setAsDefault( uint uoid );

      /** @return the identity named @p identityName. This method returns a
          reference to the identity that can be modified. To let others
          see this change, use @ref commit.
          @param identityName the identity name to return modifiable reference
      **/
      Identity &modifyIdentityForName( const QString &identityName );

      /** @return the identity with Unique Object Identifier (UOID) @p uoid.
          This method returns a reference to the identity that can
          be modified. To let others see this change, use @ref commit.
      **/
      Identity &modifyIdentityForUoid( uint uoid );

      /** Removes the identity with name @p identityName
          Will return false if the identity is not found,
          or when one tries to remove the last identity.
          @param identityName the identity to remove
       **/
      bool removeIdentity( const QString &identityName );

      /**
       * Removes the identity with name @p identityName
       * Will return @c false if the identity is not found, @c true otherwise.
       *
       * @note In opposite to removeIdentity, this method allows to remove the
       *       last remaining identity.
       *
       * @since 4.6
       */
      bool removeIdentityForced( const QString &identityName );

      ConstIterator begin() const;
      ConstIterator end() const;
      /// Iterator used by the configuration dialog, which works on a separate list
      /// of identities, for modification. Changes are made effective by commit().
      Iterator modifyBegin();
      Iterator modifyEnd();

      Identity &newFromScratch( const QString &name );
      Identity &newFromControlCenter( const QString &name );
      Identity &newFromExisting( const Identity &other,
                                  const QString &name=QString() );

      /** Returns the list of all email addresses (only name@host) from all
          identities */
      QStringList allEmails() const;

    Q_SIGNALS:
      /** Emitted whenever a commit changes any configure option */
      void changed();
      /** Emitted whenever the identity with Unique Object Identifier
          (UOID) @p uoid changed. Useful for more fine-grained change
          notifications than what is possible with the standard @ref
          changed() signal. */
      void changed( uint uoid );
      /** Emitted whenever the identity @p ident changed. Useful for more
          fine-grained change notifications than what is possible with the
          standard @ref changed() signal. */
      void changed( const KPIMIdentities::Identity &ident );
      /** Emitted on @ref commit() for each deleted identity. At the time
          this signal is emitted, the identity does still exist and can be
          retrieved by @ref identityForUoid() if needed */
      void deleted( uint uoid );
      /** Emitted on @ref commit() for each new identity */
      void added( const KPIMIdentities::Identity &ident );

    protected:
      /**
       * This is called when no identity has been defined, so we need to
       * create a default one. The parameters are filled with some default
       * values from KUser, but reimplementations of this method can give
       * them another value.
       */
      virtual void createDefaultIdentity( QString&/*fullName*/,
                                          QString&/*emailAddress*/ ) {}

    protected Q_SLOTS:
      void slotRollback();

    protected:
      /** The list that will be seen by everyone */
      QList<Identity> mIdentities;
      /** The list that will be seen by the config dialog */
      QList<Identity> mShadowIdentities;

    Q_SIGNALS:
      void identitiesChanged( const QString &id );

    private Q_SLOTS:
      // Connected to the DBus signal
      void slotIdentitiesChanged( const QString &id );

    private:
      void writeConfig() const;
      void readConfig( KConfig *config );
      QStringList groupList( KConfig *config ) const;
      void createDefaultIdentity();

      // returns a new Unique Object Identifier
      int newUoid();

    private:
      KConfig *mConfig;
      bool mReadOnly;
};

} // namespace

#endif // _KMAIL_IDENTITYMANAGER_H_
