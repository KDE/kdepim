#include "abbrowser.h"
#include <kapp.h>

int main(int argc, char *argv[])
{
	KApplication app(argc, argv, "abbrowser");

	// All session management is handled in the RESTORE macro
	if (app.isRestored())
	{
		RESTORE(Pab)
	}
	else
	{
		Pab *widget = new Pab;
		widget->show();
	}

	return app.exec();
}
