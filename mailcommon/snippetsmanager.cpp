/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#include "snippetsmanager.h"

#include "snippetdialog_p.h"
#include "snippetsmodel_p.h"
#include "snippetvariabledialog_p.h"

#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <QtCore/QDebug>
#include <QtGui/QAction>
#include <QtGui/QItemSelectionModel>

using namespace MailCommon;

class SnippetsManager::Private
{
  public:
    Private( SnippetsManager *qq )
      : q( qq ), mEditor( 0 )
    {
    }

    QModelIndex currentGroupIndex() const;

    void selectionChanged();

    void addSnippet();
    void editSnippet();
    void deleteSnippet();

    void addSnippetGroup();
    void editSnippetGroup();
    void deleteSnippetGroup();

    void insertSelectedSnippet();
    void insertActionSnippet();

    void updateActionCollection( const QString &oldName, const QString &newName,
                                 const QKeySequence &keySequence, const QString &text );

    QString replaceVariables( const QString &text );

    void load();
    void save();

    SnippetsManager *q;
    SnippetsModel *mModel;
    QItemSelectionModel *mSelectionModel;
    KActionCollection *mActionCollection;
    QObject *mEditor;
    QByteArray mEditorInsertMethod;
    QMap<QString, QString> mSavedVariables;

    QAction *mAddSnippetAction;
    QAction *mEditSnippetAction;
    QAction *mDeleteSnippetAction;
    QAction *mAddSnippetGroupAction;
    QAction *mEditSnippetGroupAction;
    QAction *mDeleteSnippetGroupAction;
    QAction *mInsertSnippetAction;
};

QModelIndex SnippetsManager::Private::currentGroupIndex() const
{
  if ( mSelectionModel->selectedIndexes().isEmpty() )
    return QModelIndex();

  const QModelIndex index = mSelectionModel->selectedIndexes().first();
  if ( index.data( SnippetsModel::IsGroupRole ).toBool() )
    return index;
  else
    return mModel->parent( index );
}

void SnippetsManager::Private::selectionChanged()
{
  const bool itemSelected = !mSelectionModel->selectedIndexes().isEmpty();

  if ( itemSelected ) {
    const QModelIndex index = mSelectionModel->selectedIndexes().first();
    const bool isGroup = index.data( SnippetsModel::IsGroupRole ).toBool();
    if ( isGroup ) {
      mEditSnippetAction->setEnabled( false );
      mDeleteSnippetAction->setEnabled( false );
      mEditSnippetGroupAction->setEnabled( true );
      mDeleteSnippetGroupAction->setEnabled( true );
      mInsertSnippetAction->setEnabled( false );
    } else {
      mEditSnippetAction->setEnabled( true );
      mDeleteSnippetAction->setEnabled( true );
      mEditSnippetGroupAction->setEnabled( false );
      mDeleteSnippetGroupAction->setEnabled( false );
      mInsertSnippetAction->setEnabled( true );
    }
  } else {
    mEditSnippetAction->setEnabled( false );
    mDeleteSnippetAction->setEnabled( false );
    mEditSnippetGroupAction->setEnabled( false );
    mDeleteSnippetGroupAction->setEnabled( false );
    mInsertSnippetAction->setEnabled( false );
  }
}

void SnippetsManager::Private::addSnippet()
{
  const bool noGroupAvailable = (mModel->rowCount() == 0);

  if ( noGroupAvailable ) {
    // create a 'General' snippet group
    if ( !mModel->insertRow( mModel->rowCount(), QModelIndex() ) )
      return;

    const QModelIndex groupIndex = mModel->index( mModel->rowCount() - 1, 0, QModelIndex() );
    mModel->setData( groupIndex, i18n( "General" ), SnippetsModel::NameRole );

    mSelectionModel->select( groupIndex, QItemSelectionModel::ClearAndSelect );
  }

  SnippetDialog dlg( mActionCollection, false );
  dlg.setWindowTitle( i18nc( "@title:window", "Add Snippet" ) );
  dlg.setGroupModel( mModel );
  dlg.setGroupIndex( currentGroupIndex() );

  if ( !dlg.exec() )
    return;

  const QModelIndex groupIndex = dlg.groupIndex();

  if ( !mModel->insertRow( mModel->rowCount( groupIndex ), groupIndex ) )
    return;

  const QModelIndex index = mModel->index( mModel->rowCount( groupIndex ) - 1, 0, groupIndex );
  mModel->setData( index, dlg.name(), SnippetsModel::NameRole );
  mModel->setData( index, dlg.text(), SnippetsModel::TextRole );
  mModel->setData( index, dlg.keySequence().toString(), SnippetsModel::KeySequenceRole );

  updateActionCollection( QString(), dlg.name(), dlg.keySequence(), dlg.text() );
}

void SnippetsManager::Private::editSnippet()
{
  QModelIndex index = mSelectionModel->selectedIndexes().first();
  if ( !index.isValid() || index.data( SnippetsModel::IsGroupRole ).toBool() )
    return;

  const QModelIndex oldGroupIndex = currentGroupIndex();

  const QString oldSnippetName = index.data( SnippetsModel::NameRole ).toString();

  SnippetDialog dlg( mActionCollection, false );
  dlg.setWindowTitle( i18nc( "@title:window", "Edit Snippet" ) );
  dlg.setGroupModel( mModel );
  dlg.setGroupIndex( oldGroupIndex );
  dlg.setName( oldSnippetName );
  dlg.setText( index.data( SnippetsModel::TextRole ).toString() );
  dlg.setKeySequence( QKeySequence::fromString( index.data( SnippetsModel::KeySequenceRole ).toString() ) );

  if ( !dlg.exec() )
    return;

  const QModelIndex newGroupIndex = dlg.groupIndex();

  if ( oldGroupIndex != newGroupIndex ) {
    mModel->removeRow( index.row(), oldGroupIndex );
    mModel->insertRow( mModel->rowCount( newGroupIndex ), newGroupIndex );

    index = mModel->index( mModel->rowCount( newGroupIndex ) - 1, 0, newGroupIndex );
  }

  mModel->setData( index, dlg.name(), SnippetsModel::NameRole );
  mModel->setData( index, dlg.text(), SnippetsModel::TextRole );
  mModel->setData( index, dlg.keySequence().toString(), SnippetsModel::KeySequenceRole );

  updateActionCollection( oldSnippetName, dlg.name(), dlg.keySequence(), dlg.text() );
}

void SnippetsManager::Private::deleteSnippet()
{
  const QModelIndex index = mSelectionModel->selectedIndexes().first();

  const QString snippetName = index.data( SnippetsModel::NameRole ).toString();

  if ( KMessageBox::warningContinueCancel( 0, i18nc( "@info",
                  "Do you really want to remove snippet \"%1\"?<nl/>"
                  "<warning>There is no way to undo the removal.</warning>", snippetName ),
                  QString(),
                  KStandardGuiItem::remove() ) == KMessageBox::Cancel ) {
    return;
  }

  mModel->removeRow( index.row(), currentGroupIndex() );

  updateActionCollection( snippetName, QString(), QKeySequence(), QString() );
}

void SnippetsManager::Private::addSnippetGroup()
{
  SnippetDialog dlg( mActionCollection, true );
  dlg.setWindowTitle( i18nc( "@title:window", "Add Group" ) );

  if ( !dlg.exec() )
    return;

  if ( !mModel->insertRow( mModel->rowCount(), QModelIndex() ) ) {
    qDebug() << "unable to insert row";
    return;
  }

  const QModelIndex groupIndex = mModel->index( mModel->rowCount() - 1, 0, QModelIndex() );
  mModel->setData( groupIndex, dlg.name(), SnippetsModel::NameRole );
}

void SnippetsManager::Private::editSnippetGroup()
{
  const QModelIndex groupIndex = currentGroupIndex();
  if ( !groupIndex.isValid() || !groupIndex.data( SnippetsModel::IsGroupRole ).toBool() )
    return;

  SnippetDialog dlg( mActionCollection, true );
  dlg.setWindowTitle( i18nc( "@title:window", "Edit Group" ) );
  dlg.setName( groupIndex.data( SnippetsModel::NameRole ).toString() );

  if ( !dlg.exec() )
    return;

  mModel->setData( groupIndex, dlg.name(), SnippetsModel::NameRole );
}

void SnippetsManager::Private::deleteSnippetGroup()
{
  const QModelIndex groupIndex = currentGroupIndex();
  if ( !groupIndex.isValid() )
    return;

  const QString groupName = groupIndex.data( SnippetsModel::NameRole ).toString();

  if ( mModel->rowCount( groupIndex ) > 0 ) {
    if ( KMessageBox::warningContinueCancel( 0, i18nc( "@info",
                    "Do you really want to remove group \"%1\" along with all its snippets?<nl/>"
                    "<warning>There is no way to undo the removal.</warning>", groupName ),
                    QString(),
                    KStandardGuiItem::remove() ) == KMessageBox::Cancel ) {
      return;
    }
  } else {
    if ( KMessageBox::warningContinueCancel( 0, i18nc( "@info",
                    "Do you really want to remove group \"%1\"?", groupName ),
                    QString(),
                    KStandardGuiItem::remove() ) == KMessageBox::Cancel ) {
      return;
    }
  }

  mModel->removeRow( groupIndex.row(), QModelIndex() );
}

void SnippetsManager::Private::insertSelectedSnippet()
{
  if ( !mEditor )
    return;

  if ( !mSelectionModel->hasSelection() )
    return;

  const QModelIndex index = mSelectionModel->selectedIndexes().first();
  if ( index.data( SnippetsModel::IsGroupRole ).toBool() )
    return;

  const QString text = replaceVariables( index.data( SnippetsModel::TextRole ).toString() );
  QMetaObject::invokeMethod( mEditor, mEditorInsertMethod, Qt::DirectConnection,
                             Q_ARG( QString, text ) );
}

void SnippetsManager::Private::insertActionSnippet()
{
  if ( !mEditor )
    return;

  QAction *action = qobject_cast<QAction*>( q->sender() );
  if ( !action )
    return;

  const QString text = replaceVariables( action->property( "snippetText" ).toString() );
  QMetaObject::invokeMethod( mEditor, mEditorInsertMethod, Qt::DirectConnection,
                             Q_ARG( QString, text ) );
}

void SnippetsManager::Private::updateActionCollection( const QString &oldName, const QString &newName,
                                                       const QKeySequence &keySequence, const QString &text )
{
  // remove previous action in case that the name changed
  if ( !oldName.isEmpty() ) {
    const QString actionName = i18nc( "@action", "Snippet %1", oldName );
    const QString normalizedName = QString( actionName ).replace( ' ', '_' );

    QAction *action = mActionCollection->action( normalizedName );
    if ( action )
      mActionCollection->removeAction( action );
  }

  if ( !newName.isEmpty() ) {
    const QString actionName = i18nc( "@action", "Snippet %1", newName );
    const QString normalizedName = QString( actionName ).replace( ' ', '_' );

    KAction *action = mActionCollection->addAction( normalizedName, q, SLOT(insertActionSnippet()) );
    action->setProperty( "snippetText", text );
    action->setText( actionName );
    action->setShortcut( keySequence );
  }
}

QString SnippetsManager::Private::replaceVariables( const QString &text )
{
  QString result = text;
  QString variableName;
  QString variableValue;
  QMap<QString, QString> localVariables( mSavedVariables );
  int iFound = -1;
  int iEnd = -1;

  do {
    iFound = text.indexOf( QRegExp( "\\$[A-Za-z-_0-9\\s]*\\$" ), iEnd + 1 );  //find the next variable by this QRegExp
    if ( iFound >= 0 ) {
      iEnd = text.indexOf( '$', iFound + 1 ) + 1;

      variableName = text.mid( iFound, iEnd - iFound );

      if ( variableName != QLatin1String( "$$" ) ) {  // if not double-delimiter
        if ( !localVariables.contains( variableName ) ) {  // and not already in map

          SnippetVariableDialog dlg( variableName, &mSavedVariables );
          if ( !dlg.exec() )
            return QString();

          variableValue = dlg.variableValue();
        } else {
          variableValue = localVariables.value( variableName );
        }
      } else {
        variableValue = '$'; //if double-delimiter -> replace by single character
      }

      result.replace( variableName, variableValue );
      localVariables[ variableName ] = variableValue;
    }
  } while ( iFound != -1 );

  return result;
}

void SnippetsManager::Private::load()
{
  const KSharedConfig::Ptr config = KSharedConfig::openConfig( "kmailsnippetrc", KConfig::NoGlobals );
  const KConfigGroup group = config->group( "SnippetPart" );

  const int groupCount = group.readEntry( "snippetGroupCount", 0 );

  for ( int i = 0; i < groupCount; ++i ) {
    const KConfigGroup group = config->group( QString::fromLatin1( "SnippetGroup_%1" ).arg ( i ) );
    const QString groupName = group.readEntry( "Name" );

    // create group
    mModel->insertRow( mModel->rowCount(), QModelIndex() );
    const QModelIndex groupIndex = mModel->index( mModel->rowCount() - 1, 0, QModelIndex() );

    mModel->setData( groupIndex, groupName, SnippetsModel::NameRole );

    const int snippetCount = group.readEntry( "snippetCount", 0 );
    for ( int j = 0; j < snippetCount; ++j ) {
      const QString snippetName = group.readEntry( QString::fromLatin1( "snippetName_%1" ).arg( j ), QString() );
      const QString snippetText = group.readEntry( QString::fromLatin1( "snippetText_%1" ).arg( j ), QString() );
      const QString snippetKeySequence = group.readEntry( QString::fromLatin1( "snippetKeySequence_%1" ).arg( j ), QString() );

      // create snippet
      mModel->insertRow( mModel->rowCount( groupIndex ), groupIndex );
      const QModelIndex index = mModel->index( mModel->rowCount( groupIndex ) - 1, 0, groupIndex );

      mModel->setData( index, snippetName, SnippetsModel::NameRole );
      mModel->setData( index, snippetText, SnippetsModel::TextRole );
      mModel->setData( index, snippetKeySequence, SnippetsModel::KeySequenceRole );

      updateActionCollection( QString(), snippetName, QKeySequence::fromString( snippetKeySequence ), snippetText );
    }
  }

  {
    mSavedVariables.clear();
    const KConfigGroup group = config->group( "SavedVariablesPart" );
    const int variablesCount = group.readEntry( "variablesCount", 0 );

    for ( int i = 0; i < variablesCount; ++i ) {
      const QString variableKey = group.readEntry( QString::fromLatin1( "variableName_%1" ).arg( i ), QString() );
      const QString variableValue = group.readEntry( QString::fromLatin1( "variableValue_%1" ).arg( i ), QString() );

      mSavedVariables.insert( variableKey, variableValue );
    }
  }
}

void SnippetsManager::Private::save()
{
  KSharedConfig::Ptr config = KSharedConfig::openConfig( "kmailsnippetrc", KConfig::NoGlobals );

  // clear everything
  foreach ( const QString &group, config->groupList() )
    config->deleteGroup( group );

  // write number of snippet groups
  KConfigGroup group = config->group( "SnippetPart" );

  const int groupCount = mModel->rowCount();
  group.writeEntry( "snippetGroupCount", groupCount );

  for ( int i = 0; i < groupCount; ++i ) {
    const QModelIndex groupIndex = mModel->index( i, 0, QModelIndex() );
    const QString groupName = groupIndex.data( SnippetsModel::NameRole ).toString();

    KConfigGroup group = config->group( QString::fromLatin1( "SnippetGroup_%1" ).arg ( i ) );
    group.writeEntry( "Name", groupName );

    const int snippetCount = mModel->rowCount( groupIndex );

    group.writeEntry( "snippetCount", snippetCount );
    for ( int j = 0; j < snippetCount; ++j ) {
      const QModelIndex index = mModel->index( j, 0, groupIndex );

      const QString snippetName = index.data( SnippetsModel::NameRole ).toString();
      const QString snippetText = index.data( SnippetsModel::TextRole ).toString();
      const QString snippetKeySequence = index.data( SnippetsModel::KeySequenceRole ).toString();

      group.writeEntry( QString::fromLatin1( "snippetName_%1" ).arg( j ), snippetName );
      group.writeEntry( QString::fromLatin1( "snippetText_%1" ).arg( j ), snippetText );
      group.writeEntry( QString::fromLatin1( "snippetKeySequence_%1" ).arg( j ), snippetKeySequence );
    }
  }

  {
    KConfigGroup group = config->group( "SavedVariablesPart" );

    const int variablesCount = mSavedVariables.count();
    group.writeEntry( "variablesCount", variablesCount );

    int counter = 0;
    QMapIterator<QString, QString> it( mSavedVariables );
    while ( it.hasNext() ) {
      it.next();
      group.writeEntry( QString::fromLatin1( "variableName_%1" ).arg( counter ), it.key() );
      group.writeEntry( QString::fromLatin1( "variableValue_%1" ).arg( counter ), it.value() );
      counter++;
    }
  }

  config->sync();
}


SnippetsManager::SnippetsManager( KActionCollection *actionCollection, QObject *parent )
  : QObject( parent ), d( new Private( this ) )
{
  d->mModel = new SnippetsModel( this );
  d->mSelectionModel = new QItemSelectionModel( d->mModel );
  d->mActionCollection = actionCollection;

  d->mAddSnippetAction = new QAction( i18n( "Add Snippet..." ), this );
  d->mEditSnippetAction = new QAction( i18n( "Edit Snippet..." ), this );
  d->mEditSnippetAction->setIcon( KIcon( "document-properties" ) );
  d->mDeleteSnippetAction = new QAction( i18n( "Remove Snippet" ), this );
  d->mDeleteSnippetAction->setIcon( KIcon( "edit-delete" ) );

  d->mAddSnippetGroupAction = new QAction( i18n( "Add Group..." ), this );
  d->mEditSnippetGroupAction = new QAction( i18n( "Rename Group..." ), this );
  d->mEditSnippetGroupAction->setIcon( KIcon( "edit-rename" ) );
  d->mDeleteSnippetGroupAction = new QAction( i18n( "Remove Group" ), this );
  d->mDeleteSnippetGroupAction->setIcon( KIcon( "edit-delete" ) );

  d->mInsertSnippetAction = new QAction( i18n( "Insert Snippet" ), this );

  connect( d->mSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
           this, SLOT(selectionChanged()) );

  connect( d->mAddSnippetAction, SIGNAL(triggered(bool)), SLOT(addSnippet()) );
  connect( d->mEditSnippetAction, SIGNAL(triggered(bool)), SLOT(editSnippet()) );
  connect( d->mDeleteSnippetAction, SIGNAL(triggered(bool)), SLOT(deleteSnippet()) );

  connect( d->mAddSnippetGroupAction, SIGNAL(triggered(bool)), SLOT(addSnippetGroup()) );
  connect( d->mEditSnippetGroupAction, SIGNAL(triggered(bool)), SLOT(editSnippetGroup()) );
  connect( d->mDeleteSnippetGroupAction, SIGNAL(triggered(bool)), SLOT(deleteSnippetGroup()) );

  connect( d->mInsertSnippetAction, SIGNAL(triggered(bool)), SLOT(insertSelectedSnippet()) );

  d->selectionChanged();

  d->load();
}

SnippetsManager::~SnippetsManager()
{
  d->save();

  delete d;
}

void SnippetsManager::setEditor( QObject *editor, const char *insertSnippetMethod, const char *dropSignal )
{
  d->mEditor = editor;
  d->mEditorInsertMethod = insertSnippetMethod;

  if ( dropSignal ) {
    const int index = editor->metaObject()->indexOfSignal( QMetaObject::normalizedSignature( dropSignal + 1 ).data() ); // skip the leading '2'
    if ( index != -1 )
      connect( editor, dropSignal, this, SLOT(insertSelectedSnippet()) );
  }
}

QAbstractItemModel *SnippetsManager::model() const
{
  return d->mModel;
}

QItemSelectionModel *SnippetsManager::selectionModel() const
{
  return d->mSelectionModel;
}

QAction *SnippetsManager::addSnippetAction() const
{
  return d->mAddSnippetAction;
}

QAction *SnippetsManager::editSnippetAction() const
{
  return d->mEditSnippetAction;
}

QAction *SnippetsManager::deleteSnippetAction() const
{
  return d->mDeleteSnippetAction;
}

QAction *SnippetsManager::addSnippetGroupAction() const
{
  return d->mAddSnippetGroupAction;
}

QAction *SnippetsManager::editSnippetGroupAction() const
{
  return d->mEditSnippetGroupAction;
}

QAction *SnippetsManager::deleteSnippetGroupAction() const
{
  return d->mDeleteSnippetGroupAction;
}

QAction *SnippetsManager::insertSnippetAction() const
{
  return d->mInsertSnippetAction;
}

bool SnippetsManager::snippetGroupSelected() const
{
  if ( d->mSelectionModel->selectedIndexes().isEmpty() )
    return false;

  return d->mSelectionModel->selectedIndexes().first().data( SnippetsModel::IsGroupRole ).toBool();
}

QString SnippetsManager::selectedName() const
{
  if ( d->mSelectionModel->selectedIndexes().isEmpty() )
    return QString();

  return d->mSelectionModel->selectedIndexes().first().data( SnippetsModel::NameRole ).toString();
}

#include "snippetsmanager.moc"
