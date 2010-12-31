/*
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef MAILCOMMON_KERNEL_H
#define MAILCOMMON_KERNEL_H

#include "mailinterfaces.h"
#include "mailcommon_export.h"

#include <QObject>

#include <ksharedconfig.h>

#include "akonadi/collection.h"
#include "akonadi/kmime/specialmailcollections.h"



namespace MailCommon {


/** Deals with common mail application related operations. The required interfaces
 *  MUST be registered before using it!
 *  Be careful when using in multi-threaded applications, as Kernel is a QObject
 *  singleton, created in the main thread, thus event handling for Kernel::self()
 *  will happen in the main thread.
 */

class MAILCOMMON_EXPORT Kernel : public QObject {
  Q_OBJECT  
public:

  virtual ~Kernel();
  
  static Kernel *self();

  /** Register the interface dealing with main mail functionality. This function
   * MUST be called with a valid interface pointer, before any Kernel::self()
   * method is used. The pointer ownership will not be transfered to Kernel. */
  void registerKernelIf( IKernel* kernelIf ) {
    mKernelIf = kernelIf;
  }

  IKernel *kernelIf() const {
    Q_ASSERT( mKernelIf );
    return mKernelIf;
  }

  /** Register the interface dealing with mail settings. This function
   * MUST be called with a valid interface pointer, before any Kernel::self()
   * method is used. The pointer ownership will not be transfered to Kernel. */
  void registerSettingsIf( ISettings* settingsIf ) {
    mSettingsIf = settingsIf;
  }

  ISettings *settingsIf() const {
    Q_ASSERT( mSettingsIf );
    return mSettingsIf;
  }

  /** Register the interface dealing with mail settings. This function
   * MUST be called with a valid interface pointer, before any Kernel::self()
   * method is used. The pointer ownership will not be transfered to Kernel. */
  void registerFilterIf( IFilter* filterIf ) {
    mFilterIf = filterIf;
  }

  IFilter *filterIf() const {
    Q_ASSERT( mFilterIf );
    return mFilterIf;
  }

  /**
  * Returns the collection associated with the given @p id, or an invalid collection if not found.
  * The EntityTreeModel of the kernel is searched for the collection. Since the ETM is loaded
  * async, this method will not find the collection right after startup, when the ETM is not yet
  * fully loaded.
  */
  Akonadi::Collection collectionFromId( const Akonadi::Collection::Id& id ) const;

  /**
  * Converts @p idString into a number and returns the collection for it.
  * @see collectionFromId( qint64 )
  */
  Akonadi::Collection collectionFromId( const QString &idString ) const;

  Akonadi::Collection inboxCollectionFolder();
  Akonadi::Collection outboxCollectionFolder();
  Akonadi::Collection sentCollectionFolder();
  Akonadi::Collection trashCollectionFolder();
  Akonadi::Collection draftsCollectionFolder();
  Akonadi::Collection templatesCollectionFolder();

  bool isSystemFolderCollection( const Akonadi::Collection &col );

  /** Returns true if this folder is the inbox on the local disk */
  bool isMainFolderCollection( const Akonadi::Collection &col );

  bool folderIsDraftOrOutbox(const Akonadi::Collection &collection);
  bool folderIsDrafts(const Akonadi::Collection&);

  bool folderIsTemplates( const Akonadi::Collection &collection );
  bool folderIsTrash( const Akonadi::Collection &collection );

  Akonadi::Collection trashCollectionFromResource( const Akonadi::Collection & col );

  /**
   * Returns true if the folder is one of the sent-mail folders.
   */
  bool folderIsSentMailFolder( const Akonadi::Collection& );
  
  bool folderIsInbox( const Akonadi::Collection& );

  void initFolders();

  void emergencyExit( const QString& reason );

private:  
  void findCreateDefaultCollection( Akonadi::SpecialMailCollections::Type );

private Q_SLOTS:  
  void createDefaultCollectionDone( KJob * job);
  void slotDefaultCollectionsChanged();

Q_SIGNALS:
  void requestConfigSync();
  void requestSystemTrayUpdate();

private:
  Kernel( QObject* parent = 0 );
  friend class KernelPrivate;
  
  IKernel* mKernelIf;
  IFilter* mFilterIf;
  ISettings* mSettingsIf;
};

}

#define KernelIf MailCommon::Kernel::self()->kernelIf()
#define FilterIf MailCommon::Kernel::self()->filterIf()
#define SettingsIf MailCommon::Kernel::self()->settingsIf()
#define CommonKernel MailCommon::Kernel::self()

#endif
