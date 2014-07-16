/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "configureimmutablewidgetutils.h"
#include "simplestringlisteditor.h"

#include <KLocalizedString>
#include <KDialog>
#include <KUrlRequester>
#include <KIntSpinBox>

#include <QWidget>
#include <QLineEdit>
#include <QGroupBox>
#include <QButtonGroup>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QComboBox>

using namespace PimCommon;

static const char * lockedDownWarning =
        I18N_NOOP("<qt><p>This setting has been fixed by your administrator.</p>"
                  "<p>If you think this is an error, please contact him.</p></qt>");

void ConfigureImmutableWidgetUtils::checkLockDown( QWidget * w, const KConfigSkeletonItem *item )
{
    if ( item->isImmutable() ) {
        w->setEnabled( false );
        w->setToolTip( i18n( lockedDownWarning ) );
    }
}

void ConfigureImmutableWidgetUtils::populateButtonGroup( QGroupBox * box, QButtonGroup * group, int orientation,
                          const KCoreConfigSkeleton::ItemEnum *e )
{
    box->setTitle( e->label() );
    if (orientation == Qt::Horizontal) {
        box->setLayout( new QHBoxLayout() );
    } else {
        box->setLayout( new QVBoxLayout() );
    }
    box->layout()->setSpacing( KDialog::spacingHint() );
    const int numberChoices(e->choices().size());
    for (int i = 0; i < numberChoices; ++i) {
        QRadioButton *button = new QRadioButton( e->choices().at(i).label, box );
        group->addButton( button, i );
        box->layout()->addWidget( button );
    }
}

void ConfigureImmutableWidgetUtils::populateCheckBox( QCheckBox * b, const KCoreConfigSkeleton::ItemBool *e )
{
    b->setText( e->label() );
}

void ConfigureImmutableWidgetUtils::loadWidget( QLineEdit * b, const KCoreConfigSkeleton::ItemString *e )
{
    checkLockDown( b, e );
    b->setText( e->value() );
}

void ConfigureImmutableWidgetUtils::loadWidget( QCheckBox * b, const KCoreConfigSkeleton::ItemBool *e )
{
    checkLockDown( b, e );
    b->setChecked( e->value() );
}

void ConfigureImmutableWidgetUtils::loadWidget( PimCommon::SimpleStringListEditor * b, const KCoreConfigSkeleton::ItemStringList *e )
{
    checkLockDown( b, e );
    b->setStringList( e->value() );
}

void ConfigureImmutableWidgetUtils::loadWidget( QGroupBox * box, QButtonGroup * group,
                 const KCoreConfigSkeleton::ItemEnum *e )
{
    Q_ASSERT( group->buttons().size() == e->choices().size() );
    checkLockDown( box, e );
    group->buttons()[e->value()]->setChecked( true );
}

void ConfigureImmutableWidgetUtils::loadWidget( QSpinBox * b, const KCoreConfigSkeleton::ItemInt *e )
{
    checkLockDown( b, e );
    b->setValue( e->value() );
}

void ConfigureImmutableWidgetUtils::saveSpinBox( QSpinBox * b, KCoreConfigSkeleton::ItemInt *e )
{
    e->setValue( b->value() );
}

void ConfigureImmutableWidgetUtils::saveSpinBox( QSpinBox * b, KCoreConfigSkeleton::ItemUInt *e )
{
    e->setValue( b->value() );
}

void ConfigureImmutableWidgetUtils::loadWidget( QSpinBox * b, const KCoreConfigSkeleton::ItemUInt *e )
{
    checkLockDown( b, e );
    b->setValue( e->value() );
}

void ConfigureImmutableWidgetUtils::saveCheckBox( QCheckBox * b, KCoreConfigSkeleton::ItemBool *e )
{
    e->setValue( b->isChecked() );
}

void ConfigureImmutableWidgetUtils::saveLineEdit( QLineEdit * b, KCoreConfigSkeleton::ItemString *e )
{
    e->setValue( b->text() );
}

void ConfigureImmutableWidgetUtils::saveUrlRequester( KUrlRequester * b, KCoreConfigSkeleton::ItemString *e )
{
    e->setValue(b->text());
}

void ConfigureImmutableWidgetUtils::loadWidget( KUrlRequester * b, const KCoreConfigSkeleton::ItemString *e )
{
    checkLockDown( b, e );
    b->setText( e->value() );
}

void ConfigureImmutableWidgetUtils::saveButtonGroup( QButtonGroup * group, KCoreConfigSkeleton::ItemEnum *e )
{
    Q_ASSERT( group->buttons().size() == e->choices().size() );
    if ( group->checkedId() != -1 ) {
        e->setValue( group->checkedId() );
    }
}

void ConfigureImmutableWidgetUtils::saveSimpleStringListEditor( PimCommon::SimpleStringListEditor * b, KCoreConfigSkeleton::ItemStringList *e )
{
    e->setValue(b->stringList());
}

void ConfigureImmutableWidgetUtils::loadWidget( QComboBox *b, const KCoreConfigSkeleton::ItemEnum *e )
{
    checkLockDown( b, e );
    b->setCurrentIndex( e->value() );
}

void ConfigureImmutableWidgetUtils::saveComboBox( QComboBox * b, KCoreConfigSkeleton::ItemEnum *e )
{
    Q_ASSERT( b->count() == e->choices().size() );
    if ( b->currentIndex() != -1 ) {
        e->setValue( b->currentIndex() );
    }
}
