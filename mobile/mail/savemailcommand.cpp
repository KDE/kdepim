/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "savemailcommand_p.h"
#include "messagecore/utils/stringutil.h"
#include "messageviewer/viewer/nodehelper.h"

#include <AkonadiCore/collection.h>
#include <AkonadiCore/itemfetchjob.h>
#include <AkonadiCore/itemfetchscope.h>

#include <KUrl>
#include <KFileDialog>
#include <kio/jobclasses.h>
#include <KLocale>
#include <KIO/JobUiDelegate>
#include <kio/job.h>
#include <KMessageBox>
#include <KDebug>

//TODO: Review if it is needed in other place as well
KUrl subjectToUrl( const QString &subject )
{
  QString fileName = MessageCore::StringUtil::cleanFileName( subject.trimmed() );

  // avoid stripping off the last part of the subject after a "."
  // by KFileDialog, which thinks it's an extension
  if ( !fileName.endsWith( QLatin1String( ".mbox" ) ) )
    fileName += QLatin1String(".mbox");

  const QString filter = i18n( "*.mbox|email messages (*.mbox)\n*|all files (*)" );
  return KFileDialog::getSaveUrl( KUrl::fromPath( fileName ), filter );
}

#define STRDIM(x) (sizeof(x)/sizeof(*x)-1)
//TODO: copied from runtime/resources/mbox/libmbox/mbox_p.cpp . Check if we can share it.
QByteArray escapeFrom( const QByteArray &str )
{
  const unsigned int strLen = str.length();
  if ( strLen <= STRDIM( "From " ) )
    return str;

  // worst case: \nFrom_\nFrom_\nFrom_... => grows to 7/6
  QByteArray result( int( strLen + 5 ) / 6 * 7 + 1, '\0');

  const char * s = str.data();
  const char * const e = s + strLen - STRDIM( "From ");
  char * d = result.data();

  bool onlyAnglesAfterLF = false; // dont' match ^From_
  while ( s < e ) {
    switch ( *s ) {
    case '\n':
      onlyAnglesAfterLF = true;
      break;
    case '>':
      break;
    case 'F':
      if ( onlyAnglesAfterLF && qstrncmp( s+1, "rom ", STRDIM("rom ") ) == 0 )
        *d++ = '>';
      // fall through
    default:
      onlyAnglesAfterLF = false;
      break;
    }
    *d++ = *s++;
  }
  while ( s < str.data() + strLen )
    *d++ = *s++;

  result.truncate( d - result.data() );
  return result;
}
#undef STRDIM

QByteArray mboxMessageSeparator( const QByteArray &msg )
{
  KMime::Message mail;
  mail.setHead( KMime::CRLFtoLF( msg ) );
  mail.parse();

  QByteArray separator = "From ";

  KMime::Headers::From *from = mail.from( false );
  if ( !from || from->addresses().isEmpty() )
    separator += "unknown@unknown.invalid";
  else
    separator += from->addresses().first() + ' ';

  KMime::Headers::Date *date = mail.date(false);
  if (!date || date->isEmpty())
    separator += QDateTime::currentDateTime().toString( Qt::TextDate ).toUtf8() + '\n';
  else
    separator += date->as7BitString(false) + '\n';

  return separator;
}


//TODO: remove when it is moved to kdepimlibs, use Util::showJobError instead
void showJobError( KJob* job )
{
  assert(job);
  // we can be called from the KJob::kill, where we are no longer a KIO::Job
  // so better safe than sorry
  KIO::Job* kiojob = dynamic_cast<KIO::Job*>(job);
  if( kiojob && kiojob->ui() )
    kiojob->ui()->showErrorMessage();
  else
    kWarning() << "There is no GUI delegate set for a kjob, and it failed with error:" << job->errorString();
}


SaveMailCommand::SaveMailCommand(const Akonadi::Item& message, QObject *parent) :
    QObject(parent), mOffset(0), mJob(0), mTotalSize(0)
{
    mMessages.append(message);
    mMsgListIndex = 0;
}

SaveMailCommand::SaveMailCommand(const Akonadi::Item::List& messages, QObject *parent) :
    QObject(parent), mMessages(messages), mOffset(0), mJob(0), mTotalSize(0)
{
    mMsgListIndex = 0;
}

void SaveMailCommand::execute()
{
    Akonadi::Collection sourceFolder = mMessages.first().parentCollection();
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( mMessages, this );
    job->fetchScope().fetchFullPayload(true);
    connect( job, SIGNAL(result(KJob*)), this, SLOT(slotFetchDone(KJob*)));
}

void SaveMailCommand::slotFetchDone(KJob *job)
{
    if ( job->error() ) {
      // handle errors
      showJobError(job);
      emitResult( Failed );
      return;
    }

    Akonadi::ItemFetchJob *fjob = dynamic_cast<Akonadi::ItemFetchJob*>( job );
    Q_ASSERT( fjob );
    mMessages = fjob->items();

    mUrl = subjectToUrl( MessageViewer::NodeHelper::cleanSubject( mMessages.first().payload<KMime::Message::Ptr>().get() ) );
    if ( mUrl.isEmpty() ) {
        emitResult( Failed );
        return;
    }

    if (mMessages.count() == 1)
        mTotalSize = mMessages.first().size();

    kDebug() << mUrl << mTotalSize;

#ifndef KDEPIM_MOBILE_UI
    mJob = KIO::put( mUrl, -1 /*TODO: See MessageViewer::Util::getWritePermissions() */ );
    mJob->setTotalSize( mTotalSize );
    mJob->setAsyncDataEnabled( true );
    connect(mJob, SIGNAL(dataReq(KIO::Job*,QByteArray&)),
      SLOT(slotSaveDataReq()));
    connect(mJob, SIGNAL(result(KJob*)),
      SLOT(slotSaveResult(KJob*)));
#else
    if ( QFile::exists( mUrl.toLocalFile() ) && KMessageBox::warningContinueCancel( 0 /*parentWidget()*/,
          i18n("File %1 exists.\nDo you want to replace it?",
          mUrl.prettyUrl()), i18n("Save to File"), KGuiItem(i18n("&Replace")) ) != KMessageBox::Continue)
    {
      emitResult( Failed );
      return;
    }
    QFile file( mUrl.toLocalFile() );
    if ( file.open( QFile::WriteOnly ) ) {
      foreach ( const Akonadi::Item &item, mMessages ) {
        slotMessageRetrievedForSaving( item );
        file.write( mData );
      }
      emitResult( OK );
    } else {
      emitResult( Failed );
    }
#endif
}

//remove after the move to kdepimlibs
void SaveMailCommand::emitResult( Result value )
{
  emit result( value );
  deleteLater();
}


void SaveMailCommand::slotSaveDataReq()
{
  int remainingBytes = mData.size() - mOffset;
  if ( remainingBytes > 0 ) {
    // eat leftovers first
    if ( remainingBytes > MAX_CHUNK_SIZE )
      remainingBytes = MAX_CHUNK_SIZE;

    QByteArray data;
    data = QByteArray( mData.data() + mOffset, remainingBytes );
    mJob->sendAsyncData( data );
    mOffset += remainingBytes;
    return;
  }
  // No leftovers, process next message.
  if ( mMsgListIndex < static_cast<unsigned int>( mMessages.size() ) ) {
    slotMessageRetrievedForSaving( mMessages[mMsgListIndex] );
  } else {
    // No more messages. Tell the putjob we are done.
    QByteArray data = QByteArray();
    mJob->sendAsyncData( data );
  }
}

void SaveMailCommand::slotMessageRetrievedForSaving(const Akonadi::Item &msg)
{
  //if ( msg )
  {
    QByteArray msgData = msg.payloadData();
    QByteArray str( mboxMessageSeparator( msgData ) );
    str += escapeFrom( msgData );
    str += '\n';
    mData = str;
    mData.resize( mData.size() - 1 );
    mOffset = 0;
#ifndef KDEPIM_MOBILE_UI
    QByteArray data;
    int size;
    // Unless it is great than 64 k send the whole message. kio buffers for us.
    if( mData.size() >  MAX_CHUNK_SIZE )
      size = MAX_CHUNK_SIZE;
    else
      size = mData.size();

    data = QByteArray( mData, size );
    mJob->sendAsyncData( data );
    mOffset += size;
#endif
  }
  ++mMsgListIndex;
}


void SaveMailCommand::slotSaveResult(KJob *job)
{
  if (job->error())
  {
    if (job->error() == KIO::ERR_FILE_ALREADY_EXIST)
    {
      if (KMessageBox::warningContinueCancel(0 /*parentWidget()*/,
        i18n("File %1 exists.\nDo you want to replace it?",
         mUrl.prettyUrl()), i18n("Save to File"), KGuiItem(i18n("&Replace")))
        == KMessageBox::Continue) {
        mOffset = 0;

        mJob = KIO::put( mUrl, -1 /*See MessageViewer::Util::getWritePermissions()*/, KIO::Overwrite );
        mJob->setTotalSize( mTotalSize );
        mJob->setAsyncDataEnabled( true );
        connect(mJob, SIGNAL(dataReq(KIO::Job*,QByteArray&)),
            SLOT(slotSaveDataReq()));
        connect(mJob, SIGNAL(result(KJob*)),
            SLOT(slotSaveResult(KJob*)));
      }
    }
    else
    {
      showJobError(job);
      emitResult( Failed );
      deleteLater();
    }
  } else {
    emitResult( OK );
    deleteLater();
  }
}

#include "moc_savemailcommand_p.cpp"
