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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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
#include "directoryserviceswidget.h"
#include <kleo/cryptoconfig.h>

#include <klineedit.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <kdebug.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kglobal.h>

#include <qgrid.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qregexp.h>

using namespace Kleo;

inline QPixmap loadIcon( QString s ) {
  return KGlobal::instance()->iconLoader()
    ->loadIcon( s.replace( QRegExp( "[^a-zA-Z0-9_]" ), "_" ), KIcon::NoGroup, KIcon::SizeMedium );
}

static const KJanusWidget::Face determineJanusFace( const Kleo::CryptoConfig * config ) {
  return config && config->componentList().size() < 2
    ? KJanusWidget::Plain
    : KJanusWidget::IconList ;
}

Kleo::CryptoConfigModule::CryptoConfigModule( Kleo::CryptoConfig* config, QWidget * parent, const char * name )
  : KJanusWidget( parent, name, determineJanusFace( config ) ), mConfig( config )
{
//  QVBoxLayout *vlay = new QVBoxLayout( this, 0, KDialog::spacingHint() );
//  mTabWidget = new QTabWidget( this );
//  vlay->addWidget( mTabWidget );

  const QStringList components = config->componentList();

  QWidget * vbox = 0;
  if ( face() == Plain ) {
    vbox = plainPage();
    QVBoxLayout * vlay = new QVBoxLayout( vbox, 0, KDialog::spacingHint() );
    vlay->setAutoAdd( true );
  }

  for( QStringList::const_iterator compit = components.begin(); compit != components.end(); ++compit ) {
    //kdDebug(5150) << "Component " << (*compit).local8Bit() << ":" << endl;
    Kleo::CryptoConfigComponent* comp = config->component( *compit );
    Q_ASSERT( comp );
    if ( comp->groupList().empty() )
      continue;
    if ( face() != Plain )
    vbox = addVBoxPage( comp->description(), QString::null, loadIcon( *compit ) );
    CryptoConfigComponentGUI* compGUI =
      new CryptoConfigComponentGUI( this, comp, vbox, (*compit).local8Bit() );
    // KJanusWidget doesn't seem to have iterators, so we store a copy...
    mComponentGUIs.append( compGUI );
  }
}

void Kleo::CryptoConfigModule::save()
{
  bool changed = false;
  QValueList<CryptoConfigComponentGUI *>::Iterator it = mComponentGUIs.begin();
  for( ; it != mComponentGUIs.end(); ++it ) {
    if ( (*it)->save() )
      changed = true;
  }
  if ( changed )
    mConfig->sync(true /*runtime*/);
}

void Kleo::CryptoConfigModule::reset()
{
  QValueList<CryptoConfigComponentGUI *>::Iterator it = mComponentGUIs.begin();
  for( ; it != mComponentGUIs.end(); ++it ) {
    (*it)->load();
  }
}

void Kleo::CryptoConfigModule::defaults()
{
  QValueList<CryptoConfigComponentGUI *>::Iterator it = mComponentGUIs.begin();
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
#ifdef USE_TABS // Old idea, dead code
  : QTabWidget( parent, name ),
#else
  : QWidget( parent, name ),
#endif
    mComponent( component )
{
#ifndef USE_TABS
  QVBoxLayout *vlay = new QVBoxLayout( this, 0, KDialog::spacingHint() );
#endif

  QStringList groups = mComponent->groupList();
  for( QStringList::Iterator groupit = groups.begin(); groupit != groups.end(); ++groupit ) {
    Kleo::CryptoConfigGroup* group = mComponent->group( *groupit );
    Q_ASSERT( group );
    CryptoConfigGroupGUI* gg = new CryptoConfigGroupGUI( module, group, this );
#ifdef USE_TABS
    addTab( gg, group->description() );
#else
    vlay->addWidget( gg );
#endif
    mGroupGUIs.append( gg );
  }
#ifndef USE_TABS
  vlay->addStretch( 1 );
#endif
}


bool Kleo::CryptoConfigComponentGUI::save()
{
  bool changed = false;
  QValueList<CryptoConfigGroupGUI *>::Iterator it = mGroupGUIs.begin();
  for( ; it != mGroupGUIs.end(); ++it ) {
    if ( (*it)->save() )
      changed = true;
  }
  return changed;
}

void Kleo::CryptoConfigComponentGUI::load()
{
  QValueList<CryptoConfigGroupGUI *>::Iterator it = mGroupGUIs.begin();
  for( ; it != mGroupGUIs.end(); ++it )
    (*it)->load();
}

void Kleo::CryptoConfigComponentGUI::defaults()
{
  QValueList<CryptoConfigGroupGUI *>::Iterator it = mGroupGUIs.begin();
  for( ; it != mGroupGUIs.end(); ++it )
    (*it)->defaults();
}

////

Kleo::CryptoConfigGroupGUI::CryptoConfigGroupGUI(
  CryptoConfigModule* module, Kleo::CryptoConfigGroup* group,
  QWidget* parent, const char* name )
  : QGroupBox( 1, Qt::Horizontal, // yeah that means a vertical layout...
               group->description(), parent, name ), mGroup( group )
{
  QStringList entries = mGroup->entryList();
  for( QStringList::Iterator entryit = entries.begin(); entryit != entries.end(); ++entryit ) {
    Kleo::CryptoConfigEntry* entry = group->entry( *entryit );
    Q_ASSERT( entry );
    CryptoConfigEntryGUI* entryGUI =
      CryptoConfigEntryGUIFactory::createEntryGUI( module, entry, *entryit, this );
    if ( entryGUI ) {
      mEntryGUIs.append( entryGUI );
      entryGUI->load();
    }
  }
}

bool Kleo::CryptoConfigGroupGUI::save()
{
  bool changed = false;
  QValueList<CryptoConfigEntryGUI *>::Iterator it = mEntryGUIs.begin();
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
  QValueList<CryptoConfigEntryGUI *>::Iterator it = mEntryGUIs.begin();
  for( ; it != mEntryGUIs.end(); ++it )
    (*it)->load();
}

void Kleo::CryptoConfigGroupGUI::defaults()
{
  QValueList<CryptoConfigEntryGUI *>::Iterator it = mEntryGUIs.begin();
  for( ; it != mEntryGUIs.end(); ++it )
    (*it)->resetToDefault();
}

////

CryptoConfigEntryGUI* Kleo::CryptoConfigEntryGUIFactory::createEntryGUI( CryptoConfigModule* module, Kleo::CryptoConfigEntry* entry, const QString& entryName, QWidget* parent, const char* name )
{
  if ( entry->isList() ) {
    switch( entry->argType() ) {
    case Kleo::CryptoConfigEntry::ArgType_None:
      // A list of options with no arguments (e.g. -v -v -v) is shown as a spinbox
      return new CryptoConfigEntrySpinBox( module, entry, entryName, parent, name );
    case Kleo::CryptoConfigEntry::ArgType_Int:
    case Kleo::CryptoConfigEntry::ArgType_UInt:
      // Let people type list of numbers (1,2,3....). Untested.
      return new CryptoConfigEntryLineEdit( module, entry, entryName, parent, name );
    case Kleo::CryptoConfigEntry::ArgType_URL:
    case Kleo::CryptoConfigEntry::ArgType_Path:
    case Kleo::CryptoConfigEntry::ArgType_String:
      kdWarning(5150) << "No widget implemented for list of type " << entry->argType() << endl;
      return 0; // TODO when the need arises :)
    case Kleo::CryptoConfigEntry::ArgType_LDAPURL:
      return new CryptoConfigEntryLDAPURL( module, entry, entryName, parent, name );
    }
  }

  switch( entry->argType() ) {
  case Kleo::CryptoConfigEntry::ArgType_None:
    return new CryptoConfigEntryCheckBox( module, entry, entryName, parent, name );
  case Kleo::CryptoConfigEntry::ArgType_Int:
    // fallthrough
  case Kleo::CryptoConfigEntry::ArgType_UInt:
    return new CryptoConfigEntrySpinBox( module, entry, entryName, parent, name );
  case Kleo::CryptoConfigEntry::ArgType_LDAPURL:
    // TODO when the need arises
  case Kleo::CryptoConfigEntry::ArgType_URL:
    // fallthrough
  case Kleo::CryptoConfigEntry::ArgType_Path:
    // fallthrough
  case Kleo::CryptoConfigEntry::ArgType_String:
    return new CryptoConfigEntryLineEdit( module, entry, entryName, parent, name );
  }
  kdWarning(5150) << "No widget implemented for list of (unknown) type " << entry->argType() << endl;
  return 0;
}

////

Kleo::CryptoConfigEntryGUI::CryptoConfigEntryGUI(
  CryptoConfigModule* module,
  Kleo::CryptoConfigEntry* entry,
  const QString& entryName,
  QWidget* parent, const char* name )
  : QHBox( parent, name ), mEntry( entry ), mName( entryName ), mChanged( false )
{
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
  QWidget* parent, const char* name )
  : CryptoConfigEntryGUI( module, entry, entryName, parent, name )
{
  setSpacing( KDialog::spacingHint() );
  QLabel* label = new QLabel( description(), this );
  mLineEdit = new KLineEdit( this );
  connect( mLineEdit, SIGNAL( textChanged( const QString& ) ), SLOT( slotChanged() ) );
  label->setBuddy( mLineEdit );
  QWidget* stretch = new QWidget( this );
  setStretchFactor( stretch, 1 );
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

Kleo::CryptoConfigEntrySpinBox::CryptoConfigEntrySpinBox(
  CryptoConfigModule* module,
  Kleo::CryptoConfigEntry* entry, const QString& entryName,
  QWidget* parent, const char* name )
  : CryptoConfigEntryGUI( module, entry, entryName, parent, name )
{
  setSpacing( KDialog::spacingHint() );
  QLabel* label = new QLabel( description(), this );

  if ( entry->argType() == Kleo::CryptoConfigEntry::ArgType_None && entry->isList() ) {
    mKind = ListOfNone;
  } else if ( entry->argType() == Kleo::CryptoConfigEntry::ArgType_UInt ) {
    mKind = UInt;
  } else {
    Q_ASSERT( entry->argType() == Kleo::CryptoConfigEntry::ArgType_Int );
    mKind = Int;
  }

  mNumInput = new KIntNumInput( this );
  if ( mKind == UInt || mKind == ListOfNone )
    mNumInput->setMinValue( 0 );
  connect( mNumInput, SIGNAL( valueChanged(int) ), SLOT( slotChanged() ) );
  label->setBuddy( mNumInput );

  QWidget* stretch = new QWidget( this );
  setStretchFactor( stretch, 1 );
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
  QWidget* parent, const char* name )
  : CryptoConfigEntryGUI( module, entry, entryName, parent, name )
{
  mCheckBox = new QCheckBox( this);
  mCheckBox->setText( description() );
  connect( mCheckBox, SIGNAL( toggled(bool) ), SLOT( slotChanged() ) );
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
  QWidget* parent, const char* name )
  : CryptoConfigEntryGUI( module, entry, entryName, parent, name )
{
  setSpacing( KDialog::spacingHint() );
  QLabel* label = new QLabel( description(), this );
  mPushButton = new QPushButton( i18n( "Edit..." ), this );
  mLabel = new QLabel( this );
  connect( mPushButton, SIGNAL( clicked() ), SLOT( slotOpenDialog() ) );
  label->setBuddy( mPushButton );
  QWidget* stretch = new QWidget( this );
  setStretchFactor( stretch, 1 );
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
  KDialogBase dialog( this, 0, true /*modal*/,
                      i18n( "Configure LDAP Servers" ),
                      KDialogBase::Default|KDialogBase::Cancel|KDialogBase::Ok,
                      KDialogBase::Ok, true /*separator*/ );
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

void Kleo::CryptoConfigEntryLDAPURL::setURLList( const KURL::List& urlList )
{
  mURLList = urlList;
  if ( mURLList.isEmpty() )
    mLabel->setText( i18n( "No server configured yet" ) );
  else
    mLabel->setText( i18n( "1 server configured", "%n servers configured", mURLList.count() ) );
}

#include "cryptoconfigmodule.moc"
