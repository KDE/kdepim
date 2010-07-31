/*
    This file is part of KOrganizer.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqvbuttongroup.h>
#include <tqwidgetstack.h>
#include <tqradiobutton.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqvbox.h>
#include <tqsplitter.h>

#include <kprinter.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kdeversion.h>

#include "korganizer/corehelper.h"

#include "calprinter.h"
#ifndef KORG_NOPRINTER
#include "calprinter.moc"

#include "calprintdefaultplugins.h"

CalPrinter::CalPrinter( TQWidget *parent, Calendar *calendar, KOrg::CoreHelper *helper )
  : TQObject( parent, "CalPrinter" )
{
  mParent = parent;
  mConfig = new KSimpleConfig( "korganizer_printing.rc" );
  mCoreHelper = helper;

  init( calendar );
}

CalPrinter::~CalPrinter()
{
  kdDebug(5850) << "~CalPrinter()" << endl;

  mPrintPlugins.clear();

  delete mConfig;
}

void CalPrinter::init( Calendar *calendar )
{
  mCalendar = calendar;

  mPrintPlugins.clear();
  mPrintPlugins.setAutoDelete( true );

  mPrintPlugins = mCoreHelper->loadPrintPlugins();
  mPrintPlugins.prepend( new CalPrintTodos() );
  mPrintPlugins.prepend( new CalPrintMonth() );
  mPrintPlugins.prepend( new CalPrintWeek() );
  mPrintPlugins.prepend( new CalPrintDay() );
  mPrintPlugins.prepend( new CalPrintIncidence() );

  KOrg::PrintPlugin::List::Iterator it = mPrintPlugins.begin();
  for ( ; it != mPrintPlugins.end(); ++it ) {
    if ( *it ) {
      (*it)->setConfig( mConfig );
      (*it)->setCalendar( mCalendar );
      (*it)->setKOrgCoreHelper( mCoreHelper );
      (*it)->doLoadConfig();
    }
  }
}

void CalPrinter::setDateRange( const TQDate &fd, const TQDate &td )
{
  KOrg::PrintPlugin::List::Iterator it = mPrintPlugins.begin();
  for ( ; it != mPrintPlugins.end(); ++it ) {
    (*it)->setDateRange( fd, td );
  }
}

void CalPrinter::print( int type, const TQDate &fd, const TQDate &td,
                        Incidence::List selectedIncidences, bool preview )
{
  KOrg::PrintPlugin::List::Iterator it = mPrintPlugins.begin();
  for ( it = mPrintPlugins.begin(); it != mPrintPlugins.end(); ++it ) {
    (*it)->setSelectedIncidences( selectedIncidences );
  }
  CalPrintDialog printDialog( mPrintPlugins, mParent );
  printDialog.setOrientation( CalPrinter::ePrintOrientation( mConfig->readNumEntry("Orientation", 1 ) ) );
  printDialog.setPreview( preview );
  printDialog.setPrintType( type );
  setDateRange( fd, td );

  if ( printDialog.exec() == TQDialog::Accepted ) {
    mConfig->writeEntry( "Orientation", printDialog.orientation() );

    // Save all changes in the dialog
    for ( it = mPrintPlugins.begin(); it != mPrintPlugins.end(); ++it ) {
      (*it)->doSaveConfig();
    }
    doPrint( printDialog.selectedPlugin(), printDialog.orientation(), preview );
  }
  for ( it = mPrintPlugins.begin(); it != mPrintPlugins.end(); ++it ) {
    (*it)->setSelectedIncidences( Incidence::List() );
  }
}

void CalPrinter::doPrint( KOrg::PrintPlugin *selectedStyle,
                          CalPrinter::ePrintOrientation dlgorientation, bool preview )
{
  if ( !selectedStyle ) {
    KMessageBox::error( mParent,
                 i18n("Unable to print, no valid print style was returned."),
                 i18n("Printing error") );
    return;
  }
  KPrinter printer;

  printer.setPreviewOnly( preview );
  switch ( dlgorientation ) {
    case eOrientPlugin:
      printer.setOrientation( selectedStyle->defaultOrientation() );
      break;
    case eOrientPortrait:
      printer.setOrientation( KPrinter::Portrait );
      break;
    case eOrientLandscape:
      printer.setOrientation( KPrinter::Landscape );
      break;
    case eOrientPrinter:
    default:
      break;
  }

  if ( preview || printer.setup( mParent, i18n("Print Calendar") ) ) {
    selectedStyle->doPrint( &printer );
  }
}

///////////////////////////////////////////////////////////////////////////////

void CalPrinter::updateConfig()
{
}



/****************************************************************************/

CalPrintDialog::CalPrintDialog( KOrg::PrintPlugin::List plugins,
                                TQWidget *parent, const char *name )
  : KDialogBase( parent, name, /*modal*/true, i18n("Print"), Ok | Cancel )
{
  TQVBox *page = makeVBoxMainWidget();

  TQSplitter *splitter = new TQSplitter( page );
  splitter->setOrientation( TQSplitter::Horizontal );

  mTypeGroup = new TQVButtonGroup( i18n("Print Style"), splitter, "buttonGroup" );
  // use the minimal width possible = max width of the radio buttons, not extensible
/*  mTypeGroup->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)4,
    (TQSizePolicy::SizeType)5, 0, 0,
      mTypeGroup->sizePolicy().hasHeightForWidth() ) );*/

  TQWidget *splitterRight = new TQWidget( splitter, "splitterRight" );
  TQGridLayout *splitterRightLayout = new TQGridLayout( splitterRight );
  splitterRightLayout->setMargin( marginHint() );
  splitterRightLayout->setSpacing( spacingHint() );

  mConfigArea = new TQWidgetStack( splitterRight, "configWidgetStack" );
  splitterRightLayout->addMultiCellWidget( mConfigArea, 0,0, 0,1 );

  TQLabel *orientationLabel = new TQLabel( i18n("Page &orientation:"),
                                         splitterRight, "orientationLabel" );
  splitterRightLayout->addWidget( orientationLabel, 1, 0 );

  mOrientationSelection = new TQComboBox( splitterRight, "orientationCombo" );
  mOrientationSelection->insertItem( i18n("Use Default Orientation of Selected Style") );
  mOrientationSelection->insertItem( i18n("Use Printer Default") );
  mOrientationSelection->insertItem( i18n("Portrait") );
  mOrientationSelection->insertItem( i18n("Landscape") );
  splitterRightLayout->addWidget( mOrientationSelection, 1, 1 );

  // signals and slots connections
  connect( mTypeGroup, TQT_SIGNAL( clicked( int ) ), TQT_SLOT( setPrintType( int ) ) );
  orientationLabel->setBuddy( mOrientationSelection );

  // First insert the config widgets into the widget stack. This possibly assigns
  // proper ids (when two plugins have the same sortID), so store them in a map
  // and use these new IDs to later sort the plugins for the type selection.
  for ( KOrg::PrintPlugin::List::Iterator it = plugins.begin();
        it != plugins.end(); ++it ) {
    int newid = mConfigArea->addWidget( (*it)->configWidget( mConfigArea ), (*it)->sortID() );
    mPluginIDs[newid] = (*it);
  }
  // Insert all plugins with in sorted order; plugins with clashing IDs will be first...
  TQMap<int, KOrg::PrintPlugin*>::ConstIterator mapit;
  for ( mapit = mPluginIDs.begin(); mapit != mPluginIDs.end(); ++mapit ) {
    KOrg::PrintPlugin *p = mapit.data();
    TQRadioButton *radioButton = new TQRadioButton( p->description(), mTypeGroup );
    radioButton->setEnabled( p->enabled() );
    mTypeGroup->insert( radioButton, mapit.key() );
//     radioButton->setMinimumHeight( radioButton->sizeHint().height() - 5 );
  }

  setMinimumSize( minimumSizeHint() );
  resize( minimumSizeHint() );
}

CalPrintDialog::~CalPrintDialog()
{
}

void CalPrintDialog::setPreview(bool preview)
{
#if KDE_IS_VERSION( 3, 1, 93 )
  setButtonOK( preview ? i18n("&Preview") : KStdGuiItem::print() );
#else
  setButtonOKText( preview ? i18n("&Preview") : i18n("&Print...") );
#endif
}

void CalPrintDialog::setPrintType( int i )
{
  mTypeGroup->setButton( i );
  mConfigArea->raiseWidget( i );
}

void CalPrintDialog::setOrientation( CalPrinter::ePrintOrientation orientation )
{
  mOrientation = orientation;
  mOrientationSelection->setCurrentItem( mOrientation );
}

KOrg::PrintPlugin *CalPrintDialog::selectedPlugin()
{
  int id = mTypeGroup->selectedId();
  if ( mPluginIDs.contains( id ) ) {
    return mPluginIDs[id];
  } else {
    return 0;
  }
}

void CalPrintDialog::slotOk()
{
  mOrientation = (CalPrinter::ePrintOrientation)mOrientationSelection->currentItem();

  TQMap<int, KOrg::PrintPlugin*>::Iterator it = mPluginIDs.begin();
  for ( ; it != mPluginIDs.end(); ++it ) {
    if ( it.data() )
      it.data()->readSettingsWidget();
  }

  KDialogBase::slotOk();
}

#endif
