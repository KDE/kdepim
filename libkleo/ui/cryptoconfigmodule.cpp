/*
    cryptoconfigmodule.cpp

    This file is part of kgpgcertmanager
    Copyright (c) 2004 Klar�vdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
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

#include "cryptoconfigmodule.h"
#include "cryptoconfigmodule_p.h"
#include "directoryserviceswidget.h"
#include "kdhorizontalline.h"

#include <kleo/cryptoconfig.h>

#include <klineedit.h>
#include <klocale.h>
#include <kdebug.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <kurlrequester.h>
#include <kicon.h>

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QRegExp>
#include <QPixmap>
#include <QVBoxLayout>
#include <QList>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QDesktopWidget>

using namespace Kleo;

inline KIcon loadIcon( QString s ) {
  return KIcon( s.replace( QRegExp( "[^a-zA-Z0-9_]" ), "-" ) );
}

static const KPageView::FaceType determineJanusFace( const Kleo::CryptoConfig * config ) {
  return config && config->componentList().size() < 2
    ? KPageView::Plain
    : KPageView::List ;
}

Kleo::CryptoConfigModule::CryptoConfigModule( Kleo::CryptoConfig* config, QWidget * parent )
  : KPageWidget( parent ), mConfig( config )
{
  const KPageView::FaceType type=determineJanusFace( config );
  setFaceType(type);
  QVBoxLayout * vlay = 0;
  QWidget * vbox = 0;
  if ( type == Plain ) {
    vbox = new QWidget(this);
    vlay = new QVBoxLayout( vbox );
    vlay->setSpacing( KDialog::spacingHint() );
    vlay->setMargin( 0 );
  }

  const QStringList components = config->componentList();
  for ( QStringList::const_iterator it = components.begin(); it != components.end(); ++it ) {
    //kDebug(5150) << "Component " << (*it).toLocal8Bit() << ":" << endl;
    Kleo::CryptoConfigComponent* comp = config->component( *it );
    Q_ASSERT( comp );
    if ( comp->groupList().empty() )
      continue;
    if ( type != Plain ) {
      vbox = new QWidget(this);
      vlay = new QVBoxLayout( vbox );
      vlay->setSpacing( KDialog::spacingHint() );
      vlay->setMargin( 0 );
      KPageWidgetItem *pageItem = new KPageWidgetItem( vbox, comp->description() );
      pageItem->setIcon( loadIcon( comp->iconName() ) );
      addPage(pageItem);
    }

    QScrollArea* scrollArea = new QScrollArea( this );
    vlay->addWidget( scrollArea );
    scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    scrollArea->setWidgetResizable( true );

    CryptoConfigComponentGUI* compGUI =
      new CryptoConfigComponentGUI( this, comp, scrollArea, (*it).toLocal8Bit() );
    scrollArea->setWidget( compGUI );
    // KJanusWidget doesn't seem to have iterators, so we store a copy...
    mComponentGUIs.append( compGUI );

    // Set a nice startup size
    const int deskHeight = QApplication::desktop()->height();
    int dialogHeight;
    if (deskHeight > 1000) // very big desktop ?
      dialogHeight = 800;
    else if (deskHeight > 650) // big desktop ?
      dialogHeight = 500;
    else // small (800x600, 640x480) desktop
      dialogHeight = 400;
    QSize sz = scrollArea->sizeHint();
    scrollArea->setMinimumSize( sz.width()
                                + scrollArea->style()->pixelMetric(QStyle::PM_ScrollBarExtent),
                                qMin( compGUI->sizeHint().height(), dialogHeight ) );
  }
}

void Kleo::CryptoConfigModule::save()
{
  bool changed = false;
  QList<CryptoConfigComponentGUI *>::Iterator it = mComponentGUIs.begin();
  for( ; it != mComponentGUIs.end(); ++it ) {
    if ( (*it)->save() )
      changed = true;
  }
  if ( changed )
    mConfig->sync(true /*runtime*/);
}

void Kleo::CryptoConfigModule::reset()
{
  QList<CryptoConfigComponentGUI *>::Iterator it = mComponentGUIs.begin();
  for( ; it != mComponentGUIs.end(); ++it ) {
    (*it)->load();
  }
}

void Kleo::CryptoConfigModule::defaults()
{
  QList<CryptoConfigComponentGUI *>::Iterator it = mComponentGUIs.begin();
  for( ; it != mComponentGUIs.end(); ++it ) {
    (*it)->defaults();
  }
}

void Kleo::CryptoConfigModule::cancel()
{
  mConfig->clear();
}

////

Kleo::CryptoConfigComponentGUI::CryptoConfigComponentGUI(
  CryptoConfigModule* module, Kleo::CryptoConfigComponent* component,
  QWidget* parent, const char* name )
  : QWidget( parent ),
    mComponent( component )
{
  setObjectName( name );
  QGridLayout * glay = new QGridLayout( this );
  glay->setSpacing( KDialog::spacingHint() );
  const QStringList groups = mComponent->groupList();
  if ( groups.size() > 1 ) {
    glay->setColumnMinimumWidth( 0, KDHorizontalLine::indentHint() );
    for ( QStringList::const_iterator it = groups.begin(), end = groups.end() ; it != end; ++it ) {
      Kleo::CryptoConfigGroup* group = mComponent->group( *it );
      Q_ASSERT( group );
      if ( !group )
        continue;
      KDHorizontalLine * hl = new KDHorizontalLine( group->description(), this );
      const int row = glay->rowCount();
      glay->addWidget( hl, row, 0, 1, 3 );
      mGroupGUIs.append( new CryptoConfigGroupGUI( module, group, glay, this ) );
    }
  } else if ( !groups.empty() ) {
    mGroupGUIs.append( new CryptoConfigGroupGUI( module, mComponent->group( groups.front() ), glay, this ) );
  }
  glay->setRowStretch( glay->rowCount(), 1 );
}


bool Kleo::CryptoConfigComponentGUI::save()
{
  bool changed = false;
  QList<CryptoConfigGroupGUI *>::Iterator it = mGroupGUIs.begin();
  for( ; it != mGroupGUIs.end(); ++it ) {
    if ( (*it)->save() )
      changed = true;
  }
  return changed;
}

void Kleo::CryptoConfigComponentGUI::load()
{
  QList<CryptoConfigGroupGUI *>::Iterator it = mGroupGUIs.begin();
  for( ; it != mGroupGUIs.end(); ++it )
    (*it)->load();
}

void Kleo::CryptoConfigComponentGUI::defaults()
{
  QList<CryptoConfigGroupGUI *>::Iterator it = mGroupGUIs.begin();
  for( ; it != mGroupGUIs.end(); ++it )
    (*it)->defaults();
}

////

Kleo::CryptoConfigGroupGUI::CryptoConfigGroupGUI(
  CryptoConfigModule* module, Kleo::CryptoConfigGroup* group,
  QGridLayout * glay, QWidget* widget, const char* name )
  : QObject( module ), mGroup( group )
{
  setObjectName( name );
  const int startRow = glay->rowCount();
  const QStringList entries = mGroup->entryList();
  for( QStringList::const_iterator it = entries.begin(), end = entries.end() ; it != end; ++it ) {
    Kleo::CryptoConfigEntry* entry = group->entry( *it );
    Q_ASSERT( entry );
    if ( entry->level() > CryptoConfigEntry::Level_Advanced ) continue;
    CryptoConfigEntryGUI* entryGUI =
      CryptoConfigEntryGUIFactory::createEntryGUI( module, entry, *it, glay, widget );
    if ( entryGUI ) {
      mEntryGUIs.append( entryGUI );
      entryGUI->load();
    }
  }
  const int endRow = glay->rowCount() - 1;
  if ( endRow < startRow )
    return;

  const QString iconName = group->iconName();
  if ( iconName.isEmpty() )
    return;

  QLabel * l = new QLabel( widget );
  l->setPixmap( loadIcon( iconName ).pixmap( K3Icon::SizeMedium, K3Icon::SizeMedium ) );
  glay->addWidget( l, startRow, 0, endRow-startRow+1, 1, Qt::AlignTop );
}

bool Kleo::CryptoConfigGroupGUI::save()
{
  bool changed = false;
  QList<CryptoConfigEntryGUI *>::Iterator it = mEntryGUIs.begin();
  for( ; it != mEntryGUIs.end(); ++it ) {
    if ( (*it)->isChanged() ) {
      (*it)->save();
      changed = true;
    }
  }
  return changed;
}

void Kleo::CryptoConfigGroupGUI::load()
{
  QList<CryptoConfigEntryGUI *>::Iterator it = mEntryGUIs.begin();
  for( ; it != mEntryGUIs.end(); ++it )
    (*it)->load();
}

void Kleo::CryptoConfigGroupGUI::defaults()
{
  QList<CryptoConfigEntryGUI *>::Iterator it = mEntryGUIs.begin();
  for( ; it != mEntryGUIs.end(); ++it )
    (*it)->resetToDefault();
}

////

CryptoConfigEntryGUI* Kleo::CryptoConfigEntryGUIFactory::createEntryGUI( CryptoConfigModule* module, Kleo::CryptoConfigEntry* entry, const QString& entryName, QGridLayout * glay, QWidget* widget, const char* name )
{
  if ( entry->isList() ) {
    switch( entry->argType() ) {
    case Kleo::CryptoConfigEntry::ArgType_None:
      // A list of options with no arguments (e.g. -v -v -v) is shown as a spinbox
      return new CryptoConfigEntrySpinBox( module, entry, entryName, glay, widget, name );
    case Kleo::CryptoConfigEntry::ArgType_Int:
    case Kleo::CryptoConfigEntry::ArgType_UInt:
      // Let people type list of numbers (1,2,3....). Untested.
      return new CryptoConfigEntryLineEdit( module, entry, entryName, glay, widget, name );
    case Kleo::CryptoConfigEntry::ArgType_URL:
    case Kleo::CryptoConfigEntry::ArgType_Path:
    case Kleo::CryptoConfigEntry::ArgType_DirPath:
    case Kleo::CryptoConfigEntry::ArgType_String:
      kWarning(5150) << "No widget implemented for list of type " << entry->argType() << endl;
      return 0; // TODO when the need arises :)
    case Kleo::CryptoConfigEntry::ArgType_LDAPURL:
      return new CryptoConfigEntryLDAPURL( module, entry, entryName, glay, widget, name );
    }
    kWarning(5150) << "No widget implemented for list of (unknown) type " << entry->argType() << endl;
    return 0;
  }

  switch( entry->argType() ) {
  case Kleo::CryptoConfigEntry::ArgType_None:
    return new CryptoConfigEntryCheckBox( module, entry, entryName, glay, widget, name );
  case Kleo::CryptoConfigEntry::ArgType_Int:
  case Kleo::CryptoConfigEntry::ArgType_UInt:
    return new CryptoConfigEntrySpinBox( module, entry, entryName, glay, widget, name );
  case Kleo::CryptoConfigEntry::ArgType_URL:
    return new CryptoConfigEntryURL( module, entry, entryName, glay, widget, name );
  case Kleo::CryptoConfigEntry::ArgType_Path:
    return new CryptoConfigEntryPath( module, entry, entryName, glay, widget, name );
  case Kleo::CryptoConfigEntry::ArgType_DirPath:
    return new CryptoConfigEntryDirPath( module, entry, entryName, glay, widget, name );
  case Kleo::CryptoConfigEntry::ArgType_LDAPURL:
      kWarning(5150) << "No widget implemented for type " << entry->argType() << endl;
      return 0; // TODO when the need arises :)
  case Kleo::CryptoConfigEntry::ArgType_String:
    return new CryptoConfigEntryLineEdit( module, entry, entryName, glay, widget, name );
  }
  kWarning(5150) << "No widget implemented for (unknown) type " << entry->argType() << endl;
  return 0;
}

////

Kleo::CryptoConfigEntryGUI::CryptoConfigEntryGUI(
  CryptoConfigModule* module,
  Kleo::CryptoConfigEntry* entry,
  const QString& entryName,
  const char* name )
  : QObject( module ), mEntry( entry ), mName( entryName ), mChanged( false )
{
  setObjectName( name );
  connect( this, SIGNAL( changed() ), module, SIGNAL( changed() ) );
}

QString Kleo::CryptoConfigEntryGUI::description() const
{
  QString descr = mEntry->description();
  if ( descr.isEmpty() ) // shouldn't happen
    descr = QString( "<%1>" ).arg( mName );
  return descr;
}

void Kleo::CryptoConfigEntryGUI::resetToDefault()
{
  mEntry->resetToDefault();
  load();
}

////

Kleo::CryptoConfigEntryLineEdit::CryptoConfigEntryLineEdit(
  CryptoConfigModule* module,
  Kleo::CryptoConfigEntry* entry, const QString& entryName,
  QGridLayout * glay, QWidget* widget, const char* name )
  : CryptoConfigEntryGUI( module, entry, entryName, name )
{
  const int row = glay->rowCount();
  mLineEdit = new KLineEdit( widget );
  QLabel *label = new QLabel( description(), widget );
  label->setBuddy( mLineEdit );
  glay->addWidget( label, row, 1 );
  glay->addWidget( mLineEdit, row, 2 );
  if ( entry->isReadOnly() ) {
    label->setEnabled( false );
    mLineEdit->setEnabled( false );
  } else {
    connect( mLineEdit, SIGNAL( textChanged( const QString& ) ), SLOT( slotChanged() ) );
  }
}

void Kleo::CryptoConfigEntryLineEdit::doSave()
{
  mEntry->setStringValue( mLineEdit->text() );
}

void Kleo::CryptoConfigEntryLineEdit::doLoad()
{
  mLineEdit->setText( mEntry->stringValue() );
}

////

Kleo::CryptoConfigEntryPath::CryptoConfigEntryPath(
  CryptoConfigModule* module,
  Kleo::CryptoConfigEntry* entry, const QString& entryName,
  QGridLayout * glay, QWidget* widget, const char* name )
  : CryptoConfigEntryGUI( module, entry, entryName, name )
{
  const int row = glay->rowCount();
  mUrlRequester = new KUrlRequester( widget );
  mUrlRequester->setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
  QLabel *label = new QLabel( description(), widget );
  label->setBuddy( mUrlRequester );
  glay->addWidget( label, row, 1 );
  glay->addWidget( mUrlRequester, row, 2 );
  if ( entry->isReadOnly() ) {
    label->setEnabled( false );
    mUrlRequester->setEnabled( false );
  } else {
    connect( mUrlRequester, SIGNAL( textChanged( const QString& ) ), SLOT( slotChanged() ) );
  }
}

void Kleo::CryptoConfigEntryPath::doSave()
{
  mEntry->setURLValue( mUrlRequester->url() );
}

void Kleo::CryptoConfigEntryPath::doLoad()
{
  mUrlRequester->setUrl( mEntry->urlValue() );
}

////

Kleo::CryptoConfigEntryDirPath::CryptoConfigEntryDirPath(
  CryptoConfigModule* module,
  Kleo::CryptoConfigEntry* entry, const QString& entryName,
  QGridLayout * glay, QWidget* widget, const char* name )
  : CryptoConfigEntryGUI( module, entry, entryName, name )
{
  const int row = glay->rowCount();
  mUrlRequester = new KUrlRequester( widget );
  mUrlRequester->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
  QLabel *label = new QLabel( description(), widget );
  label->setBuddy( mUrlRequester );
  glay->addWidget( label, row, 1 );
  glay->addWidget( mUrlRequester, row, 2 );
  if ( entry->isReadOnly() ) {
    label->setEnabled( false );
    mUrlRequester->setEnabled( false );
  } else {
    connect( mUrlRequester, SIGNAL( textChanged( const QString& ) ), SLOT( slotChanged() ) );
  }
}

void Kleo::CryptoConfigEntryDirPath::doSave()
{
  mEntry->setURLValue( mUrlRequester->url() );
}

void Kleo::CryptoConfigEntryDirPath::doLoad()
{
  mUrlRequester->setUrl( mEntry->urlValue() );
}

////

Kleo::CryptoConfigEntryURL::CryptoConfigEntryURL(
  CryptoConfigModule* module,
  Kleo::CryptoConfigEntry* entry, const QString& entryName,
  QGridLayout * glay, QWidget* widget, const char* name )
  : CryptoConfigEntryGUI( module, entry, entryName, name )
{
  const int row = glay->rowCount();
  mUrlRequester = new KUrlRequester( widget );
  mUrlRequester->setMode( KFile::File | KFile::ExistingOnly );
  QLabel *label = new QLabel( description(), widget );
  label->setBuddy( mUrlRequester );
  glay->addWidget( label, row, 1 );
  glay->addWidget( mUrlRequester, row, 2 );
  if ( entry->isReadOnly() ) {
    label->setEnabled( false );
    mUrlRequester->setEnabled( false );
  } else {
    connect( mUrlRequester, SIGNAL( textChanged( const QString& ) ), SLOT( slotChanged() ) );
  }
}

void Kleo::CryptoConfigEntryURL::doSave()
{
  mEntry->setURLValue( mUrlRequester->url() );
}

void Kleo::CryptoConfigEntryURL::doLoad()
{
  mUrlRequester->setUrl( mEntry->urlValue().url() );
}

////

Kleo::CryptoConfigEntrySpinBox::CryptoConfigEntrySpinBox(
  CryptoConfigModule* module,
  Kleo::CryptoConfigEntry* entry, const QString& entryName,
  QGridLayout * glay, QWidget* widget, const char* name )
  : CryptoConfigEntryGUI( module, entry, entryName, name )
{

  if ( entry->argType() == Kleo::CryptoConfigEntry::ArgType_None && entry->isList() ) {
    mKind = ListOfNone;
  } else if ( entry->argType() == Kleo::CryptoConfigEntry::ArgType_UInt ) {
    mKind = UInt;
  } else {
    Q_ASSERT( entry->argType() == Kleo::CryptoConfigEntry::ArgType_Int );
    mKind = Int;
  }

  const int row = glay->rowCount();
  mNumInput = new KIntNumInput( widget );
  QLabel *label = new QLabel( description(), widget );
  label->setBuddy( mNumInput );
  glay->addWidget( label, row, 1 );
  glay->addWidget( mNumInput, row, 2 );

  if ( entry->isReadOnly() ) {
    label->setEnabled( false );
    mNumInput->setEnabled( false );
  } else {
    if ( mKind == UInt || mKind == ListOfNone )
      mNumInput->setMinimum( 0 );
    connect( mNumInput, SIGNAL( valueChanged(int) ), SLOT( slotChanged() ) );
  }
}

void Kleo::CryptoConfigEntrySpinBox::doSave()
{
  int value = mNumInput->value();
  switch ( mKind ) {
  case ListOfNone:
    mEntry->setNumberOfTimesSet( value );
    break;
  case UInt:
    mEntry->setUIntValue( value );
    break;
  case Int:
    mEntry->setIntValue( value );
    break;
  }
}

void Kleo::CryptoConfigEntrySpinBox::doLoad()
{
  int value = 0;
  switch ( mKind ) {
  case ListOfNone:
    value = mEntry->numberOfTimesSet();
    break;
  case UInt:
    value = mEntry->uintValue();
    break;
  case Int:
    value = mEntry->intValue();
    break;
  }
  mNumInput->setValue( value );
}

////

Kleo::CryptoConfigEntryCheckBox::CryptoConfigEntryCheckBox(
  CryptoConfigModule* module,
  Kleo::CryptoConfigEntry* entry, const QString& entryName,
  QGridLayout * glay, QWidget* widget, const char* name )
  : CryptoConfigEntryGUI( module, entry, entryName, name )
{
  const int row = glay->rowCount();
  mCheckBox = new QCheckBox( widget );
  glay->addWidget( mCheckBox, row, 1, 1, 2 );
  mCheckBox->setText( description() );
  if ( entry->isReadOnly() ) {
    mCheckBox->setEnabled( false );
  } else {
    connect( mCheckBox, SIGNAL( toggled(bool) ), SLOT( slotChanged() ) );
  }
}

void Kleo::CryptoConfigEntryCheckBox::doSave()
{
  mEntry->setBoolValue( mCheckBox->isChecked() );
}

void Kleo::CryptoConfigEntryCheckBox::doLoad()
{
  mCheckBox->setChecked( mEntry->boolValue() );
}

Kleo::CryptoConfigEntryLDAPURL::CryptoConfigEntryLDAPURL(
  CryptoConfigModule* module,
  Kleo::CryptoConfigEntry* entry,
  const QString& entryName,
  QGridLayout * glay, QWidget* widget, const char* name )
  : CryptoConfigEntryGUI( module, entry, entryName, name )
{
  mLabel = new QLabel( widget );
  mPushButton = new QPushButton( i18n( "Edit..." ), widget );


  const int row = glay->rowCount();
  QLabel *label = new QLabel( description(), widget );
  label->setBuddy( mPushButton );
  glay->addWidget( label, row, 1 );
  QHBoxLayout * hlay = new QHBoxLayout;
  glay->addLayout( hlay, row, 2 );
  hlay->addWidget( mLabel, 1 );
  hlay->addWidget( mPushButton );

  if ( entry->isReadOnly() ) {
    mLabel->setEnabled( false );
    mPushButton->hide();
  } else {
    connect( mPushButton, SIGNAL( clicked() ), SLOT( slotOpenDialog() ) );
  }
}

void Kleo::CryptoConfigEntryLDAPURL::doLoad()
{
  setURLList( mEntry->urlValueList() );
}

void Kleo::CryptoConfigEntryLDAPURL::doSave()
{
  mEntry->setURLValueList( mURLList );
}

void Kleo::CryptoConfigEntryLDAPURL::slotOpenDialog()
{
  // I'm a bad boy and I do it all on the stack. Enough classes already :)
  // This is just a simple dialog around the directory-services-widget
  KDialog dialog( mPushButton->parentWidget() );
  dialog.setCaption( i18n( "Configure LDAP Servers" ) );
  dialog.setButtons( KDialog::Default|KDialog::Cancel|KDialog::Ok );
  DirectoryServicesWidget* dirserv = new DirectoryServicesWidget( mEntry, &dialog );
  dirserv->load();
  dialog.setMainWidget( dirserv );
  connect( &dialog, SIGNAL( defaultClicked() ), dirserv, SLOT( defaults() ) );
  if ( dialog.exec() ) {
    // Note that we just grab the urls from the dialog, we don't call its save method,
    // since the user hasn't confirmed the big config dialog yet.
    setURLList( dirserv->urlList() );
    slotChanged();
  }
}

void Kleo::CryptoConfigEntryLDAPURL::setURLList( const KUrl::List& urlList )
{
  mURLList = urlList;
  if ( mURLList.isEmpty() )
    mLabel->setText( i18n( "No server configured yet" ) );
  else
    mLabel->setText( i18np( "1 server configured", "%1 servers configured", mURLList.count() ) );
}

#include "cryptoconfigmodule.moc"
#include "cryptoconfigmodule_p.moc"
