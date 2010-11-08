
#ifndef MAILCOMMON_ACLMANAGER_H
#define MAILCOMMON_ACLMANAGER_H

#include "mailcommon_export.h"

#include <akonadi/collection.h>

#include <QtCore/QObject>

class QAbstractItemModel;
class QAction;
class QItemSelectionModel;

namespace MailCommon {

class AclEditor : public QObject
{
  Q_OBJECT

  Q_PROPERTY( QString userId READ userId WRITE setUserId NOTIFY userIdChanged )
  Q_PROPERTY( Permissions permissions READ permissions WRITE setPermissions NOTIFY permissionsChanged )

  Q_ENUMS( Permissions )

  public:
    /**
     * Describes the possible permissions of an ACL.
     */
    enum Permissions {
      NonePermissions,
      ReadPermissions,
      AppendPermissions,
      WritePermissions,
      AllPermissions,
      CustomPermissions
    };

    /**
     * Creates a new ACL editor.
     */
    AclEditor( QObject *parent = 0 );

    /**
     * Destroys the ACL editor.
     */
    ~AclEditor();

    /**
     * Sets the user @p id of the ACL.
     */
    void setUserId( const QString &id );

    /**
     * Returns the user id of the ACL.
     */
    QString userId() const;

    /**
     * Sets the @p permissions of the ACL.
     */
    void setPermissions( Permissions permissions );

    /**
     * Returns the permissions of the ACL.
     */
    Permissions permissions() const;

  public Q_SLOTS:
    /**
     * Saves the ACL back.
     */
    void save();

    /**
     * Cancels the editing of the ACL.
     */
    void cancel();

  Q_SIGNALS:
    /**
     * This signal is emitted when the user @p id has been changed.
     */
    void userIdChanged( const QString &id );

    /**
     * This signal is emitted when the @p permissions have been changed.
     */
    void permissionsChanged( MailCommon::AclEditor::Permissions permissions );

  private:
    //@cond PRIVATE
    friend class AclManager;

    class Private;
    Private* const d;
    //@endcond
};

class MAILCOMMON_EXPORT AclManager : public QObject
{
  Q_OBJECT

  Q_PROPERTY( Akonadi::Collection collection READ collection WRITE setCollection NOTIFY collectionChanged )
  Q_PROPERTY( QAbstractItemModel* model READ model )
  Q_PROPERTY( QItemSelectionModel* selectionModel READ selectionModel )
  Q_PROPERTY( QAction* addAction READ addAction )
  Q_PROPERTY( QAction* editAction READ editAction )
  Q_PROPERTY( QAction* deleteAction READ deleteAction )

  public:
    /**
     * Creates a new ACL manager.
     *
     * @param parent The parent object.
     */
    AclManager( QObject *parent = 0 );

    /**
     * Destroys the ACL manager.
     */
    ~AclManager();

    /**
     * Sets the @p collection whose ACL will be managed.
     */
    void setCollection( const Akonadi::Collection &collection );

    /**
     * Sets the @p collection whose ACL are managed.
     */
    Akonadi::Collection collection() const;

    /**
     * Returns the model that represents the ACL of the managed collection.
     */
    QAbstractItemModel *model() const;

    /**
     * Returns the selection model that is used by the manager to select the
     * ACL entry to work on.
     */
    QItemSelectionModel *selectionModel() const;

    /**
     * Returns the action that handles adding new ACL entries.
     */
    QAction *addAction() const;

    /**
     * Returns the action that handles editing the currently selected ACL entry.
     */
    QAction *editAction() const;

    /**
     * Returns the action that handles deleting the currently selected ACL entry.
     */
    QAction *deleteAction() const;

  public Q_SLOTS:
    /**
     * Saves the changes of the ACL back to the collection.
     */
    void save();

  Q_SIGNALS:
    /**
     * This signal is emitted whenever the collection whose ACL will
     * be managed has changed.
     */
    void collectionChanged( const Akonadi::Collection &collection );

    void addAcl( AclEditor *editor );
    void editAcl( AclEditor *editor );

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void selectionChanged( const QItemSelection&, const QItemSelection& ) )
    Q_PRIVATE_SLOT( d, void addAcl() )
    Q_PRIVATE_SLOT( d, void editAcl() )
    Q_PRIVATE_SLOT( d, void deleteAcl() )
    //@endcond
};

}

#endif
