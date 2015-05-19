/*
  This file is part of KAddressBook.

  Copyright (c) 2015 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef GRAVATARUPDATEJOB_H
#define GRAVATARUPDATEJOB_H

#include <QObject>
#include <AkonadiCore/Item>
#include <QUrl>
namespace PimCommon
{
class GravatarResolvUrlJob;
}
namespace KABGravatar
{
class GravatarUpdateJob : public QObject
{
    Q_OBJECT
public:
    explicit GravatarUpdateJob(QObject *parent = Q_NULLPTR);
    ~GravatarUpdateJob();

    void start();
    bool canStart() const;

    QString email() const;
    void setEmail(const QString &email);

    Akonadi::Item item() const;
    void setItem(const Akonadi::Item &item);

Q_SIGNALS:
    void resolvedUrl(const QUrl &url);
    void gravatarPixmap(const QPixmap &pix);

private Q_SLOTS:
    void slotGravatarResolvUrlFinished(PimCommon::GravatarResolvUrlJob *job);

    void slotUpdateGravatarDone(KJob *job);
private:
    void updatePixmap(const QPixmap &pix);
    QString mEmail;
    Akonadi::Item mItem;
};
}

#endif // GRAVATARUPDATEJOB_H
