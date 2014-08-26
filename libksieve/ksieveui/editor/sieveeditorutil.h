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

#ifndef SIEVEEDITORUTIL_H
#define SIEVEEDITORUTIL_H

#include <QString>

namespace KSieveUi
{
namespace SieveEditorUtil
{

enum HelpVariableName {
    UnknownHelp,
    AddressCondition,
    BodyCondition,
    ConvertCondition,
    CurrentdateCondition,
    DateCondition,
    EnvelopeCondition,
    EnvironmentCondition,
    ExistsCondition,
    FalseCondition,
    HasflagCondition,
    HeaderCondition,
    IhaveCondition,
    MailboxexistsCondition,
    MetadataexistsCondition,
    MetadataCondition,
    ServermetadataexistsCondition,
    ServermetadataCondition,
    SizeCondition,
    SpamtestCondition,
    TrueCondition,
    VirustestCondition,
    AbstracteditheaderAction,
    AddflagsAction,
    AddheaderAction,
    BreakAction,
    ConvertAction,
    DeleteheaderAction,
    DiscardAction,
    EncloseAction,
    ExtracttextAction,
    FileintoAction,
    KeepAction,
    NotifyAction,
    RedirectAction,
    RejectAction,
    RemoveflagsAction,
    ReplaceAction,
    ReturnAction,
    SetflagsAction,
    SetvariableAction,
    StopAction,
    VacationAction,
    GlobalVariable,
    Includes,
    ForEveryPart,
    CopyExtension,
    MBoxMetaDataExtension,
    SubAddressExtension
};
KSieveUi::SieveEditorUtil::HelpVariableName strToVariableName(const QString &str);

QString helpUrl(KSieveUi::SieveEditorUtil::HelpVariableName type);
}
}

#endif // SIEVEEDITORUTIL_H
