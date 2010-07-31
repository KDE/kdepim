#ifndef KDE_USE_FINAL
#define QT_NO_CAST_ASCII
#endif
// configuredialog_p.cpp: classes internal to ConfigureDialog
// see configuredialog.cpp for details.

// This must be first
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// my header:
#include "configuredialog_p.h"

// other KMail headers:
#include "kmtransport.h"
#include "globalsettings.h"
#include "kmacctcachedimap.h"

// other kdenetwork headers: (none)

// other KDE headers:
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>

// Qt headers:
#include <tqheader.h>
#include <tqtabwidget.h>
#include <tqradiobutton.h>
#include <tqbuttongroup.h>
#include <tqlabel.h>
#include <tqlayout.h>

// Other headers:
#include <assert.h>


NewIdentityDialog::NewIdentityDialog( const TQStringList & identities,
				      TQWidget *parent, const char *name,
				      bool modal )
  : KDialogBase( parent, name, modal, i18n("New Identity"),
		 Ok|Cancel|Help, Ok, true )
{
  setHelp( TQString::fromLatin1("configure-identity-newidentitydialog") );
  TQWidget * page = makeMainWidget();
  TQVBoxLayout * vlay = new TQVBoxLayout( page, 0, spacingHint() );

  // row 0: line edit with label
  TQHBoxLayout * hlay = new TQHBoxLayout( vlay ); // inherits spacing
  mLineEdit = new KLineEdit( page );
  mLineEdit->setFocus();
  hlay->addWidget( new TQLabel( mLineEdit, i18n("&New identity:"), page ) );
  hlay->addWidget( mLineEdit, 1 );
  connect( mLineEdit, TQT_SIGNAL(textChanged(const TQString&)),
	   this, TQT_SLOT(slotEnableOK(const TQString&)) );

  mButtonGroup = new TQButtonGroup( page );
  mButtonGroup->hide();

  // row 1: radio button
  TQRadioButton *radio = new TQRadioButton( i18n("&With empty fields"), page );
  radio->setChecked( true );
  mButtonGroup->insert( radio, Empty );
  vlay->addWidget( radio );

  // row 2: radio button
  radio = new TQRadioButton( i18n("&Use Control Center settings"), page );
  mButtonGroup->insert( radio, ControlCenter );
  vlay->addWidget( radio );

  // row 3: radio button
  radio = new TQRadioButton( i18n("&Duplicate existing identity"), page );
  mButtonGroup->insert( radio, ExistingEntry );
  vlay->addWidget( radio );

  // row 4: combobox with existing identities and label
  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  mComboBox = new TQComboBox( false, page );
  mComboBox->insertStringList( identities );
  mComboBox->setEnabled( false );
  TQLabel *label = new TQLabel( mComboBox, i18n("&Existing identities:"), page );
  label->setEnabled( false );
  hlay->addWidget( label );
  hlay->addWidget( mComboBox, 1 );

  vlay->addStretch( 1 ); // spacer

  // enable/disable combobox and label depending on the third radio
  // button's state:
  connect( radio, TQT_SIGNAL(toggled(bool)),
	   label, TQT_SLOT(setEnabled(bool)) );
  connect( radio, TQT_SIGNAL(toggled(bool)),
	   mComboBox, TQT_SLOT(setEnabled(bool)) );

  enableButtonOK( false ); // since line edit is empty
}

NewIdentityDialog::DuplicateMode NewIdentityDialog::duplicateMode() const {
  int id = mButtonGroup->id( mButtonGroup->selected() );
  assert( id == (int)Empty
	  || id == (int)ControlCenter
	  || id == (int)ExistingEntry );
  return static_cast<DuplicateMode>( id );
}

void NewIdentityDialog::slotEnableOK( const TQString & proposedIdentityName ) {
  // OK button is disabled if
  TQString name = proposedIdentityName.stripWhiteSpace();
  // name isn't empty
  if ( name.isEmpty() ) {
    enableButtonOK( false );
    return;
  }
  // or name doesn't yet exist.
  for ( int i = 0 ; i < mComboBox->count() ; i++ )
    if ( mComboBox->text(i) == name ) {
      enableButtonOK( false );
      return;
    }
  enableButtonOK( true );
}

ListView::ListView( TQWidget *parent, const char *name,
				     int visibleItem )
  : KListView( parent, name )
{
  setVisibleItem(visibleItem);
}


void ListView::resizeEvent( TQResizeEvent *e )
{
  KListView::resizeEvent(e);
  resizeColums();
}


void ListView::showEvent( TQShowEvent *e )
{
  KListView::showEvent(e);
  resizeColums();
}


void ListView::resizeColums()
{
  int c = columns();
  if( c == 0 )
  {
    return;
  }

  int w1 = viewport()->width();
  int w2 = w1 / c;
  int w3 = w1 - (c-1)*w2;

  for( int i=0; i<c-1; i++ )
  {
    setColumnWidth( i, w2 );
  }
  setColumnWidth( c-1, w3 );
}


void ListView::setVisibleItem( int visibleItem, bool updateSize )
{
  mVisibleItem = QMAX( 1, visibleItem );
  if( updateSize == true )
  {
    TQSize s = sizeHint();
    setMinimumSize( s.width() + verticalScrollBar()->sizeHint().width() +
		    lineWidth() * 2, s.height() );
  }
}


TQSize ListView::sizeHint() const
{
  TQSize s = TQListView::sizeHint();

  int h = fontMetrics().height() + 2*itemMargin();
  if( h % 2 > 0 ) { h++; }

  s.setHeight( h*mVisibleItem + lineWidth()*2 + header()->sizeHint().height());
  return s;
}


static TQString flagPng = TQString::fromLatin1("/flag.png");

NewLanguageDialog::NewLanguageDialog( LanguageItemList & suppressedLangs,
				      TQWidget *parent, const char *name,
				      bool modal )
  : KDialogBase( parent, name, modal, i18n("New Language"), Ok|Cancel, Ok, true )
{
  // layout the page (a combobox with label):
  TQWidget *page = makeMainWidget();
  TQHBoxLayout *hlay = new TQHBoxLayout( page, 0, spacingHint() );
  mComboBox = new TQComboBox( false, page );
  hlay->addWidget( new TQLabel( mComboBox, i18n("Choose &language:"), page ) );
  hlay->addWidget( mComboBox, 1 );

  TQStringList pathList = KGlobal::dirs()->findAllResources( "locale",
                               TQString::fromLatin1("*/entry.desktop") );
  // extract a list of language tags that should not be included:
  TQStringList suppressedAcronyms;
  for ( LanguageItemList::Iterator lit = suppressedLangs.begin();
	lit != suppressedLangs.end(); ++lit )
    suppressedAcronyms << (*lit).mLanguage;

  // populate the combo box:
  for ( TQStringList::ConstIterator it = pathList.begin();
	it != pathList.end(); ++it )
  {
    KSimpleConfig entry( *it );
    entry.setGroup( "KCM Locale" );
    // full name:
    TQString name = entry.readEntry( "Name" );
    // {2,3}-letter abbreviation:
    // we extract it from the path: "/prefix/de/entry.desktop" -> "de"
    TQString acronym = (*it).section( '/', -2, -2 );

    if ( suppressedAcronyms.find( acronym ) == suppressedAcronyms.end() ) {
      // not found:
      TQString displayname = TQString::fromLatin1("%1 (%2)")
	.arg( name ).arg( acronym );
      TQPixmap flag( locate("locale", acronym + flagPng ) );
      mComboBox->insertItem( flag, displayname );
    }
  }
  if ( !mComboBox->count() ) {
    mComboBox->insertItem( i18n("No More Languages Available") );
    enableButtonOK( false );
  } else mComboBox->listBox()->sort();
}

TQString NewLanguageDialog::language() const
{
  TQString s = mComboBox->currentText();
  int i = s.findRev( '(' );
  return s.mid( i + 1, s.length() - i - 2 );
}


LanguageComboBox::LanguageComboBox( bool rw, TQWidget *parent, const char *name )
  : TQComboBox( rw, parent, name )
{
}

int LanguageComboBox::insertLanguage( const TQString & language )
{
  static TQString entryDesktop = TQString::fromLatin1("/entry.desktop");
  KSimpleConfig entry( locate("locale", language + entryDesktop) );
  entry.setGroup( "KCM Locale" );
  TQString name = entry.readEntry( "Name" );
  TQString output = TQString::fromLatin1("%1 (%2)").arg( name ).arg( language );
  insertItem( TQPixmap( locate("locale", language + flagPng ) ), output );
  return listBox()->index( listBox()->findItem(output) );
}

TQString LanguageComboBox::language() const
{
  TQString s = currentText();
  int i = s.findRev( '(' );
  return s.mid( i + 1, s.length() - i - 2 );
}

void LanguageComboBox::setLanguage( const TQString & language )
{
  TQString parenthizedLanguage = TQString::fromLatin1("(%1)").arg( language );
  for (int i = 0; i < count(); i++)
    // ### FIXME: use .endWith():
    if ( text(i).find( parenthizedLanguage ) >= 0 ) {
      setCurrentItem(i);
      return;
    }
}

//
//
//  ProfileDialog
//
//

ProfileDialog::ProfileDialog( TQWidget * parent, const char * name, bool modal )
  : KDialogBase( parent, name, modal, i18n("Load Profile"), Ok|Cancel, Ok, true )
{
  // tmp. vars:
  TQWidget * page = makeMainWidget();
  TQVBoxLayout * vlay = new TQVBoxLayout( page, 0, spacingHint() );

  mListView = new KListView( page, "mListView" );
  mListView->addColumn( i18n("Available Profiles") );
  mListView->addColumn( i18n("Description") );
  mListView->setFullWidth( true );
  mListView->setAllColumnsShowFocus( true );
  mListView->setSorting( -1 );

  vlay->addWidget( new TQLabel( mListView,
			       i18n("&Select a profile and click 'OK' to "
				    "load its settings:"), page ) );
  vlay->addWidget( mListView, 1 );

  setup();

  connect( mListView, TQT_SIGNAL(selectionChanged()),
	   TQT_SLOT(slotSelectionChanged()) );
  connect( mListView, TQT_SIGNAL(doubleClicked ( TQListViewItem *, const TQPoint &, int ) ),
	   TQT_SLOT(slotOk()) );

  connect( this, TQT_SIGNAL(finished()), TQT_SLOT(delayedDestruct()) );

  enableButtonOK( false );
}

void ProfileDialog::slotSelectionChanged()
{
  enableButtonOK( mListView->selectedItem() );
}

void ProfileDialog::setup() {
  mListView->clear();
  // find all profiles (config files named "profile-xyz-rc"):
  const TQString profileFilenameFilter = TQString::fromLatin1("kmail/profile-*-rc");
  mProfileList = KGlobal::dirs()->findAllResources( "data", profileFilenameFilter );

  kdDebug(5006) << "Profile manager: found " << mProfileList.count()
		<< " profiles:" << endl;

  // build the list and populate the list view:
  TQListViewItem * listItem = 0;
  for ( TQStringList::const_iterator it = mProfileList.begin() ;
	it != mProfileList.end() ; ++it ) {
    KConfig profile( *it, true /* read-only */, false /* no KDE global */ );
    profile.setGroup("KMail Profile");
    TQString name = profile.readEntry( "Name" );
    if ( name.isEmpty() ) {
      kdWarning(5006) << "File \"" << (*it)
		      << "\" doesn't provide a profile name!" << endl;
      name = i18n("Missing profile name placeholder","Unnamed");
    }
    TQString desc = profile.readEntry( "Comment" );
    if ( desc.isEmpty() ) {
      kdWarning(5006) << "File \"" << (*it)
		      << "\" doesn't provide a description!" << endl;
      desc = i18n("Missing profile description placeholder","Not available");
    }
    listItem = new TQListViewItem( mListView, listItem, name, desc );
  }
}

void ProfileDialog::slotOk() {
  const int index = mListView->itemIndex( mListView->selectedItem() );
  if ( index < 0 )
    return; // none selected

  assert( (unsigned int)index < mProfileList.count() );

  KConfig profile( *mProfileList.at(index), true, false );
  emit profileSelected( &profile );
  KDialogBase::slotOk();
}


ConfigModuleWithTabs::ConfigModuleWithTabs( TQWidget * parent,
						  const char * name )
  : ConfigModule( parent, name )
{
  TQVBoxLayout *vlay = new TQVBoxLayout( this, 0, KDialog::spacingHint() );
  mTabWidget = new TQTabWidget( this );
  vlay->addWidget( mTabWidget );
}

void ConfigModuleWithTabs::addTab( ConfigModuleTab* tab, const TQString & title ) {
  mTabWidget->addTab( tab, title );
  connect( tab, TQT_SIGNAL(changed( bool )),
	        this, TQT_SIGNAL(changed( bool )) );
}

void ConfigModuleWithTabs::load() {
  for ( int i = 0 ; i < mTabWidget->count() ; ++i ) {
    ConfigModuleTab *tab = dynamic_cast<ConfigModuleTab*>( mTabWidget->page(i) );
    if ( tab )
      tab->load();
  }
  KCModule::load();
}

void ConfigModuleWithTabs::save() {
  KCModule::save();
   for ( int i = 0 ; i < mTabWidget->count() ; ++i ) {
    ConfigModuleTab *tab = dynamic_cast<ConfigModuleTab*>( mTabWidget->page(i) );
    if ( tab )
      tab->save();
  }
}

void ConfigModuleWithTabs::defaults() {
  ConfigModuleTab *tab = dynamic_cast<ConfigModuleTab*>( mTabWidget->currentPage() );
  if ( tab )
    tab->defaults();
  KCModule::defaults();
}

void ConfigModuleWithTabs::installProfile(KConfig * /* profile */ ) {
  for ( int i = 0 ; i < mTabWidget->count() ; ++i ) {
    ConfigModuleTab *tab = dynamic_cast<ConfigModuleTab*>( mTabWidget->page(i) );
    if ( tab )
      tab->installProfile();
  }
}

void ConfigModuleTab::load()
{
  doLoadFromGlobalSettings();
  doLoadOther();
}

void ConfigModuleTab::defaults()
{
  // reset settings which are available via GlobalSettings to their defaults
  // (stolen from KConfigDialogManager::updateWidgetsDefault())
  const bool bUseDefaults = GlobalSettings::self()->useDefaults( true );
  doLoadFromGlobalSettings();
  GlobalSettings::self()->useDefaults( bUseDefaults );
  // reset other settings to default values
  doResetToDefaultsOther();
}

void ConfigModuleTab::slotEmitChanged( void ) {
   emit changed( true );
}


#include "configuredialog_p.moc"
