/*
  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.trolltech.com and http://www.kde.org respectively

  Copyright (c) 2002-2004 Klar�vdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>
  Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA  02110-1301, USA.

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute liinitialCheckForChangesnked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#ifndef FREEBUSYMANAGER_P_H
#define FREEBUSYMANAGER_P_H

#include <KCalCore/ICalFormat>

#include <QtCore/QPointer>

class KJob;

namespace CalendarSupport {

class FreeBusyManager;

class FreeBusyManagerPrivate : public QObject
{
  Q_OBJECT

  FreeBusyManager *const q_ptr;
  Q_DECLARE_PUBLIC( FreeBusyManager )

  public: /// Members
    CalendarSupport::Calendar *mCalendar;
    KCalCore::ICalFormat mFormat;

    QStringList mRetrieveQueue;
    QMap<KUrl, QString> mFreeBusyUrlEmailMap;

    // Free/Busy uploading
    QDateTime mNextUploadTime;
    int mTimerID;
    bool mUploadingFreeBusy;
    bool mBrokenUrl;

    // the parentWidget to use while doing our "recursive" retrieval
    QPointer<QWidget>  mParentWidgetForRetrieval;

  public: /// Functions
    FreeBusyManagerPrivate( FreeBusyManager *q );
    void checkFreeBusyUrl();
    QString freeBusyDir() const;
    void freeBusyUrl( const QString &email );
    QString freeBusyToIcal( const KCalCore::FreeBusy::Ptr & );
    KCalCore::FreeBusy::Ptr iCalToFreeBusy( const QByteArray &freeBusyData );
    KCalCore::FreeBusy::Ptr ownerFreeBusy();
    QString ownerFreeBusyAsString();
    void processFreeBusyDownloadResult( KJob *_job );
    void processFreeBusyUploadResult( KJob *_job );
    bool processRetrieveQueue();
    void uploadFreeBusy();

  public slots:
    void contactSearchJobFinished( KJob *_job );
    void onFreeBusyUrlRetrieved( const QString &email, const KUrl &url );

  signals:
    void freeBusyUrlRetrieved( const QString &email, const KUrl &url );
};

}

#endif // FREEBUSYMANAGER_P_H