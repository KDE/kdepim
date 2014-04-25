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

/*TODO: This class belongs to kdepimlibs/akonadi/kmime (an SMAM command), just that as
  of now, MessageViewer::NodeHelper and MessageCore::StringUtil is in kdepim, not in kdepimlibs
   There it should inherit from CommandBase, like the other commands.
   Based on KMSaveMsgCommand.
*/


#ifndef SAVEMAILCOMMAND_P_H
#define SAVEMAILCOMMAND_P_H

#include <QObject>

#include <AkonadiCore/item.h>

#include <KUrl>

namespace KIO {
    class TransferJob;
}

class KJob;

class SaveMailCommand : public QObject
{
    Q_OBJECT
public:
    explicit SaveMailCommand(const Akonadi::Item& message, QObject *parent = 0);
    explicit SaveMailCommand(const Akonadi::Item::List& messages, QObject *parent = 0);

    /*reimp*/ void execute();

    enum Result { Undefined, OK, Canceled, Failed }; //Remove after moving to kdepimlibs

Q_SIGNALS:
    void result( Result ); //Remove after moving to kdepimlibs


protected Q_SLOTS:
    virtual void emitResult( Result result ); //Remove after moving to kdepimlibs

private Q_SLOTS:
  void slotFetchDone( KJob* job );
  void slotSaveResult(KJob *job);
  /** the message has been transferred for saving */
  void slotMessageRetrievedForSaving(const Akonadi::Item &msg);
  void slotSaveDataReq();

private:
  Akonadi::Item::List mMessages;
  KUrl mUrl;
  uint mMsgListIndex;
  QByteArray mData;
  int mOffset;
  KIO::TransferJob *mJob;
  size_t mTotalSize;
  static const int MAX_CHUNK_SIZE = 64*1024;
};

#endif // SAVEMAILCOMMAND_P_H
