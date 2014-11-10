/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "agentconfigdialog.h"
#include "agentconfigmodel.h"
#include <KLocalizedString>
#include <QIcon>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <KGuiItem>
#include <QVBoxLayout>

AgentConfigDialog::AgentConfigDialog(QWidget *parent) :
    QDialog(parent),
    m_model(new AgentConfigModel(this))
{
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    ui.setupUi(mainWidget);
    ui.propertyView->setModel(m_model);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::Apply);
    QPushButton *user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    QPushButton *user2Button = new QPushButton;
    buttonBox->addButton(user2Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AgentConfigDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AgentConfigDialog::reject);
    mainLayout->addWidget(buttonBox);
    KGuiItem::assign(user1Button, KGuiItem(i18n("Save Configuration")));
    KGuiItem::assign(user2Button, KGuiItem(i18n("Refresh")));
    buttonBox->button(QDialogButtonBox::Apply)->setText(i18n("Apply Configuration"));

    setWindowTitle(i18n("Agent Configuration"));

    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &AgentConfigDialog::reconfigure);
    connect(user1Button, &QPushButton::clicked, m_model, &AgentConfigModel::writeConfig);
    connect(user2Button, &QPushButton::clicked, m_model, &AgentConfigModel::reload);
}

void AgentConfigDialog::setAgentInstance(const Akonadi::AgentInstance &instance)
{
    m_instance = instance;
    m_model->setAgentInstance(instance);
}

void AgentConfigDialog::reconfigure()
{
    m_instance.reconfigure();
}

