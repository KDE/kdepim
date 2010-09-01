/*
    This file is part of libkdepim.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (C) 2005 Allen Winter <winter@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqbuttongroup.h>
#include <tqlineedit.h>
#include <tqfont.h>
#include <tqspinbox.h>
#include <tqframe.h>
#include <tqcombobox.h>
#include <tqcheckbox.h>
#include <tqradiobutton.h>
#include <tqpushbutton.h>
#include <tqdatetimeedit.h>
#include <tqwhatsthis.h>

#include <kcolorbutton.h>
#include <kdebug.h>
#include <klocale.h>
#include <kfontdialog.h>
#include <kmessagebox.h>
#include <kconfigskeleton.h>
#include <kurlrequester.h>
#include "ktimeedit.h"
#include "kdateedit.h"

#include "kprefsdialog.h"
#include "kprefsdialog.moc"

namespace KPrefsWidFactory {

KPrefsWid *create( KConfigSkeletonItem *item, TQWidget *parent )
{
  KConfigSkeleton::ItemBool *boolItem =
      dynamic_cast<KConfigSkeleton::ItemBool *>( item );
  if ( boolItem ) {
    return new KPrefsWidBool( boolItem, parent );
  }

  KConfigSkeleton::ItemString *stringItem =
      dynamic_cast<KConfigSkeleton::ItemString *>( item );
  if ( stringItem ) {
    return new KPrefsWidString( stringItem, parent );
  }

  KConfigSkeleton::ItemEnum *enumItem =
      dynamic_cast<KConfigSkeleton::ItemEnum *>( item );
  if ( enumItem ) {
    TQValueList<KConfigSkeleton::ItemEnum::Choice> choices = enumItem->choices();
    if ( choices.isEmpty() ) {
      kdError() << "KPrefsWidFactory::create(): Enum has no choices." << endl;
      return 0;
    } else {
      KPrefsWidRadios *radios = new KPrefsWidRadios( enumItem, parent );
      TQValueList<KConfigSkeleton::ItemEnum::Choice>::ConstIterator it;
      for( it = choices.begin(); it != choices.end(); ++it ) {
        radios->addRadio( (*it).label );
      }
      return radios;
    }
  }

  KConfigSkeleton::ItemInt *intItem =
      dynamic_cast<KConfigSkeleton::ItemInt *>( item );
  if ( intItem ) {
    return new KPrefsWidInt( intItem, parent );
  }

  return 0;
}

}


TQValueList<TQWidget *> KPrefsWid::widgets() const
{
  return TQValueList<TQWidget *>();
}


KPrefsWidBool::KPrefsWidBool( KConfigSkeleton::ItemBool *item, TQWidget *parent )
  : mItem( item )
{
  mCheck = new TQCheckBox( item->label(), parent);
  connect( mCheck, TQT_SIGNAL( clicked() ), TQT_SIGNAL( changed() ) );
  if ( !item->whatsThis().isNull() ) {
    TQWhatsThis::add( mCheck, item->whatsThis() );
  }
}

void KPrefsWidBool::readConfig()
{
  mCheck->setChecked( mItem->value() );
}

void KPrefsWidBool::writeConfig()
{
  mItem->setValue( mCheck->isChecked() );
}

TQCheckBox *KPrefsWidBool::checkBox()
{
  return mCheck;
}

TQValueList<TQWidget *> KPrefsWidBool::widgets() const
{
  TQValueList<TQWidget *> widgets;
  widgets.append( mCheck );
  return widgets;
}


KPrefsWidInt::KPrefsWidInt( KConfigSkeleton::ItemInt *item,
                            TQWidget *parent )
  : mItem( item )
{
  mLabel = new TQLabel( mItem->label()+':', parent );
  mSpin = new TQSpinBox( parent );
  if ( !item->minValue().isNull() ) {
    mSpin->setMinValue( item->minValue().toInt() );
  }
  if ( !item->maxValue().isNull() ) {
    mSpin->setMaxValue( item->maxValue().toInt() );
  }
  connect( mSpin, TQT_SIGNAL( valueChanged( int ) ), TQT_SIGNAL( changed() ) );
  mLabel->setBuddy( mSpin );
  TQString whatsThis = mItem->whatsThis();
  if ( !whatsThis.isEmpty() ) {
    TQWhatsThis::add( mLabel, whatsThis );
    TQWhatsThis::add( mSpin, whatsThis );
  }
}

void KPrefsWidInt::readConfig()
{
  mSpin->setValue( mItem->value() );
}

void KPrefsWidInt::writeConfig()
{
  mItem->setValue( mSpin->value() );
}

TQLabel *KPrefsWidInt::label()
{
  return mLabel;
}

TQSpinBox *KPrefsWidInt::spinBox()
{
  return mSpin;
}

TQValueList<TQWidget *> KPrefsWidInt::widgets() const
{
  TQValueList<TQWidget *> widgets;
  widgets.append( mLabel );
  widgets.append( mSpin );
  return widgets;
}


KPrefsWidColor::KPrefsWidColor( KConfigSkeleton::ItemColor *item,
                                TQWidget *parent )
  : mItem( item )
{
  mButton = new KColorButton( parent );
  connect( mButton, TQT_SIGNAL( changed( const TQColor & ) ), TQT_SIGNAL( changed() ) );
  mLabel = new TQLabel( mButton, mItem->label()+':', parent );
  mLabel->setBuddy( mButton );
  TQString whatsThis = mItem->whatsThis();
  if (!whatsThis.isNull()) {
    TQWhatsThis::add(mButton, whatsThis);
  }
}

KPrefsWidColor::~KPrefsWidColor()
{
//  kdDebug(5300) << "KPrefsWidColor::~KPrefsWidColor()" << endl;
}

void KPrefsWidColor::readConfig()
{
  mButton->setColor( mItem->value() );
}

void KPrefsWidColor::writeConfig()
{
  mItem->setValue( mButton->color() );
}

TQLabel *KPrefsWidColor::label()
{
  return mLabel;
}

KColorButton *KPrefsWidColor::button()
{
  return mButton;
}


KPrefsWidFont::KPrefsWidFont( KConfigSkeleton::ItemFont *item,
                              TQWidget *parent, const TQString &sampleText )
  : mItem( item )
{
  mLabel = new TQLabel( mItem->label()+':', parent );

  mPreview = new TQLabel( sampleText, parent );
  mPreview->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );

  mButton = new TQPushButton( i18n("Choose..."), parent );
  connect( mButton, TQT_SIGNAL( clicked() ), TQT_SLOT( selectFont() ) );
  TQString whatsThis = mItem->whatsThis();
  if (!whatsThis.isNull()) {
    TQWhatsThis::add(mPreview, whatsThis);
    TQWhatsThis::add(mButton, whatsThis);
  }
}

KPrefsWidFont::~KPrefsWidFont()
{
}

void KPrefsWidFont::readConfig()
{
  mPreview->setFont( mItem->value() );
}

void KPrefsWidFont::writeConfig()
{
  mItem->setValue( mPreview->font() );
}

TQLabel *KPrefsWidFont::label()
{
  return mLabel;
}

TQFrame *KPrefsWidFont::preview()
{
  return mPreview;
}

TQPushButton *KPrefsWidFont::button()
{
  return mButton;
}

void KPrefsWidFont::selectFont()
{
  TQFont myFont(mPreview->font());
  int result = KFontDialog::getFont(myFont);
  if (result == KFontDialog::Accepted) {
    mPreview->setFont(myFont);
    emit changed();
  }
}


KPrefsWidTime::KPrefsWidTime( KConfigSkeleton::ItemDateTime *item,
                              TQWidget *parent )
  : mItem( item )
{
  mLabel = new TQLabel( mItem->label()+':', parent );
  mTimeEdit = new KTimeEdit( parent );
  mLabel->setBuddy( mTimeEdit );
  connect( mTimeEdit, TQT_SIGNAL( timeChanged( TQTime ) ), TQT_SIGNAL( changed() ) );
  TQString whatsThis = mItem->whatsThis();
  if ( !whatsThis.isNull() ) {
    TQWhatsThis::add( mTimeEdit, whatsThis );
  }
}

void KPrefsWidTime::readConfig()
{
  mTimeEdit->setTime( mItem->value().time() );
}

void KPrefsWidTime::writeConfig()
{
  // Don't overwrite the date value of the TQDateTime, so we can use a
  // KPrefsWidTime and a KPrefsWidDate on the same config entry!
  TQDateTime dt( mItem->value() );
  dt.setTime( mTimeEdit->getTime() );
  mItem->setValue( dt );
}

TQLabel *KPrefsWidTime::label()
{
  return mLabel;
}

KTimeEdit *KPrefsWidTime::timeEdit()
{
  return mTimeEdit;
}


KPrefsWidDuration::KPrefsWidDuration( KConfigSkeleton::ItemDateTime *item,
                                      TQWidget *parent )
  : mItem( item )
{
  mLabel = new TQLabel( mItem->label()+':', parent );
  mTimeEdit = new QTimeEdit( parent );
  mLabel->setBuddy( mTimeEdit );
  mTimeEdit->setAutoAdvance( true );
  mTimeEdit->setDisplay( QTimeEdit::Hours | QTimeEdit::Minutes );
  mTimeEdit->setRange( TQTime( 0, 1 ), TQTime( 24, 0 ) ); // [1min, 24hr]
  connect( mTimeEdit,
           TQT_SIGNAL( valueChanged( const TQTime & ) ), TQT_SIGNAL( changed() ) );
  TQString whatsThis = mItem->whatsThis();
  if ( !whatsThis.isNull() ) {
    TQWhatsThis::add( mTimeEdit, whatsThis );
  }
}

void KPrefsWidDuration::readConfig()
{
  mTimeEdit->setTime( mItem->value().time() );
}

void KPrefsWidDuration::writeConfig()
{
  TQDateTime dt( mItem->value() );
  dt.setTime( mTimeEdit->time() );
  mItem->setValue( dt );
}

TQLabel *KPrefsWidDuration::label()
{
  return mLabel;
}

QTimeEdit *KPrefsWidDuration::timeEdit()
{
  return mTimeEdit;
}


KPrefsWidDate::KPrefsWidDate( KConfigSkeleton::ItemDateTime *item,
                              TQWidget *parent )
  : mItem( item )
{
  mLabel = new TQLabel( mItem->label()+':', parent );
  mDateEdit = new KDateEdit( parent );
  mLabel->setBuddy( mDateEdit );
  connect( mDateEdit, TQT_SIGNAL( dateChanged( const TQDate& ) ), TQT_SIGNAL( changed() ) );
  TQString whatsThis = mItem->whatsThis();
  if ( !whatsThis.isNull() ) {
    TQWhatsThis::add( mDateEdit, whatsThis );
  }
}

void KPrefsWidDate::readConfig()
{
  mDateEdit->setDate( mItem->value().date().isValid() ? mItem->value().date() : TQDate::currentDate()  );
}

void KPrefsWidDate::writeConfig()
{
  TQDateTime dt( mItem->value() );
  dt.setDate( mDateEdit->date() );
  mItem->setValue( dt );
}

TQLabel *KPrefsWidDate::label()
{
  return mLabel;
}

KDateEdit *KPrefsWidDate::dateEdit()
{
  return mDateEdit;
}


KPrefsWidRadios::KPrefsWidRadios( KConfigSkeleton::ItemEnum *item,
                                  TQWidget *parent )
  : mItem( item )
{
  mBox = new TQButtonGroup( 1, Qt::Horizontal, mItem->label(), parent );
  connect( mBox, TQT_SIGNAL( clicked( int ) ), TQT_SIGNAL( changed() ) );
}

KPrefsWidRadios::~KPrefsWidRadios()
{
}

void KPrefsWidRadios::addRadio(const TQString &text, const TQString &whatsThis)
{
  TQRadioButton *r = new TQRadioButton(text,mBox);
  if (!whatsThis.isNull()) {
    TQWhatsThis::add(r, whatsThis);
  }
}

TQButtonGroup *KPrefsWidRadios::groupBox()
{
  return mBox;
}

void KPrefsWidRadios::readConfig()
{
  mBox->setButton( mItem->value() );
}

void KPrefsWidRadios::writeConfig()
{
  mItem->setValue( mBox->id( mBox->selected() ) );
}

TQValueList<TQWidget *> KPrefsWidRadios::widgets() const
{
  TQValueList<TQWidget *> w;
  w.append( mBox );
  return w;
}

KPrefsWidCombo::KPrefsWidCombo( KConfigSkeleton::ItemEnum *item,
                                  TQWidget *parent )
  : mItem( item )
{
  TQHBox *hbox = new TQHBox(parent);
  new TQLabel( mItem->label(), hbox );
  mCombo = new TQComboBox( hbox );
  connect( mCombo, TQT_SIGNAL( activated( int ) ), TQT_SIGNAL( changed() ) );
}

KPrefsWidCombo::~KPrefsWidCombo()
{
}

void KPrefsWidCombo::readConfig()
{
  mCombo->setCurrentItem( mItem->value() );
}

void KPrefsWidCombo::writeConfig()
{
  mItem->setValue( mCombo->currentItem() );
}

TQValueList<TQWidget *> KPrefsWidCombo::widgets() const
{
  TQValueList<TQWidget *> w;
  w.append( mCombo );
  return w;
}

TQComboBox* KPrefsWidCombo::comboBox()
{
  return mCombo;
}

KPrefsWidString::KPrefsWidString( KConfigSkeleton::ItemString *item,
                                  TQWidget *parent,
                                  TQLineEdit::EchoMode echomode )
  : mItem( item )
{
  mLabel = new TQLabel( mItem->label()+':', parent );
  mEdit = new TQLineEdit( parent );
  mLabel->setBuddy( mEdit );
  connect( mEdit, TQT_SIGNAL( textChanged( const TQString & ) ),
           TQT_SIGNAL( changed() ) );
  mEdit->setEchoMode( echomode );
  TQString whatsThis = mItem->whatsThis();
  if ( !whatsThis.isNull() ) {
    TQWhatsThis::add( mEdit, whatsThis );
  }
}

KPrefsWidString::~KPrefsWidString()
{
}

void KPrefsWidString::readConfig()
{
  mEdit->setText( mItem->value() );
}

void KPrefsWidString::writeConfig()
{
  mItem->setValue( mEdit->text() );
}

TQLabel *KPrefsWidString::label()
{
  return mLabel;
}

TQLineEdit *KPrefsWidString::lineEdit()
{
  return mEdit;
}

TQValueList<TQWidget *> KPrefsWidString::widgets() const
{
  TQValueList<TQWidget *> widgets;
  widgets.append( mLabel );
  widgets.append( mEdit );
  return widgets;
}


KPrefsWidPath::KPrefsWidPath( KConfigSkeleton::ItemPath *item, TQWidget *parent,
                              const TQString &filter, uint mode )
  : mItem( item )
{
  mLabel = new TQLabel( mItem->label()+':', parent );
  mURLRequester = new KURLRequester( parent );
  mLabel->setBuddy( mURLRequester );
  mURLRequester->setMode( mode );
  mURLRequester->setFilter( filter );
  connect( mURLRequester, TQT_SIGNAL( textChanged( const TQString & ) ),
           TQT_SIGNAL( changed() ) );
  TQString whatsThis = mItem->whatsThis();
  if ( !whatsThis.isNull() ) {
    TQWhatsThis::add( mURLRequester, whatsThis );
  }
}

KPrefsWidPath::~KPrefsWidPath()
{
}

void KPrefsWidPath::readConfig()
{
  mURLRequester->setURL( mItem->value() );
}

void KPrefsWidPath::writeConfig()
{
  mItem->setValue( mURLRequester->url() );
}

TQLabel *KPrefsWidPath::label()
{
  return mLabel;
}

KURLRequester *KPrefsWidPath::urlRequester()
{
  return mURLRequester;
}

TQValueList<TQWidget *> KPrefsWidPath::widgets() const
{
  TQValueList<TQWidget *> widgets;
  widgets.append( mLabel );
  widgets.append( mURLRequester );
  return widgets;
}


KPrefsWidManager::KPrefsWidManager( KConfigSkeleton *prefs )
  : mPrefs( prefs )
{
}

KPrefsWidManager::~KPrefsWidManager()
{
}

void KPrefsWidManager::addWid( KPrefsWid *wid )
{
  mPrefsWids.append( wid );
}

KPrefsWidBool *KPrefsWidManager::addWidBool( KConfigSkeleton::ItemBool *item,
                                             TQWidget *parent )
{
  KPrefsWidBool *w = new KPrefsWidBool( item, parent );
  addWid( w );
  return w;
}

KPrefsWidTime *KPrefsWidManager::addWidTime( KConfigSkeleton::ItemDateTime *item,
                                             TQWidget *parent )
{
  KPrefsWidTime *w = new KPrefsWidTime( item, parent );
  addWid( w );
  return w;
}

KPrefsWidDuration *KPrefsWidManager::addWidDuration( KConfigSkeleton::ItemDateTime *item,
                                                     TQWidget *parent )
{
  KPrefsWidDuration *w = new KPrefsWidDuration( item, parent );
  addWid( w );
  return w;
}

KPrefsWidDate *KPrefsWidManager::addWidDate( KConfigSkeleton::ItemDateTime *item,
                                             TQWidget *parent )
{
  KPrefsWidDate *w = new KPrefsWidDate( item, parent );
  addWid( w );
  return w;
}

KPrefsWidColor *KPrefsWidManager::addWidColor( KConfigSkeleton::ItemColor *item,
                                               TQWidget *parent )
{
  KPrefsWidColor *w = new KPrefsWidColor( item, parent );
  addWid( w );
  return w;
}

KPrefsWidRadios *KPrefsWidManager::addWidRadios( KConfigSkeleton::ItemEnum *item,
                                                 TQWidget *parent )
{
  KPrefsWidRadios *w = new KPrefsWidRadios( item, parent );
  TQValueList<KConfigSkeleton::ItemEnum::Choice> choices;
  choices = item->choices();
  TQValueList<KConfigSkeleton::ItemEnum::Choice>::ConstIterator it;
  for( it = choices.begin(); it != choices.end(); ++it ) {
    w->addRadio( (*it).label, (*it).whatsThis );
  }
  addWid( w );
  return w;
}

KPrefsWidCombo *KPrefsWidManager::addWidCombo( KConfigSkeleton::ItemEnum *item,
                                               TQWidget* parent )
{
  KPrefsWidCombo *w = new KPrefsWidCombo( item, parent );
  TQValueList<KConfigSkeleton::ItemEnum::Choice> choices;
  choices = item->choices();
  TQValueList<KConfigSkeleton::ItemEnum::Choice>::ConstIterator it;
  for( it = choices.begin(); it != choices.end(); ++it ) {
    w->comboBox()->insertItem( (*it).label, -1 );
  }
  addWid( w );
  return w;
}

KPrefsWidString *KPrefsWidManager::addWidString( KConfigSkeleton::ItemString *item,
                                                 TQWidget *parent )
{
  KPrefsWidString *w = new KPrefsWidString( item, parent,
                                            TQLineEdit::Normal );
  addWid( w );
  return w;
}

KPrefsWidPath *KPrefsWidManager::addWidPath( KConfigSkeleton::ItemPath *item,
                                             TQWidget *parent, const TQString &filter, uint mode )
{
  KPrefsWidPath *w = new KPrefsWidPath( item, parent, filter, mode );
  addWid( w );
  return w;
}

KPrefsWidString *KPrefsWidManager::addWidPassword( KConfigSkeleton::ItemString *item,
                                                   TQWidget *parent )
{
  KPrefsWidString *w = new KPrefsWidString( item, parent, TQLineEdit::Password );
  addWid( w );
  return w;
}

KPrefsWidFont *KPrefsWidManager::addWidFont( KConfigSkeleton::ItemFont *item,
                                             TQWidget *parent,
                                             const TQString &sampleText )
{
  KPrefsWidFont *w = new KPrefsWidFont( item, parent, sampleText );
  addWid( w );
  return w;
}

KPrefsWidInt *KPrefsWidManager::addWidInt( KConfigSkeleton::ItemInt *item,
                                           TQWidget *parent )
{
  KPrefsWidInt *w = new KPrefsWidInt( item, parent );
  addWid( w );
  return w;
}

void KPrefsWidManager::setWidDefaults()
{
  kdDebug() << "KPrefsWidManager::setWidDefaults()" << endl;

  bool tmp = mPrefs->useDefaults( true );

  readWidConfig();

  mPrefs->useDefaults( tmp );
}

void KPrefsWidManager::readWidConfig()
{
  kdDebug(5310) << "KPrefsWidManager::readWidConfig()" << endl;

  KPrefsWid *wid;
  for( wid = mPrefsWids.first(); wid; wid = mPrefsWids.next() ) {
    wid->readConfig();
  }
}

void KPrefsWidManager::writeWidConfig()
{
  kdDebug(5310) << "KPrefsWidManager::writeWidConfig()" << endl;

  KPrefsWid *wid;
  for( wid = mPrefsWids.first(); wid; wid = mPrefsWids.next() ) {
    wid->writeConfig();
  }

  mPrefs->writeConfig();
}


KPrefsDialog::KPrefsDialog( KConfigSkeleton *prefs, TQWidget *parent, char *name,
                            bool modal )
  : KDialogBase(IconList,i18n("Preferences"),Ok|Apply|Cancel|Default,Ok,parent,
                name,modal,true),
    KPrefsWidManager( prefs )
{
// TODO: This seems to cause a crash on exit. Investigate later.
//  mPrefsWids.setAutoDelete(true);

//  connect(this,TQT_SIGNAL(defaultClicked()),TQT_SLOT(setDefaults()));
  connect(this,TQT_SIGNAL(cancelClicked()),TQT_SLOT(reject()));
}

KPrefsDialog::~KPrefsDialog()
{
}

void KPrefsDialog::autoCreate()
{
  KConfigSkeletonItem::List items = prefs()->items();

  TQMap<TQString,TQWidget *> mGroupPages;
  TQMap<TQString,TQGridLayout *> mGroupLayouts;
  TQMap<TQString,int> mCurrentRows;

  KConfigSkeletonItem::List::ConstIterator it;
  for( it = items.begin(); it != items.end(); ++it ) {
    TQString group = (*it)->group();
    TQString name = (*it)->name();

    kdDebug() << "ITEMS: " << (*it)->name() << endl;

    TQWidget *page;
    TQGridLayout *layout;
    int currentRow;
    if ( !mGroupPages.contains( group ) ) {
      page = addPage( group );
      layout = new TQGridLayout( page );
      mGroupPages.insert( group, page );
      mGroupLayouts.insert( group, layout );
      currentRow = 0;
      mCurrentRows.insert( group, currentRow );
    } else {
      page = mGroupPages[ group ];
      layout = mGroupLayouts[ group ];
      currentRow = mCurrentRows[ group ];
    }

    KPrefsWid *wid = KPrefsWidFactory::create( *it, page );

    if ( wid ) {
      TQValueList<TQWidget *> widgets = wid->widgets();
      if ( widgets.count() == 1 ) {
        layout->addMultiCellWidget( widgets[ 0 ],
                                    currentRow, currentRow, 0, 1 );
      } else if ( widgets.count() == 2 ) {
        layout->addWidget( widgets[ 0 ], currentRow, 0 );
        layout->addWidget( widgets[ 1 ], currentRow, 1 );
      } else {
        kdError() << "More widgets than expected: " << widgets.count() << endl;
      }

      if ( (*it)->isImmutable() ) {
        TQValueList<TQWidget *>::Iterator it2;
        for( it2 = widgets.begin(); it2 != widgets.end(); ++it2 ) {
          (*it2)->setEnabled( false );
        }
      }

      addWid( wid );

      mCurrentRows.replace( group, ++currentRow );
    }
  }

  readConfig();
}

void KPrefsDialog::setDefaults()
{
  setWidDefaults();
}

void KPrefsDialog::readConfig()
{
  readWidConfig();

  usrReadConfig();
}

void KPrefsDialog::writeConfig()
{
  writeWidConfig();

  usrWriteConfig();

  readConfig();
}


void KPrefsDialog::slotApply()
{
  writeConfig();
  emit configChanged();
}

void KPrefsDialog::slotOk()
{
  slotApply();
  accept();
}

void KPrefsDialog::slotDefault()
{
  kdDebug() << "KPrefsDialog::slotDefault()" << endl;

  if (KMessageBox::warningContinueCancel(this,
      i18n("You are about to set all preferences to default values. All "
      "custom modifications will be lost."),i18n("Setting Default Preferences"),
      i18n("Reset to Defaults"))
    == KMessageBox::Continue) setDefaults();
}


KPrefsModule::KPrefsModule( KConfigSkeleton *prefs, TQWidget *parent,
                            const char *name )
  : KCModule( parent, name ),
    KPrefsWidManager( prefs )
{
  emit changed( false );
}

void KPrefsModule::addWid( KPrefsWid *wid )
{
  KPrefsWidManager::addWid( wid );

  connect( wid, TQT_SIGNAL( changed() ), TQT_SLOT( slotWidChanged() ) );
}

void KPrefsModule::slotWidChanged()
{
  kdDebug(5310) << "KPrefsModule::slotWidChanged()" << endl;

  emit changed( true );
}

void KPrefsModule::load()
{
  kdDebug(5310) << "KPrefsModule::load()" << endl;

  readWidConfig();

  usrReadConfig();

  emit changed( false );
}

void KPrefsModule::save()
{
  kdDebug(5310) << "KPrefsModule::save()" << endl;

  writeWidConfig();

  usrWriteConfig();
}

void KPrefsModule::defaults()
{
  setWidDefaults();

  emit changed( true );
}
