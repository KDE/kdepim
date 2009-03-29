#include "kjotsconfigdlg.h"
#include <kdemacros.h>


KJotsConfigDlg::KJotsConfigDlg( const QString & title, QWidget *parent )
  : KCMultiDialog( parent )
{
  setCaption( title );
  setFaceType( KPageDialog::List );
  setButtons( Default | Ok | Cancel );
  setDefaultButton( Ok );

  showButtonSeparator( true );

  addModule( "kjots_config_misc" );
  connect( this, SIGNAL(okClicked()), SLOT(slotOk()) );
}


KJotsConfigDlg::~KJotsConfigDlg()
{
}

void KJotsConfigDlg::slotOk()
{
}

extern "C"
{
  KDE_EXPORT KCModule *create_kjots_config_misc( QWidget *parent )
  {
      KComponentData instance( "kjots_config_misc" );
      return new KJotsConfigMisc( instance, parent );
  }
}

KJotsConfigMisc::KJotsConfigMisc( const KComponentData &inst, QWidget *parent )
    :KCModule( inst, parent )
{
    QHBoxLayout *lay = new QHBoxLayout( this );
    QWidget * w =  new confPageMisc( 0 );
    lay->addWidget( w );
    load();
}

void KJotsConfigMisc::load()
{
    KCModule::load();
}

void KJotsConfigMisc::save()
{
    KCModule::save();
}

#include "kjotsconfigdlg.moc"
