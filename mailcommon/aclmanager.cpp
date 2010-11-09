
#include "aclmanager.h"

#include "imapaclattribute.h"
#include "imapsettings.h"
#include "mailutil.h"

#include <akonadi/collection.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <QtCore/QAbstractListModel>
#include <QtGui/QAction>
#include <QtGui/QItemSelectionModel>

using namespace MailCommon;

// The set of standard permission sets
static const struct {
  KIMAP::Acl::Rights permissions;
  const char* userString;
} standardPermissions[] = {
  { KIMAP::Acl::None, I18N_NOOP2( "Permissions", "None" ) },
  { KIMAP::Acl::Lookup | KIMAP::Acl::Read | KIMAP::Acl::KeepSeen, I18N_NOOP2( "Permissions", "Read" ) },
  { KIMAP::Acl::Lookup | KIMAP::Acl::Read | KIMAP::Acl::KeepSeen | KIMAP::Acl::Insert | KIMAP::Acl::Post, I18N_NOOP2( "Permissions", "Append" ) },
  { KIMAP::Acl::Lookup | KIMAP::Acl::Read | KIMAP::Acl::KeepSeen | KIMAP::Acl::Insert | KIMAP::Acl::Post | KIMAP::Acl::Write | KIMAP::Acl::CreateMailbox | KIMAP::Acl::DeleteMailbox | KIMAP::Acl::DeleteMessage | KIMAP::Acl::Expunge, I18N_NOOP2( "Permissions", "Write" ) },
  { KIMAP::Acl::Lookup | KIMAP::Acl::Read | KIMAP::Acl::KeepSeen | KIMAP::Acl::Insert | KIMAP::Acl::Post | KIMAP::Acl::Write | KIMAP::Acl::CreateMailbox | KIMAP::Acl::DeleteMailbox | KIMAP::Acl::DeleteMessage | KIMAP::Acl::Expunge | KIMAP::Acl::Admin, I18N_NOOP2( "Permissions", "All" ) }
};

// internalRightsList is only used if permissions doesn't match the standard set
static QString permissionsToUserString( KIMAP::Acl::Rights permissions, const QString& internalRightsList )
{
  for ( uint i = 0; i < sizeof( standardPermissions ) / sizeof( *standardPermissions ); ++i ) {
    if ( KIMAP::Acl::normalizedRights( permissions ) == standardPermissions[i].permissions )
      return i18nc( "Permissions", standardPermissions[ i ].userString );
  }

  if ( internalRightsList.isEmpty() )
    return i18n( "Custom Permissions" ); // not very helpful, but should not happen
  else
    return i18n( "Custom Permissions (%1)", internalRightsList );
}

////////////// AclEditor //////////////////////

class AclEditor::Private
{
  public:
    Private()
      : mPermissions( AclEditor::ReadPermissions ),
        mShallBeSaved( false )
    {
    }

    QString mUserId;
    AclEditor::Permissions mPermissions;
    bool mShallBeSaved;
};

AclEditor::AclEditor( QObject *parent )
  : QObject( parent ), d( new Private )
{
}

AclEditor::~AclEditor()
{
  delete d;
}

void AclEditor::setUserId( const QString &id )
{
  d->mUserId = id;
  emit userIdChanged( d->mUserId );
}

QString AclEditor::userId() const
{
  return d->mUserId;
}

void AclEditor::setPermissions( Permissions permissions )
{
  d->mPermissions = permissions;
  emit permissionsChanged( d->mPermissions );
}

AclEditor::Permissions AclEditor::permissions() const
{
  return d->mPermissions;
}

void AclEditor::save()
{
  d->mShallBeSaved = true;
}

void AclEditor::cancel()
{
  d->mShallBeSaved = false;
}

////////////// AclManager //////////////////////

class AclModel : public QAbstractListModel
{
  public:
    enum Role {
      UserIdRole = Qt::UserRole + 1,
      PermissionsRole,
      PermissionsTextRole
    };

    AclModel( QObject *parent = 0 )
      : QAbstractListModel( parent )
    {
    }

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const
    {
      if ( index.row() < 0 || index.row() >= mRights.count() )
        return QVariant();

      const QPair<QByteArray, KIMAP::Acl::Rights> right = mRights.at( index.row() );
      switch ( role ) {
        case Qt::DisplayRole:
          return QString::fromLatin1( "%1: %2" ).arg( QString::fromLatin1( right.first ) )
                                                .arg( permissionsToUserString( right.second, QString() ) );
          break;
        case UserIdRole:
          return QString::fromLatin1( right.first );
          break;
        case PermissionsRole:
          return QVariant( static_cast<int>( right.second ) );
          break;
        case PermissionsTextRole:
          return permissionsToUserString( right.second, QString() );
          break;
        default:
          return QVariant();
          break;
      }

      return QVariant();
    }

    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole )
    {
      if ( index.row() < 0 || index.row() >= mRights.count() )
        return false;

      QPair<QByteArray, KIMAP::Acl::Rights> &right = mRights[ index.row() ];
      switch ( role ) {
        case UserIdRole:
          right.first = value.toByteArray();
          return true;
          break;
        case PermissionsRole:
          right.second = static_cast<KIMAP::Acl::Rights>( value.toInt() );
          return true;
          break;
        default:
          return false;
          break;
      }

      return false;
    }

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const
    {
      if ( parent.isValid() )
        return 0;
      else
        return mRights.count();
    }

    void setRights( const QMap<QByteArray, KIMAP::Acl::Rights> &rights )
    {
      mRights.clear();

      QMapIterator<QByteArray, KIMAP::Acl::Rights> it( rights );
      while ( it.hasNext() ) {
        it.next();
        mRights.append( qMakePair( it.key(), it.value() ) );
      }

      reset();
    }

    QMap<QByteArray, KIMAP::Acl::Rights> rights() const
    {
      QMap<QByteArray, KIMAP::Acl::Rights> result;

      typedef QPair<QByteArray, KIMAP::Acl::Rights> RightPair;
      foreach ( const RightPair &right, mRights )
        result.insert( right.first, right.second );

      return result;
    }

  protected:
    virtual bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() )
    {
      beginInsertRows( parent, row, row + count );
      for ( int i = 0; i < count; ++i )
        mRights.insert( row, qMakePair( QByteArray(), KIMAP::Acl::Rights() ) );
      endInsertRows();

      return true;
    }

    virtual bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() )
    {
      beginRemoveRows( parent, row, row + count );
      for ( int i = 0; i < count; ++i )
        mRights.remove( row, count );
      endRemoveRows();

      return true;
    }

  private:
    QVector<QPair<QByteArray, KIMAP::Acl::Rights> > mRights;
};

class MailCommon::AclManager::Private
{
  public:
    Private( AclManager *qq )
      : q( qq )
    {
      mAddAction = new QAction( i18n( "Add Entry..." ), q );
      q->connect( mAddAction, SIGNAL( triggered( bool ) ),
                  q, SLOT( addAcl() ) );

      mEditAction = new QAction( i18n( "Edit Entry..." ), q );
      mEditAction->setEnabled( false );
      q->connect( mEditAction, SIGNAL( triggered( bool ) ),
                  q, SLOT( editAcl() ) );

      mDeleteAction = new QAction( i18n( "Remove Entry" ), q );
      mDeleteAction->setEnabled( false );
      q->connect( mDeleteAction, SIGNAL( triggered( bool ) ),
                  q, SLOT( deleteAcl() ) );

      mModel = new AclModel( q );

      mSelectionModel = new QItemSelectionModel( mModel );
      q->connect( mSelectionModel, SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
                  q, SLOT( selectionChanged( const QItemSelection&, const QItemSelection& ) ) );
    }

    ~Private()
    {
    }

    void selectionChanged( const QItemSelection &selection, const QItemSelection& )
    {
      const bool itemSelected = !selection.indexes().isEmpty();

      bool canAdmin = (mUserRights & KIMAP::Acl::Admin);

      bool canAdminThisItem = canAdmin;
      if ( canAdmin && itemSelected ) {
        const QModelIndex index = selection.indexes().first();
        const QString userId = index.data( AclModel::UserIdRole ).toString();
        const KIMAP::Acl::Rights rights = static_cast<KIMAP::Acl::Rights>( index.data( AclModel::PermissionsRole ).toInt() );

        // Don't allow users to remove their own admin permissions - there's no way back
        if ( mImapUserName == userId && (rights & KIMAP::Acl::Admin) )
          canAdminThisItem = false;
      }

      mAddAction->setEnabled( canAdmin );
      mEditAction->setEnabled( itemSelected && canAdminThisItem );
      mDeleteAction->setEnabled( itemSelected && canAdminThisItem );
    }

    void addAcl()
    {
      AclEditor *editor = new AclEditor( q );

      emit q->addAcl( editor );

      if ( editor->d->mShallBeSaved ) {
        if ( mModel->insertRow( mModel->rowCount() - 1 ) ) {
          const QModelIndex index = mModel->index( mModel->rowCount() - 1, 0 );
          mModel->setData( index, editor->userId(), AclModel::UserIdRole );
          if ( editor->permissions() != AclEditor::CustomPermissions )
            mModel->setData( index, static_cast<int>( standardPermissions[ editor->permissions() ].permissions ), AclModel::PermissionsRole );
        }
      }

      delete editor;
    }

    void editAcl()
    {
      const QModelIndex index = mSelectionModel->selectedIndexes().first();
      const QString userId = index.data( AclModel::UserIdRole ).toString();
      const KIMAP::Acl::Rights rights = static_cast<KIMAP::Acl::Rights>( index.data( AclModel::PermissionsRole ).toInt() );

      AclEditor::Permissions permissions = AclEditor::CustomPermissions;
      for ( int i = 0; i < 5 /*FIXME*/; ++i ) {
        if ( standardPermissions[ i ].permissions == rights ) {
          permissions = static_cast<AclEditor::Permissions>( i );
          break;
        }
      }

      AclEditor *editor = new AclEditor( q );
      editor->setUserId( userId );
      editor->setPermissions( permissions );

      emit q->editAcl( editor );

      if ( editor->d->mShallBeSaved ) {
        mModel->setData( index, editor->userId(), AclModel::UserIdRole );
        if ( editor->permissions() != AclEditor::CustomPermissions )
          mModel->setData( index, static_cast<int>( standardPermissions[ editor->permissions() ].permissions ), AclModel::PermissionsRole );
      }

      delete editor;
    }

    void deleteAcl()
    {
      const QModelIndex index = mSelectionModel->selectedIndexes().first();
      const QString userId = index.data( AclModel::UserIdRole ).toString();

      if ( mImapUserName == userId ) {
        if ( KMessageBox::Cancel == KMessageBox::warningContinueCancel( 0,
           i18n( "Do you really want to remove your own permissions for this folder? You will not be able to access it afterwards." ), i18n( "Remove" ) ) )
          return;
      }

      mModel->removeRow( index.row(), QModelIndex() );
    }

    void setCollection( const Akonadi::Collection &collection )
    {
      const MailCommon::ImapAclAttribute *attribute = collection.attribute<MailCommon::ImapAclAttribute>();
      const QMap<QByteArray, KIMAP::Acl::Rights> rights = attribute->rights();

      OrgKdeAkonadiImapSettingsInterface *imapSettingsInterface = MailCommon::Util::createImapSettingsInterface( collection.resource() );
      
      if ( imapSettingsInterface->isValid() ) {
        const QDBusReply<QString> reply = imapSettingsInterface->userName();
        if ( reply.isValid() ) {
          mImapUserName = reply;
        }
      }

      delete imapSettingsInterface;

      mUserRights = rights[ mImapUserName.toUtf8() ];

      mModel->setRights( rights );
    }

    AclManager *q;
    AclModel *mModel;
    QItemSelectionModel *mSelectionModel;
    QAction *mAddAction;
    QAction *mEditAction;
    QAction *mDeleteAction;

    Akonadi::Collection mCollection;
    QString mImapUserName;
    KIMAP::Acl::Rights mUserRights;
};

AclManager::AclManager( QObject *parent )
  : QObject( parent ), d( new Private( this ) )
{
}

AclManager::~AclManager()
{
  delete d;
}

void AclManager::setCollection( const Akonadi::Collection &collection )
{
  d->setCollection( collection );
  emit collectionChanged( d->mCollection );
}

Akonadi::Collection AclManager::collection() const
{
  return d->mCollection;
}

QAbstractItemModel* AclManager::model() const
{
  return d->mModel;
}

QItemSelectionModel* AclManager::selectionModel() const
{
  return d->mSelectionModel; 
}

QAction* AclManager::addAction() const
{
  return d->mAddAction;
}

QAction* AclManager::editAction() const
{
  return d->mEditAction;
}

QAction* AclManager::deleteAction() const
{
  return d->mDeleteAction;
}

void AclManager::save()
{
}

#include "aclmanager.moc"
