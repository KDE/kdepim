/*   -*- mode: C++; c-file-style: "gnu" -*-
*   kmail: KDE mail client
*   Copyright (C) 2006 Dmitry Morozhnikov <dmiceman@mail.ru>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License along
*   with this program; if not, write to the Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "customtemplates.h"

#include <qfont.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <QWhatsThis>
#include <qtoolbox.h>

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <kshortcut.h>
#include <kmessagebox.h>
#include <kkeysequencewidget.h>

#include "ui_customtemplates_base.h"
#include "customtemplates_kfg.h"
#include "globalsettings_base.h"

CustomTemplates::CustomTemplates( QWidget *parent, const char *name )
  : QWidget( parent ),
    mBlockChangeSignal( false )
{
  mUi = new Ui_CustomTemplatesBase;
  mUi->setupUi( this );
  setObjectName(name);

  QFont f = KGlobalSettings::fixedFont();
  mUi->mEdit->setFont( f );

  mUi->mAdd->setIcon( KIcon( "list-add" ) );
  mUi->mAdd->setEnabled( false );
  mUi->mRemove->setIcon( KIcon( "list-remove" ) );

  mUi->mList->setColumnWidth( 0, 100 );
  mUi->mList->header()->setStretchLastSection( true );

  mUi->mEditFrame->setEnabled( false );

  connect( mUi->mEdit, SIGNAL( textChanged() ),
          this, SLOT( slotTextChanged( void ) ) );
  connect( mUi->mToEdit, SIGNAL( textChanged() ),
           this, SLOT( slotTextChanged() ) );
  connect( mUi->mCCEdit, SIGNAL( textChanged() ),
           this, SLOT( slotTextChanged() ) );

  connect( mUi->mName, SIGNAL( textChanged( const QString & ) ),
           this, SLOT( slotNameChanged( const QString & ) ) );

  connect( mUi->mInsertCommand, SIGNAL( insertCommand(const QString&, int) ),
          this, SLOT( slotInsertCommand(const QString&, int) ) );

  connect( mUi->mAdd, SIGNAL( clicked() ),
          this, SLOT( slotAddClicked() ) );
  connect( mUi->mRemove, SIGNAL( clicked() ),
          this, SLOT( slotRemoveClicked() ) );
  connect(mUi->mList, SIGNAL( itemSelectionChanged() ),
          this, SLOT( slotListSelectionChanged() ) );
  connect( mUi->mType, SIGNAL( activated( int ) ),
          this, SLOT( slotTypeActivated( int ) ) );

  connect( mUi->mKeySequenceWidget, SIGNAL( keySequenceChanged( const QKeySequence& ) ),
          this, SLOT( slotShortcutChanged( const QKeySequence& ) ) );

// TODO(leo) still check with kmail? kmail can do the checking
 // mKeySequenceWidget->setCheckActionCollections( kmkernel->getKMMainWidget()->actionCollections() );

  mReplyPix = KIconLoader().loadIcon( "mail-reply-sender", KIconLoader::Small );
  mReplyAllPix = KIconLoader().loadIcon( "mail-reply-all", KIconLoader::Small );
  mForwardPix = KIconLoader().loadIcon( "mail-forward", KIconLoader::Small );

  mUi->mType->clear();
  mUi->mType->addItem( QPixmap(), i18nc( "Message->", "Universal" ) );
  mUi->mType->addItem( mReplyPix, i18nc( "Message->", "Reply" ) );
  mUi->mType->addItem( mReplyAllPix, i18nc( "Message->", "Reply to All" ) );
  mUi->mType->addItem( mForwardPix, i18nc( "Message->", "Forward" ) );

  mUi->mHelp->setText( i18n( "<a href=\"whatsthis\">How does this work?</a>" ) );
  connect( mUi->mHelp, SIGNAL( linkActivated ( const QString& ) ),
          SLOT( slotHelpLinkClicked( const QString& ) ) );

  const QString toToolTip = i18n( "Additional recipients of the message" );
  const QString ccToolTip = i18n( "Additional recipients who get a copy of the message" );
  const QString toWhatsThis = i18n( "When using this template, the default recipients are those you enter here. This is a comma-separated list of mail addresses." );
  const QString ccWhatsThis = i18n( "When using this template, the recipients you enter here will by default get a copy of this message. This is a comma-separated list of mail addresses." );

  KLineEdit *ccLineEdit = mUi->mCCEdit->lineEdit();
  KLineEdit *toLineEdit = mUi->mToEdit->lineEdit();

  mUi->mCCLabel->setToolTip( ccToolTip );
  ccLineEdit->setToolTip( ccToolTip );
  mUi->mToLabel->setToolTip( toToolTip );
  toLineEdit->setToolTip( toToolTip );
  mUi->mCCLabel->setWhatsThis( ccWhatsThis );
  ccLineEdit->setWhatsThis( ccWhatsThis );
  mUi->mToLabel->setWhatsThis( toWhatsThis );
  toLineEdit->setWhatsThis( toWhatsThis );

  slotNameChanged( mUi->mName->text() );
}

void CustomTemplates::slotHelpLinkClicked( const QString& )
{
  QString help =
      i18n( "<qt>"
      "<p>Here you can add, edit, and delete custom message "
      "templates to use when you compose a reply or forwarding message. "
      "Create the custom template by selecting it using the right mouse "
      " button menu or toolbar menu. Also, you can bind a keyboard "
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
  qDeleteAll( mItemList ); // no auto-delete with QHash
  delete mUi;
  mUi = 0;
}

void CustomTemplates::slotNameChanged( const QString& text )
{
  mUi->mAdd->setEnabled( !text.isEmpty() );
}

QString CustomTemplates::indexToType( int index )
{
  QString typeStr;
  switch ( index ) {
  case TUniversal:
    typeStr = i18nc( "Message->", "Universal" ); break;
/*  case TNewMessage:
    typeStr = i18n( "New Message" ); break;*/
  case TReply:
    typeStr = i18nc( "Message->", "Reply" ); break;
  case TReplyAll:
    typeStr = i18nc( "Message->", "Reply to All" ); break;
  case TForward:
    typeStr = i18nc( "Message->", "Forward" ); break;
  default:
    typeStr = i18nc( "Message->", "Unknown" ); break;
  }
  return typeStr;
}

void CustomTemplates::slotTextChanged()
{
  if ( mUi->mList->currentItem() ) {
    CustomTemplateItem *vitem = mItemList[ mUi->mList->currentItem()->text( 1 ) ];
    if ( vitem ) {
      vitem->mContent = mUi->mEdit->toPlainText();
      if ( !mBlockChangeSignal ) {
        vitem->mTo = mUi->mToEdit->text();
        vitem->mCC = mUi->mCCEdit->text();
      }
    }
  }

  if ( !mBlockChangeSignal )
    emit changed();
}

void CustomTemplates::load()
{
  QStringList list = TemplateParser::GlobalSettings::self()->customTemplates();
  for ( QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it ) {
    CTemplates t(*it);
    // QString typeStr = indexToType( t.type() );
    QString typeStr;
    KShortcut shortcut( t.shortcut() );
    CustomTemplateItem *vitem =
      new CustomTemplateItem( *it, t.content(),
        shortcut,
        static_cast<Type>( t.type() ), t.to(), t.cC() );
    mItemList.insert( *it, vitem );
    QTreeWidgetItem *item = new QTreeWidgetItem( mUi->mList );
    item->setText( 0, typeStr );
    item->setText( 1, *it );
    item->setText( 0, indexToType( t.type() ) );
    switch ( t.type() ) {
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

  mUi->mRemove->setEnabled( mUi->mList->topLevelItemCount() > 0 );
}

void CustomTemplates::save()
{
  // Before saving the new templates, delete the old ones. That needs to be done before
  // saving, since otherwise a new template with the new name wouldn't get saved.
  foreach( const QString &item, mItemsToDelete ) {
    CTemplates t( item );
    const QString configGroup = t.currentGroup();
    TemplateParser::GlobalSettings::self()->config()->deleteGroup( configGroup );
  }

  QStringList list;
  QTreeWidgetItemIterator lit( mUi->mList );
  while ( *lit ) {
    list.append( (*lit)->text( 1 ) );
    ++lit;
  }

  foreach( const CustomTemplateItem *item, mItemList ) {
    CTemplates t( item->mName );
    QString content = item->mContent;
    if ( content.trimmed().isEmpty() ) {
      content = "%BLANK";
    }
    t.setContent( content );
    t.setShortcut( item->mShortcut.toString() );
    t.setType( item->mType );
    t.setTo( item->mTo );
    t.setCC( item->mCC );
    t.writeConfig();
  }
  TemplateParser::GlobalSettings::self()->setCustomTemplates( list );
  TemplateParser::GlobalSettings::self()->writeConfig();

  emit templatesUpdated();
}

void CustomTemplates::slotInsertCommand( const QString &cmd, int adjustCursor )
{
  QTextCursor cursor = mUi->mEdit->textCursor();
  cursor.insertText( cmd );
  cursor.setPosition( cursor.position() + adjustCursor );
  mUi->mEdit->setTextCursor( cursor );
  mUi->mEdit->setFocus();
}

void CustomTemplates::slotAddClicked()
{
  QString str = mUi->mName->text();
  if ( !str.isEmpty() ) {
    CustomTemplateItem *vitem = mItemList[ str ];
    if ( !vitem ) {
      // KShortcut::null() doesn't seem to be present, although documented
      // at http://developer.kde.org/documentation/library/cvs-api/kdelibs-apidocs/kdecore/html/classKShortcut.html
      // see slotShortcutChanged(). oh, and you should look up documentation on the english breakfast network!
      // FIXME There must be a better way of doing this...
      KShortcut nullShortcut;
      vitem = new CustomTemplateItem( str, "", nullShortcut, TUniversal,
                                      QString(), QString() );
      mItemList.insert( str, vitem );
      QTreeWidgetItem *item =
        new QTreeWidgetItem( mUi->mList );
      item->setText( 0, indexToType( TUniversal ) );
      item->setText( 1, str );
      mUi->mList->setCurrentItem( item );
      mUi->mRemove->setEnabled( true );
      mUi->mName->clear();
      mUi->mKeySequenceWidget->setEnabled( false );
      if ( !mBlockChangeSignal )
        emit changed();
    }
  }
}

void CustomTemplates::slotRemoveClicked()
{
  if ( !mUi->mList->currentItem() )
    return;

  const QString templateName = mUi->mList->currentItem()->text( 1 );
  mItemsToDelete.append( templateName );
  delete mItemList.take( templateName );
  delete mUi->mList->takeTopLevelItem( mUi->mList->indexOfTopLevelItem (
                                  mUi->mList->currentItem() ) );
  mUi->mRemove->setEnabled( mUi->mList->topLevelItemCount() > 0 );
  if ( !mBlockChangeSignal )
    emit changed();
}

void CustomTemplates::slotListSelectionChanged()
{
  QTreeWidgetItem *item = mUi->mList->currentItem();
  if ( item ) {
    mUi->mEditFrame->setEnabled( true );
    CustomTemplateItem *vitem = mItemList[ mUi->mList->currentItem()->text( 1 ) ];
    if ( vitem ) {

      mBlockChangeSignal = true;
      mUi->mEdit->setText( vitem->mContent );
      mUi->mKeySequenceWidget->setKeySequence( vitem->mShortcut.primary(),
                                          KKeySequenceWidget::NoValidate );
      mUi->mType->setCurrentIndex( mUi->mType->findText( indexToType ( vitem->mType ) ) );
      mUi->mToEdit->setText( vitem->mTo );
      mUi->mCCEdit->setText( vitem->mCC );
      mBlockChangeSignal = false;

      // I think the logic (originally 'vitem->mType==TUniversal') was inverted here:
      // a key shortcut is only allowed for a specific type of template and not for
      // a universal, as otherwise we won't know what sort of action to do when the
      // key sequence is activated!
      // This agrees with KMMainWidget::updateCustomTemplateMenus() -- marten
      mUi->mKeySequenceWidget->setEnabled( vitem->mType != TUniversal );
    }
  } else {
    mUi->mEditFrame->setEnabled( false );
    mUi->mEdit->clear();
    // see above
    mUi->mKeySequenceWidget->clearKeySequence();
    mUi->mType->setCurrentIndex( 0 );
    mUi->mToEdit->clear();
    mUi->mCCEdit->clear();
  }
}

void CustomTemplates::slotTypeActivated( int index )
{
  if ( mUi->mList->currentItem() ) {
    // mCurrentItem->setText( 0, indexToType( index ) );
    CustomTemplateItem *vitem = mItemList[ mUi->mList->currentItem()->text( 1 ) ];
    if ( !vitem )
      return;

    vitem->mType = static_cast<Type>(index);
    mUi->mList->currentItem()->setText( 0, indexToType( vitem->mType ) );
    switch ( vitem->mType ) {
    case TReply:
      mUi->mList->currentItem()->setIcon( 0, mReplyPix );
      break;
    case TReplyAll:
      mUi->mList->currentItem()->setIcon( 0, mReplyAllPix );
      break;
    case TForward:
      mUi->mList->currentItem()->setIcon( 0, mForwardPix );
      break;
    default:
      mUi->mList->currentItem()->setIcon( 0, QPixmap() );
      break;
    };

    // see slotListSelectionChanged() above
    mUi->mKeySequenceWidget->setEnabled( vitem->mType != TUniversal );

    if ( !mBlockChangeSignal )
      emit changed();
  }
}

void CustomTemplates::slotShortcutChanged( const QKeySequence &newSeq )
{
    if ( mUi->mList->currentItem() )
    {
      mItemList[ mUi->mList->currentItem()->text( 1 ) ]->mShortcut =
          KShortcut( newSeq );
      mUi->mKeySequenceWidget->applyStealShortcut();
    }

    if ( !mBlockChangeSignal )
      emit changed();
}

#include "customtemplates.moc"
