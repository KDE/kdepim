/*
 * Copyright (C) 2006 Dmitry Morozhnikov <dmiceman@mail.ru>
 * Copyright (C) 2011,2012 Laurent Montel <montel@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "customtemplates.h"
#include "customtemplates_kfg.h"
#include "globalsettings_base.h"
#include "ui_customtemplates_base.h"
#include "pimcommon/texteditor/richtexteditor/richtexteditor.h"

#include <KIconLoader>
#include <KLocale>
#include <KMessageBox>

#include <QWhatsThis>

using namespace TemplateParser;

CustomTemplates::CustomTemplates( const QList<KActionCollection*> &actionCollection,
                                  QWidget *parent )
  : QWidget( parent ),
    mBlockChangeSignal( false )
{
  mUi = new Ui_CustomTemplatesBase;
  mUi->setupUi( this );

  mUi->mAdd->setIcon( KIcon( QLatin1String("list-add") ) );
  mUi->mAdd->setEnabled( false );
  mUi->mRemove->setIcon( KIcon( QLatin1String("list-remove") ) );
  mUi->mDuplicate->setIcon( KIcon( QLatin1String("edit-copy") ) );

  mUi->mList->setColumnWidth( 0, 100 );
  mUi->mList->header()->setStretchLastSection( true );
  mUi->mList->setItemDelegate( new CustomTemplateItemDelegate( this ) );
  mUi->mList->header()->setMovable( false );
  mUi->mEditFrame->setEnabled( false );

  mUi->mName->setTrapReturnKey( true );
  connect( mUi->mEdit->editor(), SIGNAL(textChanged()),
           this, SLOT(slotTextChanged()) );
  connect( mUi->mToEdit, SIGNAL(textChanged()),
           this, SLOT(slotTextChanged()) );
  connect( mUi->mCCEdit, SIGNAL(textChanged()),
           this, SLOT(slotTextChanged()) );

  connect( mUi->mName, SIGNAL(textChanged(QString)),
           this, SLOT(slotNameChanged(QString)) );

  connect( mUi->mName, SIGNAL(returnPressed()),
           this, SLOT(slotAddClicked()) );

  connect( mUi->mInsertCommand, SIGNAL(insertCommand(QString,int)),
           this, SLOT(slotInsertCommand(QString,int)) );

  connect( mUi->mAdd, SIGNAL(clicked()),
           this, SLOT(slotAddClicked()) );
  connect( mUi->mRemove, SIGNAL(clicked()),
           this, SLOT(slotRemoveClicked()) );
  connect( mUi->mDuplicate, SIGNAL(clicked()),
           this, SLOT(slotDuplicateClicked()) );
  connect( mUi->mList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
           this, SLOT(slotListSelectionChanged()) );
  connect( mUi->mList, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
           this, SLOT(slotItemChanged(QTreeWidgetItem*,int)) );
  connect( mUi->mType, SIGNAL(activated(int)),
           this, SLOT(slotTypeActivated(int)) );

  connect( mUi->mKeySequenceWidget, SIGNAL(keySequenceChanged(QKeySequence)),
          this, SLOT(slotShortcutChanged(QKeySequence)) );

  mUi->mKeySequenceWidget->setCheckActionCollections( actionCollection );

  mReplyPix = KIconLoader().loadIcon( QLatin1String("mail-reply-sender"), KIconLoader::Small );
  mReplyAllPix = KIconLoader().loadIcon( QLatin1String("mail-reply-all"), KIconLoader::Small );
  mForwardPix = KIconLoader().loadIcon( QLatin1String("mail-forward"), KIconLoader::Small );

  mUi->mType->clear();
  mUi->mType->addItem( QPixmap(), i18nc( "Message->", "Universal" ) );
  mUi->mType->addItem( mReplyPix, i18nc( "Message->", "Reply" ) );
  mUi->mType->addItem( mReplyAllPix, i18nc( "Message->", "Reply to All" ) );
  mUi->mType->addItem( mForwardPix, i18nc( "Message->", "Forward" ) );

  mUi->mHelp->setText( i18n( "<a href=\"whatsthis\">How does this work?</a>" ) );
  connect( mUi->mHelp, SIGNAL(linkActivated(QString)),
          SLOT(slotHelpLinkClicked(QString)) );

  slotNameChanged( mUi->mName->text() );
}

void CustomTemplates::slotHelpLinkClicked( const QString & )
{
  const QString help =
    i18n( "<qt>"
          "<p>Here you can add, edit, and delete custom message "
          "templates to use when you compose a reply or forwarding message. "
          "Create the custom template by typing the name into the input box "
          "and press the '+' button. Also, you can bind a keyboard "
          "combination to the template for faster operations.</p>"
          "<p>Message templates support substitution commands, "
          "by simply typing them or selecting them from the "
          "<i>Insert command</i> menu.</p>"
          "<p>There are four types of custom templates: used to "
          "<i>Reply</i>, <i>Reply to All</i>, <i>Forward</i>, and "
          "<i>Universal</i> which can be used for all kinds of operations. "
          "You cannot bind a keyboard shortcut to <i>Universal</i> templates.</p>"
          "</qt>" );

  QWhatsThis::showText( QCursor::pos(), help );
}

CustomTemplates::~CustomTemplates()
{
  delete mUi;
  mUi = 0;
}

void CustomTemplates::slotNameChanged( const QString &text )
{
  mUi->mAdd->setEnabled( !text.isEmpty() );
}

QString CustomTemplates::indexToType( int index )
{
  QString typeStr;
  switch ( index ) {
  case TUniversal:
    typeStr = i18nc( "Message->", "Universal" );
    break;
/*  case TNewMessage:
    typeStr = i18n( "New Message" );
    break; */
  case TReply:
    typeStr = i18nc( "Message->", "Reply" );
    break;
  case TReplyAll:
    typeStr = i18nc( "Message->", "Reply to All" );
    break;
  case TForward:
    typeStr = i18nc( "Message->", "Forward" );
    break;
  default:
    typeStr = i18nc( "Message->", "Unknown" );
    break;
  }
  return typeStr;
}

void CustomTemplates::slotTextChanged()
{
  QTreeWidgetItem *item = mUi->mList->currentItem();
  if ( item ) {
    CustomTemplateItem *vitem = static_cast<CustomTemplateItem*>( item );
    vitem->setContent( mUi->mEdit->toPlainText() );
    if ( !mBlockChangeSignal ) {
      vitem->setTo( mUi->mToEdit->text() );
      vitem->setCc( mUi->mCCEdit->text() );
    }
  }

  if ( !mBlockChangeSignal ) {
    emit changed();
  }
}

void CustomTemplates::iconFromType( CustomTemplates::Type type, CustomTemplateItem *item )
{
  switch ( type ) {
  case TReply:
    item->setIcon( 0, mReplyPix );
    break;
  case TReplyAll:
    item->setIcon( 0, mReplyAllPix );
    break;
  case TForward:
    item->setIcon( 0, mForwardPix );
    break;
  default:
    item->setIcon( 0, QPixmap() );
    break;
  };

}

void CustomTemplates::load()
{
  const QStringList list = GlobalSettings::self()->customTemplates();
  mUi->mList->clear();
  QStringList::const_iterator end( list.constEnd() );
  for ( QStringList::const_iterator it = list.constBegin(); it != end; ++it ) {
    CTemplates t(*it);
    QKeySequence shortcut( t.shortcut() );
    CustomTemplates::Type type = static_cast<Type>( t.type() );
    CustomTemplateItem *item = new CustomTemplateItem( mUi->mList, *it, t.content(),
                                                       shortcut, type, t.to(), t.cC() );
    item->setText( 1, *it );
    item->setText( 0, indexToType( type ) );
    iconFromType( type, item );
  }

  mUi->mRemove->setEnabled( mUi->mList->topLevelItemCount() > 0 && mUi->mList->currentItem() );
  mUi->mDuplicate->setEnabled( mUi->mList->topLevelItemCount() > 0 && mUi->mList->currentItem() );
}

void CustomTemplates::save()
{
  // Before saving the new templates, delete the old ones. That needs to be done before
  // saving, since otherwise a new template with the new name wouldn't get saved.
  KSharedConfig::Ptr config = KSharedConfig::openConfig( QLatin1String("customtemplatesrc"), KConfig::NoGlobals );
  foreach ( const QString &item, mItemsToDelete ) {
    CTemplates t( item );
    const QString configGroup = t.currentGroup();
    config->deleteGroup( configGroup );
  }

  QStringList list;
  QTreeWidgetItemIterator lit( mUi->mList );
  while ( *lit ) {
    CustomTemplateItem * it = static_cast<CustomTemplateItem*>( *lit );
    const QString name = it->text( 1 );
    list.append(name);

    CTemplates t(name);
    QString content = it->content();
    if ( content.trimmed().isEmpty() ) {
      content = QLatin1String("%BLANK");
    }

    t.setContent( content );
    t.setShortcut( it->shortcut().toString() );
    t.setType( it->customType() );
    t.setTo( it->to() );
    t.setCC( it->cc() );
    t.writeConfig();
    ++lit;
  }

  GlobalSettings::self()->setCustomTemplates( list );
  GlobalSettings::self()->writeConfig();

  emit templatesUpdated();
}

void CustomTemplates::slotInsertCommand( const QString &cmd, int adjustCursor )
{
  QTextCursor cursor = mUi->mEdit->editor()->textCursor();
  cursor.insertText( cmd );
  cursor.setPosition( cursor.position() + adjustCursor );
  mUi->mEdit->editor()->setTextCursor( cursor );
  mUi->mEdit->editor()->setFocus();
}

bool CustomTemplates::nameAlreadyExists( const QString &str, QTreeWidgetItem *item )
{
  QTreeWidgetItemIterator lit( mUi->mList );
  while ( *lit ) {
    const QString name = ( *lit )->text( 1 );
    if ( ( name == str ) && ( (*lit) != item ) ) {
      KMessageBox::error(
        this,
        i18n( "A template with same name already exists." ),
        i18n( "Cannot create template" ) );
      return true;
    }
    ++lit;
  }
  return false;
}

void CustomTemplates::slotAddClicked()
{
  const QString str = mUi->mName->text();
  if ( !str.isEmpty() ) {
    if ( nameAlreadyExists( str ) ) {
      return;
    }

    // KShortcut::null() doesn't seem to be present, although documented
    // see slotShortcutChanged(). oh, and you should look up documentation on the EBN!
    // FIXME There must be a better way of doing this...
    QKeySequence nullShortcut;
    CustomTemplateItem *item =
            new CustomTemplateItem( mUi->mList, str, QString(), nullShortcut, TUniversal,
                              QString(), QString() );
    item->setText( 0, indexToType( TUniversal ) );
    item->setText( 1, str );
    mUi->mList->setCurrentItem( item );
    mUi->mRemove->setEnabled( true );
    mUi->mDuplicate->setEnabled( true );
    mUi->mName->clear();
    mUi->mKeySequenceWidget->setEnabled( false );
    if ( !mBlockChangeSignal ) {
      emit changed();
    }
  }
}

QString CustomTemplates::createUniqueName( const QString &name ) const
{
  QString uniqueName = name;

  int counter = 0;
  bool found = true;

  while ( found ) {
    found = false;

    QTreeWidgetItemIterator lit( mUi->mList );
    while ( *lit ) {
      const QString itemName = ( *lit )->text( 1 );
      if ( !itemName.compare( uniqueName ) ) {
        found = true;
        ++counter;
        uniqueName = name;
        uniqueName += QLatin1String( " (" ) + QString::number( counter ) + QLatin1String( ")" );
        break;
      }
      lit++;
    }
  }

  return uniqueName;
}

void CustomTemplates::slotDuplicateClicked()
{
  QTreeWidgetItem *currentItem = mUi->mList->currentItem();
  if ( !currentItem ) {
    return;
  }
  CustomTemplateItem * origItem = static_cast<CustomTemplateItem*>( currentItem );
  const QString templateName = createUniqueName( origItem->text( 1 ) );
  // KShortcut::null() doesn't seem to be present, although documented
  // see slotShortcutChanged(). oh, and you should look up documentation on the EBN!
  // FIXME There must be a better way of doing this...
  QKeySequence nullShortcut;
  CustomTemplates::Type type =  origItem->customType();
  CustomTemplateItem *item =
      new CustomTemplateItem( mUi->mList, templateName, origItem->content(), nullShortcut, type,
                              origItem->to(), origItem->cc() );
  item->setText( 0, indexToType( type ) );
  item->setText( 1, templateName );
  iconFromType( type, item );

  mUi->mList->setCurrentItem( item );
  mUi->mRemove->setEnabled( true );
  mUi->mDuplicate->setEnabled( true );
  mUi->mName->clear();
  mUi->mKeySequenceWidget->setEnabled( type != TUniversal );

  emit changed();
}

void CustomTemplates::slotRemoveClicked()
{
  QTreeWidgetItem *item = mUi->mList->currentItem();
  if ( !item ) {
    return;
  }

  const QString templateName = item->text( 1 );

  if ( KMessageBox::warningContinueCancel(
         this,
         i18nc( "@info", "Do you really want to remove template \"%1\"?", templateName ),
         i18nc( "@title:window", "Remove Template?" ),
         KStandardGuiItem::remove(),
         KStandardGuiItem::cancel() ) == KMessageBox::Continue ) {
    mItemsToDelete.append( templateName );
    delete mUi->mList->takeTopLevelItem( mUi->mList->indexOfTopLevelItem( item ) );
    mUi->mRemove->setEnabled( mUi->mList->topLevelItemCount() > 0 );
    mUi->mDuplicate->setEnabled( mUi->mList->topLevelItemCount() > 0 );
    if ( !mBlockChangeSignal ) {
      emit changed();
    }
  }
}

void CustomTemplates::slotListSelectionChanged()
{
  QTreeWidgetItem *item = mUi->mList->currentItem();
  if ( item ) {
    mUi->mEditFrame->setEnabled( true );
    mUi->mRemove->setEnabled(true);
    mUi->mDuplicate->setEnabled(true);
    CustomTemplateItem *vitem = static_cast<CustomTemplateItem*>( item );
    mBlockChangeSignal = true;
    mUi->mEdit->setPlainText( vitem->content() );
    mUi->mKeySequenceWidget->setKeySequence( vitem->shortcut(),
                                             KKeySequenceWidget::NoValidate );
    CustomTemplates::Type type =  vitem->customType();

    mUi->mType->setCurrentIndex( mUi->mType->findText( indexToType ( type ) ) );
    mUi->mToEdit->setText( vitem->to() );
    mUi->mCCEdit->setText( vitem->cc() );
    mBlockChangeSignal = false;

    // I think the logic (originally 'vitem->mType==TUniversal') was inverted here:
    // a key shortcut is only allowed for a specific type of template and not for
    // a universal, as otherwise we won't know what sort of action to do when the
    // key sequence is activated!
    // This agrees with KMMainWidget::updateCustomTemplateMenus() -- marten
    mUi->mKeySequenceWidget->setEnabled( type != TUniversal );
  } else {
    mUi->mEditFrame->setEnabled( false );
    mUi->mEdit->editor()->clear();
    // see above
    mUi->mKeySequenceWidget->clearKeySequence();
    mUi->mType->setCurrentIndex( 0 );
    mUi->mToEdit->clear();
    mUi->mCCEdit->clear();
  }
}

void CustomTemplates::slotTypeActivated( int index )
{
  QTreeWidgetItem *item = mUi->mList->currentItem();
  if ( item ) {
    CustomTemplateItem *vitem = static_cast<CustomTemplateItem*>( item );
    CustomTemplates::Type customtype = static_cast<Type>(index);
    vitem->setCustomType( customtype );
    vitem->setText( 0, indexToType( customtype ) );

    iconFromType( customtype, vitem );

    // see slotListSelectionChanged() above
    mUi->mKeySequenceWidget->setEnabled( customtype != TUniversal );

    if ( !mBlockChangeSignal ) {
      emit changed();
    }
  }
}

void CustomTemplates::slotShortcutChanged( const QKeySequence &newSeq )
{
  QTreeWidgetItem *item = mUi->mList->currentItem();
  if ( item ) {
    CustomTemplateItem * vitem = static_cast<CustomTemplateItem*>( item );
    vitem->setShortcut( newSeq );
    mUi->mKeySequenceWidget->applyStealShortcut();
  }

  if ( !mBlockChangeSignal ) {
    emit changed();
  }
}

void CustomTemplates::slotItemChanged( QTreeWidgetItem *item, int column )
{
  if ( item ) {
    CustomTemplateItem * vitem = static_cast<CustomTemplateItem*>( item );
    if ( column == 1 ) {
      const QString newName = vitem->text( 1 );
      if( !newName.isEmpty() ) {
        const QString oldName = vitem->oldName();
        if ( nameAlreadyExists( newName, item ) ) {
          vitem->setText( 1, oldName );
          return;
        }
        if ( newName != oldName ) {
          mItemsToDelete.append( oldName );
          vitem->setOldName( newName );
          if ( !mBlockChangeSignal ) {
            emit changed();
          }
        }
      }
    }
  }
}

CustomTemplateItemDelegate::CustomTemplateItemDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

CustomTemplateItemDelegate::~CustomTemplateItemDelegate()
{
}

void CustomTemplateItemDelegate::setModelData( QWidget *editor, QAbstractItemModel *model,
                                               const QModelIndex &index ) const
{
  KLineEdit *lineEdit = static_cast<KLineEdit*>( editor );
  const QString text = lineEdit->text();
  if( !text.isEmpty() ) {
    model->setData( index, text, Qt::EditRole );
  }
}

QWidget *CustomTemplateItemDelegate::createEditor( QWidget *parent,
                                                   const QStyleOptionViewItem &option,
                                                   const QModelIndex &index ) const
{
  if ( index.column() == 1 ) {
    return QStyledItemDelegate::createEditor( parent, option, index );
  }
  return 0;
}

CustomTemplateItem::CustomTemplateItem( QTreeWidget *parent,
                                        const QString &name,
                                        const QString &content,
                                        const QKeySequence &shortcut,
                                        CustomTemplates::Type type,
                                        const QString &to,
                                        const QString &cc )
  : QTreeWidgetItem( parent ),
    mName( name ),
    mContent( content ),
    mShortcut(shortcut),
    mType( type ),
    mTo( to ),
    mCC( cc )
{
  setFlags( flags() | Qt::ItemIsEditable );
}

CustomTemplateItem::~CustomTemplateItem()
{
}

void CustomTemplateItem::setCustomType( CustomTemplates::Type type )
{
  mType = type;
}

CustomTemplates::Type CustomTemplateItem::customType() const
{
  return mType;
}

QString CustomTemplateItem::to() const
{
  return mTo;
}

QString CustomTemplateItem::cc() const
{
  return mCC;
}

QString CustomTemplateItem::content() const
{
  return mContent;
}

void CustomTemplateItem::setContent( const QString &content )
{
  mContent = content;
}

void CustomTemplateItem::setTo( const QString &to )
{
  mTo = to;
}

void CustomTemplateItem::setCc( const QString &cc )
{
  mCC = cc;
}

QKeySequence CustomTemplateItem::shortcut() const
{
  return mShortcut;
}

void CustomTemplateItem::setShortcut( const QKeySequence &shortcut )
{
  mShortcut = shortcut;
}

QString CustomTemplateItem::oldName() const
{
  return mName;
}

void CustomTemplateItem::setOldName( const QString &name )
{
  mName = name;
}

