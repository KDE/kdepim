#include <klocale.h>

#include <qlayout.h>

#include "paireditorwidget.h"

#include "paireditordialog.h"

PairEditorDialog::PairEditorDialog( QWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Pair Editor" ), Ok | Cancel, Ok,
                 parent, name, true, true )
{
  initGUI();

  setInitialSize( QSize( 300, 200 ) );
}

PairEditorDialog::~PairEditorDialog()
{
}

void PairEditorDialog::setPair( KonnectorPair *pair )
{
  mPairEditorWidget->setPair( pair );
}

KonnectorPair *PairEditorDialog::pair() const
{
  return mPairEditorWidget->pair();
}

void PairEditorDialog::initGUI()
{
  QWidget *page = plainPage();

  QVBoxLayout *layout = new QVBoxLayout( page );

  mPairEditorWidget = new PairEditorWidget( page, "PairEditorWidget" );
  layout->addWidget( mPairEditorWidget );
}

#include "paireditordialog.moc"
