#include <kcombobox.h>
#include <kdialog.h>
#include <klocale.h>
#include <kresources/configdialog.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "konnectorpair.h"
#include "plugineditorwidget.h"


PluginEditorWidget::PluginEditorWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  initGUI();

  connect( mTypeBox, SIGNAL( activated( int ) ), SLOT( typeChanged( int ) ) );
  connect( mOptionButton, SIGNAL( clicked() ), SLOT( changeOptions() ) );
}

PluginEditorWidget::~PluginEditorWidget()
{
}

void PluginEditorWidget::setLabel( const QString &label )
{
  mLabel->setText( label );
}

void PluginEditorWidget::setKonnector( KonnectorPair *pair, KSync::Konnector *konnector )
{
  mPair = pair;
  mKonnector = konnector;

  if ( mKonnector )
    mKonnectorUid = mKonnector->identifier();


  fillTypeBox();

  if ( mKonnector ) {
    QStringList types = mPair->manager()->resourceTypeNames();
    int pos = types.findIndex( mKonnector->type() );

    mTypeBox->setCurrentItem( pos );
  }
}

KSync::Konnector* PluginEditorWidget::konnector() const
{
//  mKonnector->setIdentifier( mKonnectorUid );
  return mKonnector;
}

void PluginEditorWidget::fillTypeBox()
{
  mTypeBox->clear();
  mTypeBox->insertStringList( mPair->manager()->resourceTypeDescriptions() );
}

void PluginEditorWidget::typeChanged( int )
{
  if ( mKonnector != 0 )
    mPair->manager()->remove( mKonnector );

  mKonnector = mPair->manager()->createResource( currentType() );

  if ( mKonnector != 0 )
    mPair->manager()->add( mKonnector );
}

void PluginEditorWidget::changeOptions()
{
  if ( mKonnector == 0 )
    return;

  KRES::ConfigDialog dlg( this, "konnector", mKonnector );

  if ( dlg.exec() )
    mPair->manager()->change( mKonnector );
}

QString PluginEditorWidget::currentType() const
{
  return mPair->manager()->resourceTypeNames()[ mTypeBox->currentItem() ];
}

void PluginEditorWidget::initGUI()
{
  QGridLayout *layout = new QGridLayout( this, 2, 3, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  mLabel = new QLabel( this );
  layout->addWidget( mLabel, 0, 0 );

  mTypeBox = new KComboBox( this );
  layout->addWidget( mTypeBox, 0, 1 );

  mLabel->setBuddy( mTypeBox );

  mOptionButton = new QPushButton( i18n( "Options..." ), this );
  layout->addWidget( mOptionButton, 0, 2 );

  mInfoLabel = new QLabel( this );
  layout->addMultiCellWidget( mInfoLabel, 1, 1, 1, 2 );
}

#include "plugineditorwidget.moc"
