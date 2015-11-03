/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "aclmanager.h"
#include "collectionaclwidget.h"
#include "acllistview.h"

#include <KLocalizedString>
#include <QVBoxLayout>
#include <QAction>
#include <QActionEvent>
#include <QHBoxLayout>
#include <QListView>
#include <QPushButton>

using namespace PimCommon;
/**
 * Unfortunately QPushButton doesn't support to plug in
 * a QAction like QToolButton does, so we have to reimplement it :(
 */
class ActionButton : public QPushButton
{
public:
    ActionButton(QWidget *parent = Q_NULLPTR)
        : QPushButton(parent),
          mDefaultAction(Q_NULLPTR)
    {
    }

    void setDefaultAction(QAction *action)
    {
        if (!actions().contains(action)) {
            addAction(action);
            connect(this, &QAbstractButton::clicked, action, &QAction::trigger);
        }

        setText(action->text());
        setEnabled(action->isEnabled());

        mDefaultAction = action;
    }

protected:
    void actionEvent(QActionEvent *event) Q_DECL_OVERRIDE {
        QAction *action = event->action();
        switch (event->type())
        {
        case QEvent::ActionChanged:
            if (action == mDefaultAction) {
                setDefaultAction(mDefaultAction);
            }
            return;
            break;
        default:
            break;
        }

        QPushButton::actionEvent(event);
    }

private:
    QAction *mDefaultAction;
};

CollectionAclWidget::CollectionAclWidget(QWidget *parent)
    : QWidget(parent),
      mAclManager(new PimCommon::AclManager(this))
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    AclListView *view = new AclListView;
    view->setObjectName(QStringLiteral("list_view"));
    layout->addWidget(view);

    view->setAlternatingRowColors(true);
    view->setModel(mAclManager->model());
    view->setSelectionModel(mAclManager->selectionModel());

    QWidget *buttonBox = new QWidget;
    QVBoxLayout *buttonBoxVBoxLayout = new QVBoxLayout(buttonBox);
    buttonBoxVBoxLayout->setMargin(0);
    layout->addWidget(buttonBox);

    ActionButton *button = new ActionButton(buttonBox);
    buttonBoxVBoxLayout->addWidget(button);
    button->setObjectName(QStringLiteral("add"));
    button->setDefaultAction(mAclManager->addAction());

    button = new ActionButton(buttonBox);
    buttonBoxVBoxLayout->addWidget(button);
    button->setObjectName(QStringLiteral("edit"));
    button->setDefaultAction(mAclManager->editAction());

    button = new ActionButton(buttonBox);
    buttonBoxVBoxLayout->addWidget(button);
    button->setDefaultAction(mAclManager->deleteAction());
    button->setObjectName(QStringLiteral("delete"));

    QWidget *spacer = new QWidget(buttonBox);
    buttonBoxVBoxLayout->addWidget(spacer);
    spacer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    connect(view, SIGNAL(doubleClicked(QModelIndex)), mAclManager->editAction(), SIGNAL(triggered()));
    connect(mAclManager, &AclManager::collectionCanBeAdministrated, view, &AclListView::slotCollectionCanBeAdministrated);
}

CollectionAclWidget::~CollectionAclWidget()
{

}

AclManager *CollectionAclWidget::aclManager() const
{
    return mAclManager;
}
