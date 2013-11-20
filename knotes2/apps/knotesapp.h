/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2013, The KNotes Developers

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*******************************************************************/

#ifndef KNOTESAPP_H
#define KNOTESAPP_H

#include <QDomDocument>
#include <QList>
#include <QMap>
#include <QString>
#include <QWidget>

#include <kapplication.h>
#include <ksessionmanager.h>
#include <kxmlguiclient.h>

#include <Akonadi/Item>

class QTcpServer;
class KNotesAkonadiTray;
class KAction;
class KFind;
class KMenu;
class KNote;
class KNotesAlarm;
class KXMLGUIBuilder;
class KXMLGUIFactory;
class KNotesChangeRecorder;
class KNotesAkonadiTreeModel;

namespace DNSSD {
class PublicService;
}
class KJob;
class QModelIndex;
class KNotesApp
        : public QWidget, public KSessionManager, virtual public KXMLGUIClient
{
    Q_OBJECT
public:
    KNotesApp();
    ~KNotesApp();

    bool commitData( QSessionManager & );

public slots:
    void newNote( const QString &name = QString(),
                     const QString &text = QString() );
    void newNoteFromClipboard( const QString &name = QString() );
    void hideAllNotes() const;
    void showAllNotes() const;
    void showNote( const Akonadi::Item::Id &id ) const;
    void hideNote(const Akonadi::Item::Id &id ) const;
    QString name( const Akonadi::Item::Id &id ) const;
    QString text( const Akonadi::Item::Id &id ) const;
    void setName(const Akonadi::Item::Id &id, const QString &newName );
    void setText( const Akonadi::Item::Id &id, const QString &newText );

private:
    void showNote( KNote *note ) const;


private Q_SLOTS:
    void slotPreferences();
    void slotConfigUpdated();
    void slotAcceptConnection();
    void slotNoteCreationFinished(KJob*);
    void slotRowInserted(const QModelIndex &, int, int end);
    void slotItemRemoved(const Akonadi::Item &item);
    void slotItemChanged(const Akonadi::Item &item, const QSet<QByteArray> &);
    void updateNoteActions();
    void slotActivateRequested( bool, const QPoint& pos);
    void slotSecondaryActivateRequested( const QPoint& );
    void slotPrintSelectedNotes();
    void slotQuit();
    void slotConfigureAccels();

#if 0
    void killNote( const QString &id );
    void killNote( const QString &id, bool force );

    QVariantMap notes() const;



protected slots:
    void slotShowNote();
    void slotWalkThroughNotes();

    void slotOpenFindDialog();
    void slotFindNext();



    void slotNoteKilled( KCal::Journal *journal );



private:
    void showNote( KNote *note ) const;

private slots:
    void saveNotes( const QString & uid );


    void createNote( KCal::Journal *journal );
    void killNote( KCal::Journal *journal );


private:
    KNotesAlarm            *m_alarm;

    KFind           *m_find;
    QMap<QString, KNote *>::iterator *m_findPos;


    KAction         *m_findAction;

    QString m_noteUidModify;
#endif
private:
    void saveNotes();
    void updateNetworkListener();
    KXMLGUIFactory  *m_guiFactory;
    KXMLGUIBuilder  *m_guiBuilder;
    QDomDocument    m_noteGUI;
    KNotesAkonadiTray *mTray;
    KMenu           *m_noteMenu;
    KMenu           *m_contextMenu;
    QList<QAction *>       m_noteActions;
    QTcpServer             *m_listener;
    DNSSD::PublicService   *m_publisher;
    QHash<Akonadi::Item::Id, KNote*> mNotes;
    KNotesChangeRecorder *mNoteRecorder;
    KNotesAkonadiTreeModel *mNoteTreeModel;
};

#endif
