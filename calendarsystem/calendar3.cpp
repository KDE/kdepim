// Simplest example using two kde calendar systems (gregorian and hijri)
// Carlos Moro <cfmoro@correo.uniovi.es>
// GNU-GPL v.2

#include <kapp.h>
#include "mkdatepicker.h"
#include <kdebug.h>

int main(int argc, char **argv) {

	KApplication *app;
	app = new KApplication(argc, argv, "Calendar");

	// Added optional parameter to specify calendar type y KDatePicker constructor
	KDatePicker *hijriCalendar = new KDatePicker("hijri");
	KDatePicker* defaultCalendar = new KDatePicker();
	kdDebug() << "BB";
	app->setMainWidget(defaultCalendar);

	defaultCalendar->show();
	hijriCalendar->show();

	app->exec();
}
