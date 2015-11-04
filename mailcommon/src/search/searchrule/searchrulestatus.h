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

#ifndef SEARCHRULESTATUS_H
#define SEARCHRULESTATUS_H

#include <AkonadiCore/Item>

#include "mailcommon/searchpattern.h"
#include <Akonadi/KMime/MessageStatus>
namespace MailCommon
{

//TODO: Check if the below one is needed or not!
// The below are used in several places and here so they are accessible.
struct MessageStatusInfo {
    const char *text;
    const char *icon;
};

// If you change the ordering here; also do it in the enum below
static const MessageStatusInfo StatusValues[] = {
    { I18N_NOOP2("message status", "Important"),     "emblem-important"    },
    { I18N_NOOP2("message status", "Action Item"),   "mail-task"           },
    { I18N_NOOP2("message status", "Unread"),        "mail-unread"         },
    { I18N_NOOP2("message status", "Read"),          "mail-read"           },
    { I18N_NOOP2("message status", "Deleted"),       "mail-deleted"        },
    { I18N_NOOP2("message status", "Replied"),       "mail-replied"        },
    { I18N_NOOP2("message status", "Forwarded"),     "mail-forwarded"      },
    { I18N_NOOP2("message status", "Queued"),        "mail-queued"         },
    { I18N_NOOP2("message status", "Sent"),          "mail-sent"           },
    { I18N_NOOP2("message status", "Watched"),       "mail-thread-watch"   },
    { I18N_NOOP2("message status", "Ignored"),       "mail-thread-ignored" },
    { I18N_NOOP2("message status", "Spam"),          "mail-mark-junk"      },
    { I18N_NOOP2("message status", "Ham"),           "mail-mark-notjunk"   },
    { I18N_NOOP2("message status", "Has Attachment"), "mail-attachment"     }  //must be last
};

static const int StatusValueCount =
    sizeof(StatusValues) / sizeof(MessageStatusInfo);
// we want to show all status entries in the quick search bar, but only the
// ones up to attachment in the search/filter dialog, because there the
// attachment case is handled separately.
static const int StatusValueCountWithoutHidden = StatusValueCount - 1;

/**
 *  This class represents a search to be performed against the status of a
 *  messsage. The status is represented by a bitfield.
 *
 *  @short This class represents a search pattern rule operating on message
 *  status.
 */
class MAILCOMMON_EXPORT SearchRuleStatus : public SearchRule
{
public:
    explicit SearchRuleStatus(const QByteArray &field = QByteArray(),
                              Function function = FuncContains,
                              const QString &contents = QString());

    explicit SearchRuleStatus(Akonadi::MessageStatus status,
                              Function function = FuncContains);

    bool isEmpty() const Q_DECL_OVERRIDE;
    bool matches(const Akonadi::Item &item) const Q_DECL_OVERRIDE;

    /**
     * @copydoc SearchRule::requiredPart()
     */
    RequiredPart requiredPart() const Q_DECL_OVERRIDE;

    void addQueryTerms(Akonadi::SearchTerm &groupTerm, bool &emptyIsNotAnError) const Q_DECL_OVERRIDE;

    //Not possible to implement optimized form for status searching
    using SearchRule::matches;

    static Akonadi::MessageStatus statusFromEnglishName(const QString &);

    QString informationAboutNotValidRules() const Q_DECL_OVERRIDE;

private:
    Akonadi::MessageStatus mStatus;
};

}

#endif // SEARCHRULESTATUS_H
