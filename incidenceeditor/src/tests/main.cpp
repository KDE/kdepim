/*
  Copyright (C) 2010  Bertjan Broeksema <broeksema@kde.org>
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "korganizereditorconfig.h"
#include "incidencedialog.h"
#include "incidencedefaults.h"

#include <CalendarSupport/KCalPrefs>
#include <akonadi/calendar/calendarsettings.h>
#include <Item>

#include <KCalCore/Event>
#include <KCalCore/Todo>
#include <KCalCore/Journal>

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <iostream>

using namespace IncidenceEditorNG;

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName("IncidenceEditorNGApp");
    QCoreApplication::setApplicationVersion("0.1");

    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption("new-event", i18n("Creates a new event")));
    parser.addOption(QCommandLineOption("new-todo", i18n("Creates a new todo")));
    parser.addOption(QCommandLineOption("new-journal", i18n("Creates a new journal")));
    parser.addOption(QCommandLineOption("item", i18n("Loads an existing item, or returns without doing anything "
                                        "when the item is not an event or todo."), "id"));
    parser.process(app);

    Akonadi::Item item(-1);

    IncidenceDefaults defaults;
    // Set the full emails manually here, to avoid that we get dependencies on
    // KCalPrefs all over the place.
    defaults.setFullEmails(CalendarSupport::KCalPrefs::instance()->fullEmails());
    // NOTE: At some point this should be generalized. That is, we now use the
    //       freebusy url as a hack, but this assumes that the user has only one
    //       groupware account. Which doesn't have to be the case necessarily.
    //       This method should somehow depend on the calendar selected to which
    //       the incidence is added.
    if (CalendarSupport::KCalPrefs::instance()->useGroupwareCommunication()) {
        defaults.setGroupWareDomain(
            QUrl(Akonadi::CalendarSettings::self()->freeBusyRetrieveUrl()).host());
    }

    if (parser.isSet("new-event")) {
        std::cout << "Creating new event..." << std::endl;
        KCalCore::Event::Ptr event(new KCalCore::Event);
        defaults.setDefaults(event);
        item.setPayload<KCalCore::Event::Ptr>(event);
    } else if (parser.isSet("new-todo")) {
        std::cout << "Creating new todo..." << std::endl;
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        defaults.setDefaults(todo);
        item.setPayload<KCalCore::Todo::Ptr>(todo);
    } else if (parser.isSet("new-journal")) {
        std::cout << "Creating new journal..." << std::endl;
        KCalCore::Journal::Ptr journal(new KCalCore::Journal);
        defaults.setDefaults(journal);
        item.setPayload<KCalCore::Journal::Ptr>(journal);
    } else if (parser.isSet("item")) {
        bool ok = false;
        qint64 id = parser.value("item").toLongLong(&ok);
        if (!ok) {
            std::cerr << "Invalid akonadi item id given." << std::endl;
            return 1;
        }

        item.setId(id);
        std::cout << "Trying to load Akonadi Item " << QString::number(id).toLatin1().data();
        std::cout << "..." << std::endl;
    } else {
        std::cerr << "Invalid usage." << std::endl << std::endl;
        return 1;
    }

    EditorConfig::setEditorConfig(new KOrganizerEditorConfig);

    IncidenceDialog *dialog = new IncidenceDialog();

    Akonadi::Collection collection(CalendarSupport::KCalPrefs::instance()->defaultCalendarId());

    if (collection.isValid()) {
        dialog->selectCollection(collection);
    }

    dialog->load(item);   // The dialog will show up once the item is loaded.

    return app.exec();
}
