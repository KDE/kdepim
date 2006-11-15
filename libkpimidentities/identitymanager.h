/*  -*- c++ -*-
    identitymanager.h

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/
#ifndef _KPIM_IDENTITYMANAGER_H_
#define _KPIM_IDENTITYMANAGER_H_

#include <libkdepim/configmanager.h>


//Added by qt3to4:
#include <kdemacros.h>

class KConfigBase;
class KConfig;
class KMKernel;
class QStringList;
class KMIdentity;
class OrgKdePimIdentityManagerInterface;

namespace KPIM {

class Identity;
/**
 * @short Manages the list of identities.
 * @author Marc Mutz <mutz@kde.org>
 **/
class KDE_EXPORT IdentityManager : public ConfigManager
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.pim.IdentityManager")

public:
  /**
   * Create an identity manager, which loads the emailidentities file
   * to create identities.
   * @param readonly if true, no changes can be made to the identity manager
   * This means in particular that if there is no identity configured,
   * the default identity created here will not be saved.
   */
  IdentityManager( bool readonly = false, QObject * parent=0, const char * name=0 );
  virtual ~IdentityManager();

public:
  typedef QList<Identity>::Iterator Iterator;
  typedef QList<Identity>::ConstIterator ConstIterator;

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
  **/
  const Identity & identityForAddress( const QString & addresses ) const;

  /** @return true if @p addressList contains any of our addresses,
      false otherwise.
      @see #identityForAddress
  **/
  bool thatIsMe( const QString & addressList ) const;

  /** @deprecated
      @return the identity named @p identityName or @ref
      Identity::null if not found.
  **/
  const Identity & identityForName( const QString & identityName ) const;

  /** @return the identity with Unique Object Identifier (UOID) @p
      uoid or @ref Identity::null if not found.
   **/
  const Identity & identityForUoid( uint uoid ) const;

  /** @deprecated
      Convenience method.

      @return the identity named @p identityName or the default
      identity if not found.
  **/
  const Identity & identityForNameOrDefault( const QString & identityName ) const;

  /** Convenience menthod.

      @return the identity with Unique Object Identifier (UOID) @p
      uoid or the default identity if not found.
  **/
  const Identity & identityForUoidOrDefault( uint uoid ) const;

  /** @return the default identity */
  const Identity & defaultIdentity() const;

  /** @deprecated
      Sets the identity named @p identityName to be the new default
      identity. As usual, use @ref commit to make this permanent.

      @return false if an identity named @p identityName was not found
  **/
  bool setAsDefault( const QString & identityName );

  /** Sets the identity with Unique Object Identifier (UOID) @p uoid
      to be new the default identity. As usual, use @ref commit to
      make this permanent.

      @return false if an identity with UOID @p uoid was not found
  **/
  bool setAsDefault( uint uoid );

  /** @return the identity named @p identityName. This method returns a
      reference to the identity that can be modified. To let others
      see this change, use @ref commit.
  **/
  Identity & modifyIdentityForName( const QString & identityName );

  /** @return the identity with Unique Object Identifier (UOID) @p uoid.
      This method returns a reference to the identity that can
      be modified. To let others see this change, use @ref commit.
  **/
  Identity & modifyIdentityForUoid( uint uoid );

  /** Removes the identity with name @p identityName */
  bool removeIdentity( const QString & identityName );

  ConstIterator begin() const;
  ConstIterator end() const;
  /// Iterator used by the configuration dialog, which works on a separate list
  /// of identities, for modification. Changes are made effective by commit().
  Iterator modifyBegin();
  Iterator modifyEnd();

  Identity & newFromScratch( const QString & name );
  Identity & newFromControlCenter( const QString & name );
  Identity & newFromExisting( const Identity & other,
				const QString & name=QString() );

  /** Returns the list of all email addresses (only name@host) from all identities */
  QStringList allEmails() const;

signals:
  /** Emitted whenever the identity with Unique Object Identifier
      (UOID) @p uoid changed. Useful for more fine-grained change
      notifications than what is possible with the standard @ref
      changed() signal. */
  void changed( uint uoid );
  /** Emitted whenever the identity @p ident changed. Useful for more
      fine-grained change notifications than what is possible with the
      standard @ref changed() signal. */
  void changed( const KPIM::Identity & ident );
  /** Emitted on @ref commit() for each deleted identity. At the time
      this signal is emitted, the identity does still exist and can be
      retrieved by @ref identityForUoid() if needed */
  void deleted( uint uoid );
  /** Emitted on @ref commit() for each new identity */
  void added( const KPIM::Identity & ident );

protected:
  /**
   * This is called when no identity has been defined, so we need to create a default one
   * The parameters are filled with some default values from KUser,
   * but reimplementations of this method can give them another value.
   */
  virtual void createDefaultIdentity( QString& /*fullName*/, QString& /*emailAddress*/ ) {}

protected slots:
  void slotRollback() { rollback(); };

protected:
  /** The list that will be seen by everyone */
  QList<Identity> mIdentities;
  /** The list that will be seen by the config dialog */
  QList<Identity> mShadowIdentities;

signals:
  void identitiesChanged( const QString &id );

private slots:
  // Connected to the DBus signal
  void slotIdentitiesChanged( const QString &id );

private:
  void writeConfig() const;
  void readConfig(KConfigBase* config);
  QStringList groupList(KConfigBase* config) const;
  void createDefaultIdentity();

  // returns a new Unique Object Identifier
  int newUoid();

private:
  KConfig* mConfig;
  bool mReadOnly;
  OrgKdePimIdentityManagerInterface *mIface;
};

} // namespace

#endif // _KMAIL_IDENTITYMANAGER_H_
