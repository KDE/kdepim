/*
  This file is part of the kcal library.

  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Kevin Krammer, krake@kdab.com

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef RECURRENCEACTIONS_H
#define RECURRENCEACTIONS_H

#include "calendarsupport_export.h"

#include <KCalCore/Incidence>

class KDateTime;
class KGuiItem;
class QWidget;

namespace KCalCore
{

/**
  @short Utility functions for dealing with recurrences

  Incidences with recurrencies need to be treated differently than single independend ones.
  For example the user might be given the choice to not only modify a selected occurrence
  of an incidence but also all that follow that one, etc.

  @author Kevin Krammer, krake@kdab.com
  @since 4.6
*/
namespace RecurrenceActions
{
  /**
    @short Flags for indicating on which occurrences to work on

    Flags can be OR'ed together to get a combined scope.
  */
  enum Scope
  {
    /**
     Scope does not apply to any occurrence
    */
    NoOccurrence = 0,

    /**
     Scope does include the given/selected occurrence
    */
    SelectedOccurrence = 1,

    /**
     Scope does include occurrences before the given/selected occurrence
    */
    PastOccurrences = 2,

    /**
     Scope does include occurrences after the given/selected occurrence
    */
    FutureOccurrences = 4,

    /**
     Scope does include all occurrences (past, present and future)
    */
    AllOccurrences = PastOccurrences | SelectedOccurrence | FutureOccurrences
  };

  /**
    @short Checks what scope an action could be applied on for a given incidence

    Checks whether the incidence is occurring on the given date and whether there
    are occurrences in the past and future.

    @param incidence the incidence of which to check recurrences
    @param selectedOccurrence the date (including timespec) to use as the base occurence,
           i.e., from which to check for past and future occurrences

    @return the #Scope to which actions on the given @incidence can be applied to
  */
  CALENDARSUPPORT_EXPORT // TODO should be KCAL_EXPORT
  int availableOccurrences( const Incidence::Ptr &incidence, const KDateTime &selectedOccurrence );

  /**
    @short Presents a multiple choice scope selection dialog to the user

    Shows a message box style question dialog with checkboxes for occurrence scope flags
    so the user can be asked specifically which occurrences to apply actions to.

    @param selectedOccurrence the date to use for telling the user which occurrence is the selected one
    @param message the message which explains the change and selection options
    @param caption the dialog's caption
    @param action the GUI item to use for the "OK" button
    @param availableChoices combined #Scope values to select which options should be present
    @param preselectedChoices combined #Scope values to optionally preselect some of the options specified with @p availableChoices
    @param parent QWidget parent for the dialog

    @return the chosen #Scope options, OR'ed together
  */
  CALENDARSUPPORT_EXPORT // TODO should be KCAL_EXPORT
  int questionMultipleChoice( const KDateTime &selectedOccurrence,
                              const QString &message, const QString &caption, const KGuiItem &action,
                              int availableChoices, int preselectedChoices, QWidget *parent );

  /**
    @short Presents a message box with two action choices and cancel to the user

    Shows a message box style question dialog with two action scope buttons and cancel.
    This is for quick decisions like whether to only modify a single occurrence or all occurrences.

    @param message the message which explains the change and available options
    @param caption the dialog's caption
    @param actionSelected the GUI item to use for the button representing the #SelectedOccurrence scope
    @param actionAll the GUI item to use for the button representing the #AllOccurrences scope
    @param parent QWidget parent for the dialog

    @param #NoOccurrence on cancel, #SelectedOccurrence or #AllOccurrences on the respective action
  */
  CALENDARSUPPORT_EXPORT // TODO should be KCAL_EXPORT
  int questionSelectedAllCancel( const QString &message, const QString &caption,
                                 const KGuiItem &actionSelected, const KGuiItem &actionAll,
                                 QWidget *parent );

  /**
    @short Presents a message box with three action choices and cancel to the user

    Shows a message box style question dialog with three action scope buttons and cancel.
    This is for quick decisions like whether to only modify a single occurrence, to include future or all occurrences.

    @note The calling application code can of course decide to word the future action text
          in a way that it includes the selected occurrence, e.g. "Also Future Items".
          The returned value will still just be #FutureOccurrences so the calling code
          has to include #SelectedOccurrence itself if it passes the value further on

    @param message the message which explains the change and available options
    @param caption the dialog's caption
    @param actionSelected the GUI item to use for the button representing the #SelectedOccurrence scope
    @param actionSelected the GUI item to use for the button representing the #FutureOccurrences scope
    @param actionAll the GUI item to use for the button representing the #AllOccurrences scope
    @param parent QWidget parent for the dialog

    @param #NoOccurrence on cancel, #SelectedOccurrence, #FutureOccurrences or #AllOccurrences on the respective action
  */
  CALENDARSUPPORT_EXPORT // TODO should be KCAL_EXPORT
  int questionSelectedFutureAllCancel( const QString &message, const QString &caption,
                                       const KGuiItem &actionSelected, const KGuiItem &actionFuture, const KGuiItem &actionAll,
                                       QWidget *parent );
}

}

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
