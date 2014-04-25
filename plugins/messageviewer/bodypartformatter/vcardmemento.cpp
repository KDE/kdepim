/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "vcardmemento.h"
#include <Akonadi/Contact/ContactSearchJob>
#include <KDebug>
using namespace MessageViewer;

VcardMemento::VcardMemento( const QStringList& emails )
  : QObject( 0 ),
    mIndex( 0 ),
    mFinished( false )
{
  Q_FOREACH(const QString& str, emails) {
      VCard vcard(str,false);
      mVCardList.append(vcard);
  }
  checkEmail();
}

VcardMemento::~VcardMemento()
{

}

void VcardMemento::checkEmail()
{
  Akonadi::ContactSearchJob *searchJob = new Akonadi::ContactSearchJob();
  searchJob->setQuery( Akonadi::ContactSearchJob::Email, mVCardList.at(mIndex).email.toLower() );
  connect( searchJob, SIGNAL(result(KJob*)),
           this, SLOT(slotSearchJobFinished(KJob*)) );
}

void VcardMemento::slotSearchJobFinished( KJob *job )
{
  Akonadi::ContactSearchJob *searchJob = static_cast<Akonadi::ContactSearchJob*>( job );
  if ( searchJob->error() ) {
    kWarning() << "Unable to fetch contact:" << searchJob->errorText();
    mIndex++;
    continueToCheckEmail();
    return;
  }

  const int contactSize( searchJob->contacts().size() );
  if ( contactSize >= 1 ) {
    VCard vcard = mVCardList.at(mIndex);
    vcard.found = true;
    vcard.address = searchJob->contacts().first();
    mVCardList[mIndex] = vcard;
    if (contactSize>1)
        kDebug()<<" more than 1 contact was found";
  }

  mIndex++;
  continueToCheckEmail();
}

void VcardMemento::continueToCheckEmail()
{
  if(mIndex == mVCardList.count()) {
    mFinished = true;
    emit update( Viewer::Delayed );
  } else {
    checkEmail();
  }
}


bool VcardMemento::finished() const
{
  return mFinished;
}

void VcardMemento::detach()
{
  disconnect( this, SIGNAL(update(MessageViewer::Viewer::UpdateMode)), 0, 0 );
}

bool VcardMemento::vcardExist(int index) const
{
  return mVCardList.at(index).found;
}

KABC::Addressee VcardMemento::address( int index ) const
{
  return mVCardList.at(index).address;
}

