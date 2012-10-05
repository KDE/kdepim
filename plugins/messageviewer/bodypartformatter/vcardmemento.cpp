#include "vcardmemento.h"

using namespace MessageViewer;

VcardMemento::VcardMemento( const QStringList& emails )
  : QObject( 0 ), mFinished( false )
{

}

VcardMemento::~VcardMemento()
{

}


bool VcardMemento::finished() const
{
  return mFinished;
}

void VcardMemento::detach()
{
   disconnect( this, SIGNAL(update(MessageViewer::Viewer::UpdateMode)), 0, 0 );
}
