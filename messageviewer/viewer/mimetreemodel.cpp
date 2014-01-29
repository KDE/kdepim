/*
    Copyright (c) 2007, 2008 Volker Krause <vkrause@kde.org>

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



#include "utils/util.h"
#include "mimetreemodel.h"

#include "nodehelper.h"

#include <kmime/kmime_content.h>
#include <KMime/Message>

#include <KDebug>
#include <KIcon>
#include <KLocalizedString>
#include <KMimeType>

Q_DECLARE_METATYPE(KMime::Content*)
Q_DECLARE_METATYPE(KMime::ContentIndex)

using namespace MessageViewer;

class MimeTreeModel::Private
{
  public:
    Private() :
      root ( 0 )
    {}

    // FIXME: this should actually be a member function of ContentIndex
    int contentIndexUp( KMime::ContentIndex &index )
    {
      Q_ASSERT( index.isValid() );
      QStringList ids = index.toString().split( QLatin1Char('.') );
      const QString lastId = ids.takeLast();
      index = KMime::ContentIndex( ids.join( QLatin1String(".") ) );
      return lastId.toInt();
    }

    QString descriptionForContent( KMime::Content *content )
    {
      KMime::Message * const message = dynamic_cast<KMime::Message*>( content );
      if ( message && message->subject( false ) )
        return message->subject()->asUnicodeString();
      const QString name = NodeHelper::fileName( content );
      if ( !name.isEmpty() )
        return name;
      if ( content->contentDescription( false ) ) {
        const QString desc = content->contentDescription()->asUnicodeString();
        if ( !desc.isEmpty() )
          return desc;
      }
      return i18n( "body part" );
    }

    QString mimeTypeForContent( KMime::Content *content )
    {
      if ( content->contentType( false ) )
        return QString::fromLatin1( content->contentType()->mimeType() );
      return QString();
    }

    QString typeForContent( KMime::Content *content )
    {
      if ( content->contentType( false ) )
      {
        const QString contentMimeType = QString::fromLatin1(content->contentType()->mimeType());
        KMimeType::Ptr mimeType = KMimeType::mimeType( contentMimeType );
        if ( mimeType.isNull() )
          return contentMimeType;
        return mimeType->comment();
      }
      else
        return QString();
    }

    QString sizeOfContent( KMime::Content *content )
    {
      if ( content->body().isEmpty() )
        return QString();
      return KGlobal::locale()->formatByteSize( content->body().size() );
    }

    KIcon iconForContent( KMime::Content *content )
    {
      if ( content->contentType( false ) )
      {
        KMimeType::Ptr mimeType = KMimeType::mimeType( QString::fromLatin1( content->contentType()->mimeType() ) );
        if ( mimeType.isNull() || mimeType->name() == QLatin1String("application/octet-stream") ) {
            const QString name = descriptionForContent(content);
            mimeType = MessageViewer::Util::mimetype(name);
        }
        if ( mimeType.isNull() || mimeType->iconName().isEmpty() )
          return KIcon();
        if( mimeType->name().startsWith( QLatin1String( "multipart/" ) ) )
          return KIcon( QLatin1String("folder") );
        return KIcon( mimeType->iconName() );
      }
      else
        return KIcon();
    }

    KMime::Content *root;
};

MimeTreeModel::MimeTreeModel(QObject * parent) :
    QAbstractItemModel( parent ),
    d ( new Private )
{
}

MimeTreeModel::~MimeTreeModel()
{
  delete d;
}

void MimeTreeModel::setRoot(KMime::Content * root)
{
  d->root = root;
  reset();
}

KMime::Content* MimeTreeModel::root()
{
  return d->root;
}


QModelIndex MimeTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if ( !parent.isValid() ) {
    if ( row != 0 )
      return QModelIndex();
    return createIndex( row, column, d->root );
  }

  KMime::Content *parentContent = static_cast<KMime::Content*>( parent.internalPointer() );
  if ( !parentContent || parentContent->contents().count() <= row || row < 0 )
    return QModelIndex();
  KMime::Content *content = parentContent->contents().at( row );
  return createIndex( row, column, content );
}

QModelIndex MimeTreeModel::parent(const QModelIndex & index) const
{
  if ( !index.isValid() )
    return QModelIndex();
  KMime::Content *currentContent = static_cast<KMime::Content*>( index.internalPointer() );
  if ( !currentContent )
    return QModelIndex();

  KMime::ContentIndex currentIndex = d->root->indexForContent( currentContent );
  if ( !currentIndex.isValid() )
    return QModelIndex();
  d->contentIndexUp( currentIndex );
  KMime::Content *parentContent = d->root->content( currentIndex );
  int row = 0;
  if ( currentIndex.isValid() )
    row = d->contentIndexUp( currentIndex ) - 1; // 1 based -> 0 based

  return createIndex( row, 0, parentContent );
}

int MimeTreeModel::rowCount(const QModelIndex & parent) const
{
  if ( !d->root )
    return 0;
  if ( !parent.isValid() )
    return 1;
  KMime::Content *parentContent = static_cast<KMime::Content*>( parent.internalPointer() );
  if ( parentContent )
    return parentContent->contents().count();
  return 0;
}

int MimeTreeModel::columnCount(const QModelIndex & parent) const
{
  Q_UNUSED( parent );
  return 3;
}

QVariant MimeTreeModel::data(const QModelIndex & index, int role) const
{
  KMime::Content *content = static_cast<KMime::Content*>( index.internalPointer() );
  if ( !content )
    return QVariant();
  if ( role == Qt::ToolTipRole ) {
    // TODO
    //return d->root->indexForContent( content ).toString();
  }
  if ( role == Qt::DisplayRole ) {
    switch( index.column() ) {
      case 0:
        return d->descriptionForContent( content );
      case 1:
        return d->typeForContent( content );
      case 2:
        return d->sizeOfContent( content );
    }
  }
  if ( role == Qt::DecorationRole && index.column() == 0 ) {
    return d->iconForContent( content );
  }
  if ( role == ContentIndexRole )
    return QVariant::fromValue( d->root->indexForContent( content ) );
  if ( role == ContentRole )
    return QVariant::fromValue( content );
  if ( role == MimeTypeRole )
    return d->mimeTypeForContent( content );
  if ( role == MainBodyPartRole ) {
    KMime::Message* topLevelMsg = dynamic_cast<KMime::Message*>( d->root );
    if ( !topLevelMsg )
      return false;
    return topLevelMsg->mainBodyPart() == content;
  }
  if ( role == AlternativeBodyPartRole ) {
    KMime::Message* topLevelMsg = dynamic_cast<KMime::Message*>( d->root );
    if ( !topLevelMsg )
      return false;
    return topLevelMsg->mainBodyPart( content->contentType()->mimeType() ) == content;
  }
  return QVariant();
}

QVariant MimeTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
    switch ( section ) {
      case 0:
        return i18n( "Description" );
      case 1:
        return i18n( "Type" );
      case 2:
        return i18n( "Size" );
    }
  }
  return QAbstractItemModel::headerData( section, orientation, role );
}


#include "moc_mimetreemodel.cpp"
