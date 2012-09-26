/*
    This file is part of Akregator2.
    Copyright (c) 2012 Frank Osterfeld <osterfeld@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR2_CONFIG_FEEDLISTS_H
#define AKREGATOR2_CONFIG_FEEDLISTS_H

#include <KCModule>
#include <KDialog>

#include <QPointer>
#include <QVariant>

class QWidget;

class KJob;

namespace Akonadi {
    class AgentInstance;
    class AgentType;
    class AgentTypeWidget;
}

namespace Akregator2 {
    namespace Ui {
        class SettingsFeedLists;
    }
}

class AddFeedListDialog : public KDialog {
    Q_OBJECT
public:
    explicit AddFeedListDialog( QWidget* parent );

private Q_SLOTS:
    void slotOkClicked();

Q_SIGNALS:
    void agentTypeSelected( const Akonadi::AgentType& );

private:
    Akonadi::AgentTypeWidget* m_agentTypeWidget;
};

class KCMAkregator2FeedListsConfig : public KCModule
{
    Q_OBJECT

public:
    KCMAkregator2FeedListsConfig( QWidget *parent, const QVariantList &args );
    ~KCMAkregator2FeedListsConfig();

private Q_SLOTS:
    void currentAgentChanged( const Akonadi::AgentInstance& current, const Akonadi::AgentInstance& previous );
    void agentDoubleClicked( const Akonadi::AgentInstance& current );
    void add();
    void modify();
    void remove();
    void restart();
    void agentTypeSelected( const Akonadi::AgentType& type );
    void agentCreated( KJob* job );

private:
    QWidget* m_widget;
    QPointer<AddFeedListDialog> m_addFeedListDialog;
    Akregator2::Ui::SettingsFeedLists* m_ui;
};

#endif // AKREGATOR2_CONFIG_FEEDLISTS_H
