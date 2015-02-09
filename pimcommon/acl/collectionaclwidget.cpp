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

#include <KLocalizedString>
#include <KVBox>
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
          mDefaultAction(0)
    {
    }

    void setDefaultAction(QAction *action)
    {
        if (!actions().contains(action)) {
            addAction(action);
            connect(this, SIGNAL(clicked()), action, SLOT(trigger()));
        }

        setText(action->text());
        setEnabled(action->isEnabled());

        mDefaultAction = action;
    }

protected:
    virtual void actionEvent(QActionEvent *event)
    {
        QAction *action = event->action();
        switch (event->type()) {
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

    QListView *view = new QListView;
    view->setObjectName(QLatin1String("list_view"));
    layout->addWidget(view);

    view->setAlternatingRowColors(true);
    view->setModel(mAclManager->model());
    view->setSelectionModel(mAclManager->selectionModel());

    KVBox *buttonBox = new KVBox;
    layout->addWidget(buttonBox);

    ActionButton *button = new ActionButton(buttonBox);
    button->setObjectName(QLatin1String("add"));
    button->setDefaultAction(mAclManager->addAction());

    button = new ActionButton(buttonBox);
    button->setObjectName(QLatin1String("edit"));
    button->setDefaultAction(mAclManager->editAction());

    button = new ActionButton(buttonBox);
    button->setDefaultAction(mAclManager->deleteAction());
    button->setObjectName(QLatin1String("delete"));

    QWidget *spacer = new QWidget(buttonBox);
    spacer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    connect(view, SIGNAL(doubleClicked(QModelIndex)), mAclManager->editAction(), SIGNAL(triggered()));
}

CollectionAclWidget::~CollectionAclWidget()
{

}

AclManager *CollectionAclWidget::aclManager() const
{
    return mAclManager;
}
