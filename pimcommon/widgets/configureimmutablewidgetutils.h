/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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


#ifndef CONFIGUREIMMUTABLEWIDGETUTILS_H
#define CONFIGUREIMMUTABLEWIDGETUTILS_H

#include "pimcommon_export.h"

#include <KConfigSkeletonItem>
#include <KConfigSkeleton>

class QWidget;
class QGroupBox;
class QCheckBox;
class QButtonGroup;
class QLineEdit;
class KConfigSkeletonItem;
class KUrlRequester;
namespace PimCommon {
class SimpleStringListEditor;
namespace ConfigureImmutableWidgetUtils {
PIMCOMMON_EXPORT void checkLockDown( QWidget * w, const KConfigSkeletonItem *item );
PIMCOMMON_EXPORT void populateButtonGroup( QGroupBox * box, QButtonGroup * group, int orientation, const KCoreConfigSkeleton::ItemEnum *e );
PIMCOMMON_EXPORT void populateCheckBox( QCheckBox * b, const KCoreConfigSkeleton::ItemBool *e );
PIMCOMMON_EXPORT void loadWidget( QCheckBox * b, const KCoreConfigSkeleton::ItemBool *e );
PIMCOMMON_EXPORT void loadWidget( QGroupBox * box, QButtonGroup * group, const KCoreConfigSkeleton::ItemEnum *e );
PIMCOMMON_EXPORT void loadWidget( QLineEdit * b, const KCoreConfigSkeleton::ItemString *e );
PIMCOMMON_EXPORT void loadWidget( KUrlRequester * b, const KCoreConfigSkeleton::ItemString *e );
PIMCOMMON_EXPORT void loadWidget( PimCommon::SimpleStringListEditor * b, const KCoreConfigSkeleton::ItemStringList *e );

PIMCOMMON_EXPORT void saveCheckBox( QCheckBox * b, KCoreConfigSkeleton::ItemBool *e );
PIMCOMMON_EXPORT void saveLineEdit( QLineEdit * b, KCoreConfigSkeleton::ItemString *e );
PIMCOMMON_EXPORT void saveUrlRequester( KUrlRequester * b, KCoreConfigSkeleton::ItemString *e );
PIMCOMMON_EXPORT void saveSimpleStringListEditorRequester( PimCommon::SimpleStringListEditor * b, KCoreConfigSkeleton::ItemStringList *e );

PIMCOMMON_EXPORT void saveButtonGroup( QButtonGroup * group, KCoreConfigSkeleton::ItemEnum *e );
}
}
#endif // CONFIGUREIMMUTABLEWIDGETUTILS_H
