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
#include "filenamerequester.h"

#include "kleo/cryptoconfig.h"

#include <klineedit.h>
#include <klocale.h>
#include <kdebug.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <kicon.h>
#ifndef ONLY_KLEO
# include <kurlrequester.h>
#endif

#include <QApplication>
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
#include <QCheckBox>
#include <QStyle>
#include <QComboBox>

#include <cassert>

using namespace Kleo;

inline KIcon loadIcon( const QString &s ) {
  QString ss = s;
  return KIcon( ss.replace( QRegExp( "[^a-zA-Z0-9_]" ), "-" ) );
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
    //kDebug(5150) <<"Component" << (*it).toLocal8Bit() <<":";
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
      new CryptoConfigComponentGUI( this, comp, scrollArea );
    compGUI->setObjectName( *it );
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
  QWidget* parent )
  : QWidget( parent ),
    mComponent( component )
{
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
      const QString title = group->description();
      KDHorizontalLine * hl = new KDHorizontalLine( title.isEmpty() ? *it : title, this );
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
  QGridLayout * glay, QWidget* widget)
  : QObject( module ), mGroup( group )
{
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
  l->setPixmap( loadIcon( iconName ).pixmap( KIconLoader::SizeMedium, KIconLoader::SizeMedium ) );
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

typedef CryptoConfigEntryGUI * (*constructor)( CryptoConfigModule *, Kleo::CryptoConfigEntry *, const QString &, QGridLayout *, QWidget * ); 

namespace {
template <typename T_Widget>
CryptoConfigEntryGUI * _create( CryptoConfigModule * m, Kleo::CryptoConfigEntry * e, const QString & n, QGridLayout * l, QWidget * p ) {
    return new T_Widget( m, e, n, l, p );
}
}

static const struct WidgetsByEntryName {
    const char * entryName;
    constructor create;
} widgetsByEntryName[] = {
    // sort by 'name' !!
    { "debug-level", &_create<CryptoConfigEntryDebugLevel> },
};
static const unsigned int numWidgetsByEntryName = sizeof widgetsByEntryName / sizeof *widgetsByEntryName;

static const constructor listWidgets[CryptoConfigEntry::NumArgType] = {
    // None: A list of options with no arguments (e.g. -v -v -v) is shown as a spinbox
    &_create<CryptoConfigEntrySpinBox>,
    0, // String
    // Int/UInt: Let people type list of numbers (1,2,3....). Untested.
    &_create<CryptoConfigEntryLineEdit>,
    &_create<CryptoConfigEntryLineEdit>,
    0, // Path
    0, // URL
    &_create<CryptoConfigEntryLDAPURL>,
    0, // DirPath
};

static const constructor scalarWidgets[CryptoConfigEntry::NumArgType] = {
  &_create<CryptoConfigEntryCheckBox>, // None
  &_create<CryptoConfigEntryLineEdit>, // String
  &_create<CryptoConfigEntrySpinBox>,  // Int
  &_create<CryptoConfigEntrySpinBox>,  // UInt
  &_create<CryptoConfigEntryPath>,     // Path
  &_create<CryptoConfigEntryURL>,      // URL
  0,                                   // LDAPURL
  &_create<CryptoConfigEntryDirPath>,  // DirPath
};

CryptoConfigEntryGUI* Kleo::CryptoConfigEntryGUIFactory::createEntryGUI( CryptoConfigModule* module, Kleo::CryptoConfigEntry* entry, const QString& entryName, QGridLayout * glay, QWidget* widget )
{
    assert( entry );

    // try to lookup by name:
    for ( unsigned int i = 0 ; i < numWidgetsByEntryName ; ++i )
        if ( entryName == QLatin1String( widgetsByEntryName[i].entryName ) )
            return widgetsByEntryName[i].create( module, entry, entryName, glay, widget );

    // none found, so look up by type:
    const unsigned int argType = entry->argType();
    assert( argType < CryptoConfigEntry::NumArgType );
    if ( entry->isList() )
        if ( const constructor create = listWidgets[argType] )
            return create( module, entry, entryName, glay, widget );
        else
            kWarning(5150) <<"No widget implemented for list of type" << entry->argType();
    else
        if ( const constructor create = scalarWidgets[argType] )
            return create( module, entry, entryName, glay, widget );
        else
            kWarning(5150) <<"No widget implemented for type" << entry->argType();

    return 0;
}

////

Kleo::CryptoConfigEntryGUI::CryptoConfigEntryGUI(
  CryptoConfigModule* module,
  Kleo::CryptoConfigEntry* entry,
  const QString& entryName )
  : QObject( module ), mEntry( entry ), mName( entryName ), mChanged( false )
{
  connect( this, SIGNAL( changed() ), module, SIGNAL( changed() ) );
}

QString Kleo::CryptoConfigEntryGUI::description() const
{
    QString descr = mEntry->description();
    if ( descr.isEmpty() ) // shouldn't happen
        return QString( "<%1>" ).arg( mName );
    if ( i18nc( "Translate this to 'yes' or 'no' (use the English words!) "
                "depending on whether your language uses "
                "Sentence style capitalisation in GUI labels (yes) or not (no). "
                "Context: We get some backend strings in that have the wrong "
                "capitalizaion (in English, at least) so we need to force the "
                "first character to upper-case. It is this behaviour you can "
                "control for your language with this translation.", "yes" ) == QLatin1String( "yes" ) )
        descr[0] = descr[0].toUpper();
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
  QGridLayout * glay, QWidget* widget )
  : CryptoConfigEntryGUI( module, entry, entryName )
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

static const struct {
    const char * label;
    const char * name;
} debugLevels[] = {
    { I18N_NOOP( "None (no debugging at all)" ),               "none"     },
    { I18N_NOOP( "Basic (some basic debug messages)" ),        "basic"    },
    { I18N_NOOP( "Advanced (more verbose debug messages)" ),   "advanced" },
    { I18N_NOOP( "Expert (even more detailed messages)" ),     "expert"   },
    { I18N_NOOP( "Guru (all of the debug messages you can get)" ), "guru" },
};
static const unsigned int numDebugLevels = sizeof debugLevels / sizeof *debugLevels;

Kleo::CryptoConfigEntryDebugLevel::CryptoConfigEntryDebugLevel( CryptoConfigModule * module, Kleo::CryptoConfigEntry * entry,
                                                                const QString & entryName, QGridLayout * glay, QWidget * widget )
    : CryptoConfigEntryGUI( module, entry, entryName ),
      mComboBox( new QComboBox( widget ) )
{
    QLabel *label = new QLabel( i18n("Set the debugging level to"), widget );
    label->setBuddy( mComboBox );

    for ( unsigned int i = 0 ; i < numDebugLevels ; ++i )
        mComboBox->addItem( i18n( debugLevels[i].label ) );

    if ( entry->isReadOnly() ) {
        label->setEnabled( false );
        mComboBox->setEnabled( false );
    } else {
        connect( mComboBox, SIGNAL(currentIndexChanged(int)), SLOT(slotChanged()) );
    }

    const int row = glay->rowCount();
    glay->addWidget( label, row, 1 );
    glay->addWidget( mComboBox, row, 2 );
}

void Kleo::CryptoConfigEntryDebugLevel::doSave()
{
    const unsigned int idx = mComboBox->currentIndex();
    if ( idx < numDebugLevels )
        mEntry->setStringValue( QLatin1String( debugLevels[idx].name ) );
    else
        mEntry->setStringValue( QString() );
}

void Kleo::CryptoConfigEntryDebugLevel::doLoad()
{
    const QString str = mEntry->stringValue();
    for ( unsigned int i = 0 ; i < numDebugLevels ; ++i )
        if ( str == QLatin1String( debugLevels[i].name ) ) {
            mComboBox->setCurrentIndex( i );
            return;
        }
    mComboBox->setCurrentIndex( 0 );
}

////

Kleo::CryptoConfigEntryPath::CryptoConfigEntryPath(
  CryptoConfigModule* module,
  Kleo::CryptoConfigEntry* entry, const QString& entryName,
  QGridLayout * glay, QWidget* widget )
    : CryptoConfigEntryGUI( module, entry, entryName ),
      mUrlRequester( 0 ),
      mFileNameRequester( 0 )
{
  const int row = glay->rowCount();
  QWidget * req;
#ifdef ONLY_KLEO
  req = mFileNameRequester = new FileNameRequester( widget );
  mFileNameRequester->setExistingOnly( false );
  mFileNameRequester->setFilter( QDir::Files );
#else
  req = mUrlRequester = new KUrlRequester( widget );
  mUrlRequester->setMode( KFile::File | KFile::LocalOnly );
#endif
  QLabel *label = new QLabel( description(), widget );
  label->setBuddy( req );
  glay->addWidget( label, row, 1 );
  glay->addWidget( req, row, 2 );
  if ( entry->isReadOnly() ) {
    label->setEnabled( false );
#ifndef ONLY_KLEO
    if ( mUrlRequester )
        mUrlRequester->setEnabled( false );
#endif
    if ( mFileNameRequester )
        mFileNameRequester->setEnabled( false );
  } else {
#ifndef ONLY_KLEO
	  if ( mUrlRequester )
          connect( mUrlRequester, SIGNAL(textChanged(QString)),
                   this, SLOT(slotChanged()) );
#endif
      if ( mFileNameRequester )
          connect( mFileNameRequester, SIGNAL(fileNameChanged(QString)),
                   this, SLOT(slotChanged()) );
  }
}

void Kleo::CryptoConfigEntryPath::doSave()
{
#ifdef ONLY_KLEO
  mEntry->setURLValue( KUrl::fromPath( mFileNameRequester->fileName() ) );
#else
  mEntry->setURLValue( mUrlRequester->url() );
#endif
}

void Kleo::CryptoConfigEntryPath::doLoad()
{
#ifdef ONLY_KLEO
  mFileNameRequester->setFileName( mEntry->urlValue().path() );
#else
  mUrlRequester->setUrl( mEntry->urlValue() );
#endif
}

////

Kleo::CryptoConfigEntryDirPath::CryptoConfigEntryDirPath(
  CryptoConfigModule* module,
  Kleo::CryptoConfigEntry* entry, const QString& entryName,
  QGridLayout * glay, QWidget* widget )
    : CryptoConfigEntryGUI( module, entry, entryName ),
      mUrlRequester( 0 ),
      mFileNameRequester( 0 )
{
  const int row = glay->rowCount();
  QWidget * req;
#ifdef ONLY_KLEO
  req = mFileNameRequester = new FileNameRequester( widget );
  mFileNameRequester->setExistingOnly( false );
  mFileNameRequester->setFilter( QDir::Dirs );
#else
  req = mUrlRequester = new KUrlRequester( widget );
  mUrlRequester->setMode( KFile::Directory | KFile::LocalOnly );
#endif
  QLabel *label = new QLabel( description(), widget );
  label->setBuddy( req );
  glay->addWidget( label, row, 1 );
  glay->addWidget( req, row, 2 );
  if ( entry->isReadOnly() ) {
    label->setEnabled( false );
#ifndef ONLY_KLEO
    if ( mUrlRequester )
        mUrlRequester->setEnabled( false );
#endif
    if ( mFileNameRequester )
        mFileNameRequester->setEnabled( false );
  } else {
#ifndef ONLY_KLEO
      if ( mUrlRequester )
          connect( mUrlRequester, SIGNAL(textChanged(QString)),
                   this, SLOT(slotChanged()) );
#endif
      if ( mFileNameRequester )
          connect( mFileNameRequester, SIGNAL(fileNameChanged(QString)),
                   this, SLOT(slotChanged()) );
  }
}

void Kleo::CryptoConfigEntryDirPath::doSave()
{
#ifdef ONLY_KLEO
  mEntry->setURLValue( KUrl::fromPath( mFileNameRequester->fileName() ) );
#else
  mEntry->setURLValue( mUrlRequester->url() );
#endif
}

void Kleo::CryptoConfigEntryDirPath::doLoad()
{
#ifdef ONLY_KLEO
  mFileNameRequester->setFileName( mEntry->urlValue().path() );
#else
  mUrlRequester->setUrl( mEntry->urlValue() );
#endif
}

////

Kleo::CryptoConfigEntryURL::CryptoConfigEntryURL(
  CryptoConfigModule* module,
  Kleo::CryptoConfigEntry* entry, const QString& entryName,
  QGridLayout * glay, QWidget* widget )
    : CryptoConfigEntryGUI( module, entry, entryName ),
#ifndef ONLY_KLEO
      mUrlRequester( 0 ),
#endif
      mLineEdit( 0 )
{
  const int row = glay->rowCount();
  QWidget * req;
#ifdef ONLY_KLEO
  req = mLineEdit = new QLineEdit( widget );
#else
  req = mUrlRequester = new KUrlRequester( widget );
  mUrlRequester->setMode( KFile::File | KFile::ExistingOnly );
#endif
  QLabel *label = new QLabel( description(), widget );
  label->setBuddy( req );
  glay->addWidget( label, row, 1 );
  glay->addWidget( req, row, 2 );
  if ( entry->isReadOnly() ) {
    label->setEnabled( false );
#ifndef ONLY_KLEO
    if ( mUrlRequester )
        mUrlRequester->setEnabled( false );
#endif
    if ( mLineEdit )
        mLineEdit->setEnabled( false );
  } else {
      connect( req, SIGNAL(textChanged(QString)),
               this, SLOT(slotChanged()) );
  }
}

void Kleo::CryptoConfigEntryURL::doSave()
{
#ifdef ONLY_KLEO
  mEntry->setURLValue( KUrl( mLineEdit->text() ) );
#else
  mEntry->setURLValue( mUrlRequester->url() );
#endif
}

void Kleo::CryptoConfigEntryURL::doLoad()
{
#ifdef ONLY_KLEO
  mLineEdit->setText( mEntry->urlValue().url() );
#else
  mUrlRequester->setUrl( mEntry->urlValue().url() );
#endif
}

////

Kleo::CryptoConfigEntrySpinBox::CryptoConfigEntrySpinBox(
  CryptoConfigModule* module,
  Kleo::CryptoConfigEntry* entry, const QString& entryName,
  QGridLayout * glay, QWidget* widget )
  : CryptoConfigEntryGUI( module, entry, entryName )
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
  QGridLayout * glay, QWidget* widget )
  : CryptoConfigEntryGUI( module, entry, entryName )
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
  QGridLayout * glay, QWidget* widget )
  : CryptoConfigEntryGUI( module, entry, entryName )
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
  DirectoryServicesWidget* dirserv = new DirectoryServicesWidget( &dialog );
  dirserv->setAllowedSchemes( DirectoryServicesWidget::LDAP );
  dirserv->setAllowedProtocols( DirectoryServicesWidget::X509Protocol );
  dirserv->addX509Services( mURLList );
  dialog.setMainWidget( dirserv );
  connect( &dialog, SIGNAL( defaultClicked() ), dirserv, SLOT( defaults() ) );
  if ( dialog.exec() ) {
    // Note that we just grab the urls from the dialog, we don't call its save method,
    // since the user hasn't confirmed the big config dialog yet.
    setURLList( dirserv->x509Services() );
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
