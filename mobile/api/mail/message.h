/*
Copyright 2014  Michael Bohlender michael.bohlender@kdemail.net

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QObject>
#include <QString>
#include <Akonadi/ItemFetchJob>

class Message : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString subject READ subject NOTIFY messageChanged)
    Q_PROPERTY(QString from READ from NOTIFY messageChanged)
    Q_PROPERTY(QString textContent READ textContent NOTIFY messageChanged)

public:
    explicit Message( QObject *parent = 0 );

    QString subject() const;
    QString from() const;
    QString textContent() const;

signals:
    void messageChanged();

public slots:
    void loadMessage(const QString &id);

private slots:
    void slotItemReceived(const Akonadi::Item::List &itemList);

private:
    QString m_akonadiId;
    QString m_subject;
    QString m_from;
    QString m_textContent;
};


#endif //MESSAGE_H