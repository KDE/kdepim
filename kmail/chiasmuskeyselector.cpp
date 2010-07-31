#include "chiasmuskeyselector.h"

#include <klineedit.h>
#include <klistbox.h>
#include <klocale.h>

#include <tqlayout.h>
#include <tqlabel.h>

ChiasmusKeySelector::ChiasmusKeySelector( TQWidget* parent, const TQString& caption,
                                          const TQStringList& keys, const TQString& currentKey,
                                          const TQString& lastOptions )
  : KDialogBase( parent, "chiasmusKeySelector", true, caption, Ok|Cancel, Ok, true )
{
  TQWidget *page = makeMainWidget();

  TQVBoxLayout *layout = new TQVBoxLayout(page, KDialog::spacingHint());

  mLabel = new TQLabel( i18n( "Please select the Chiasmus key file to use:" ), page );
  layout->addWidget( mLabel );

  mListBox = new KListBox( page );
  mListBox->insertStringList( keys );
  const int current = keys.findIndex( currentKey );
  mListBox->setSelected( QMAX( 0, current ), true );
  mListBox->ensureCurrentVisible();
  layout->addWidget( mListBox, 1 );

  TQLabel* optionLabel = new TQLabel( i18n( "Additional arguments for chiasmus:" ), page );
  layout->addWidget( optionLabel );

  mOptions = new KLineEdit( lastOptions, page );
  optionLabel->setBuddy( mOptions );
  layout->addWidget( mOptions );

  layout->addStretch();

  connect( mListBox, TQT_SIGNAL( doubleClicked( TQListBoxItem * ) ), this, TQT_SLOT( slotOk() ) );
  connect( mListBox, TQT_SIGNAL( returnPressed( TQListBoxItem * ) ), this, TQT_SLOT( slotOk() ) );

  mListBox->setFocus();
}

TQString ChiasmusKeySelector::key() const
{
  return mListBox->currentText();
}

TQString ChiasmusKeySelector::options() const
{
  return mOptions->text();
}


#include "chiasmuskeyselector.moc"
