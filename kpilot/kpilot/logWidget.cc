/* logWidget.cc                         KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the log window widget, which logs
** sync-messages during a HotSync.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/
static const char *logw_id =
	"$Id$";

#include "options.h"

#include <qfile.h>
#include <qlayout.h>
#include <qtextview.h>
#include <qwhatsthis.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <qhbox.h>
#include <qtextstream.h>
#include <qpainter.h>

#include <kglobal.h>
#include <kstddirs.h>
#include <kprogress.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include <pi-version.h>

#ifndef PILOT_LINK_PATCH
#define PILOT_LINK_PATCH "unknown"
#endif

#include "logWidget.moc"

LogWidget::LogWidget(QWidget * parent) :
	PilotComponent(parent, "component_log", QString::null),
	DCOPObject("KPilotIface"),
	fLog(0L),
	fShowTime(false),
	fSplash(0L),
	fLabel(0L),
	fProgress(0L),
	fButtonBox(0L)
{
	FUNCTIONSETUP;
	QGridLayout *grid = new QGridLayout(this, 4, 4, SPACING);

	grid->addRowSpacing(0, SPACING);
	grid->addRowSpacing(1, 100);
	grid->addColSpacing(2, 100);
	grid->addRowSpacing(3, SPACING);
	grid->addColSpacing(0, SPACING);
	grid->addColSpacing(3, SPACING);
	grid->setRowStretch(1, 50);
	grid->setColStretch(2, 50);

	fLog = new QTextView(this);
	QWhatsThis::add(fLog, i18n("<qt>This lists all the messages received "
			"during the current HotSync</qt>"));
	grid->addMultiCellWidget(fLog, 1, 1,1,2);


	QString initialText ;

	initialText.append(QString("<b>Version:</b> KPilot %1<br/>").arg(KPILOT_VERSION));
	initialText.append(QString("<b>Version:</b> pilot-link %1.%2.%3%4<br><br>")
		.arg(PILOT_LINK_VERSION)
		.arg(PILOT_LINK_MAJOR)
		.arg(PILOT_LINK_MINOR)
#ifdef PILOT_LINK_PATCH
		.arg(PILOT_LINK_PATCH)
#else
		.arg(QString())
#endif
		);

	initialText.append(i18n("<qt><B>HotSync Log</B></qt>"));


	fLog->setText(initialText);

	QHBox *h = new QHBox(this);
	h->setSpacing(SPACING);
	QPushButton *b = new QPushButton(
		i18n("Clear the text of HotSync messages","Clear Log"),
		h);
	QWhatsThis::add(b,i18n("<qt>Clears the list of messages from the "
		"current HotSync.</qt>"));
	connect(b,SIGNAL(clicked()),this,SLOT(clearLog()));

	b = new QPushButton(i18n("Save Log"),h);
	QWhatsThis::add(b,i18n("<qt>You can save the list of messages received "
		"during this HotSync to a file (for example for use in a "
		"bug report) by clicking here.</qt>"));
	connect(b,SIGNAL(clicked()),this,SLOT(saveLog()));

	fButtonBox = h;

	grid->addMultiCellWidget(h,2,2,1,2);

	fLabel = new QLabel(i18n("Sync Progress:"),this);
	grid->addWidget(fLabel,3,1);
	fProgress = new KProgress(this);
	QWhatsThis::add(fProgress,i18n("<qt>The (estimated) percentage "
		"completed in the current HotSync.</qt>"));
	grid->addWidget(fProgress,3,2);


	QString splashPath =
		KGlobal::dirs()->findResource("data",
			"kpilot/kpilot-splash.png");

	if (!splashPath.isEmpty() && QFile::exists(splashPath))
	{
		fLog->hide();
		fLabel->hide();
		fProgress->hide();

		QPixmap splash(splashPath);
		QPainter painter(&splash);
		painter.setPen(QColor(255, 0, 0));

		int textWidth =fontMetrics().width(KPILOT_VERSION) ;
		int textHeight = fontMetrics().height();

#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Using text size "
			<< textWidth << "x" << textHeight
			<< endl;
#endif

		painter.fillRect(splash.width() -  28 - textWidth,
			splash.height() - 6 - textHeight - textHeight ,
			textWidth + 6,
			textHeight + 4,
			black);
		painter.drawText(splash.width() -  25 - textWidth,
			splash.height() - 8 - textHeight,
			KPILOT_VERSION);
		fSplash = new QLabel(this);
		fSplash->setPixmap(splash);
		fSplash->setAlignment(AlignCenter);
		QTimer::singleShot(3000,this,SLOT(hideSplash()));
		grid->addMultiCellWidget(fSplash,1,3,1,2);
		grid->addColSpacing(0,10);
		grid->setColStretch(1,50);
		grid->setColStretch(2,50);
		grid->addColSpacing(3,10);
	}

	(void) logw_id;
}

void LogWidget::addMessage(const QString & s)
{
	FUNCTIONSETUP;

	if (s.isEmpty()) return;
	if (!fLog) return;

	QString t;

#ifdef KDE2
	t = fLog->text();
#endif

	if (fShowTime)
	{
		t.append("<b>");

		t.append(QTime::currentTime().toString());
		t.append("</b>  ");
	}

	t.append(s);
	t.append("<br>");

#ifdef KDE2
	fLog->setText(t);
#else
	fLog->append(t);
#endif
}

void LogWidget::addError(const QString & s)
{
	FUNCTIONSETUP;

	kdWarning() << "KPilot error: " << s << endl;

	if (s.isEmpty()) return;
	if (!fLog) return;

	QString t("<qt>");

	if (fShowTime)
	{
		t.append("<b>");
		t = QTime::currentTime().toString();
		t.append("</b>  ");
	}

	t.append("<i>");
	t.append(s);
	t.append("</i><br></qt>");

	addMessage(t);
}

void LogWidget::addProgress(const QString &s,int i)
{
	FUNCTIONSETUP;

	logMessage(s);

	if ((i >= 0) && (i <= 100))
	{
		// setValue seems to be in both KDE2 and
		// KDE3, but is marked deprecated in KDE3.
		//
		//
#ifdef KDE2
		fProgress->setValue(i);
#else
		fProgress->setProgress(i);
#endif
	}
}

void LogWidget::syncDone()
{
	FUNCTIONSETUP;

	addMessage(i18n("<b>HotSync Finished!</b>"));
}

void LogWidget::initialize()
{
	FUNCTIONSETUP;
}

void LogWidget::hideSplash()
{
	FUNCTIONSETUP;

	if (fSplash)
	{
		fSplash->hide();
		KPILOT_DELETE(fSplash);
	}

	fLog->show();
	fLabel->show();
	fProgress->show();
}


/* DCOP */ ASYNC LogWidget::logMessage(QString s)
{
	addMessage(s);
}

/* DCOP */ ASYNC LogWidget::logProgress(QString s, int i)
{
	FUNCTIONSETUP;

	addProgress(s,i);
}

/* slot */ void LogWidget::clearLog()
{
	FUNCTIONSETUP;

	if (fLog)
	{
		fLog->setText(QString::null);
	}
}

/* slot */ void LogWidget::saveLog()
{
	FUNCTIONSETUP;

	bool finished = false;

	while (!finished)
	{
		QString saveFileName = KFileDialog::getSaveFileName(
			QString::null,     /* default */
			"*.log",           /* show log files by default */
			this,
			i18n("Save Log"));

		if (saveFileName.isEmpty()) return;
		if (QFile::exists(saveFileName))
		{
			int r = KMessageBox::warningYesNoCancel(
				this,
				i18n("The file exists. Do you want to "
					"overwrite it?"),
				i18n("File Exists"));
			if (r==KMessageBox::Yes)
			{
				finished=saveFile(saveFileName);
			}

			if (r==KMessageBox::Cancel) return;
		}
		else
		{
			finished=saveFile(saveFileName);
		}
	}
}


bool LogWidget::saveFile(const QString &saveFileName)
{
	FUNCTIONSETUP;

	QFile f(saveFileName);
	if (!f.open(IO_WriteOnly))
	{
		int r = KMessageBox::questionYesNo(this,
			i18n("<qt>Can't open the file &quot;%1&quot; "
				"for writing. Try again?</qt>"),
			i18n("Can't Save"));

		if (r==KMessageBox::Yes) return false;
		return true;
	}
	else
	{
		QTextStream t(&f);
		t << fLog->text();
	}

	f.close();
}

// $Log$
// Revision 1.23  2002/05/19 15:01:49  adridg
// Patches for the KNotes conduit
//
// Revision 1.22  2002/05/18 23:28:19  adridg
// Compile fixes
//
// Revision 1.21  2002/05/14 22:57:40  adridg
// Merge from _BRANCH
//
// Revision 1.20  2002/04/10 20:09:03  cschumac
// Make it compile.
//
// Revision 1.19  2002/04/10 06:13:34  adridg
// Show version of pilot-link in about
//
// Revision 1.18.2.3  2002/04/14 22:26:12  adridg
// New TODO's for HEAD; cosmetic bugfix in logWidget; rectify misleading debug output when KPilot starts the daemon itself.
//
// Revision 1.18.2.2  2002/04/13 11:40:24  adridg
// Simplification of logging code, display pilot-link version on startup
//
// Revision 1.18.2.1  2002/04/04 20:28:28  adridg
// Fixing undefined-symbol crash in vcal. Fixed FD leak. Compile fixes
// when using PILOT_VERSION. kpilotTest defaults to list, like the options
// promise. Always do old-style USB sync (also works with serial devices)
// and runs conduits only for HotSync. KPilot now as it should have been
// for the 3.0 release.
//
// Revision 1.18  2002/02/10 22:21:33  adridg
// Handle pilot-link 0.10.1; spit 'n polish; m505 now supported?
//
// Revision 1.17  2002/02/02 11:46:02  adridg
// Abstracting away pilot-link stuff
//
// Revision 1.16  2002/01/25 21:43:12  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//
// Revision 1.15  2002/01/23 18:55:19  danimo
// - xml tags in QTextEdit logwin
// - new line for each entry in log
//
// Revision 1.14  2002/01/23 08:36:26  adridg
// Handle KProgress::setValue vs setProgress decisively
//
// Revision 1.13  2002/01/22 19:42:25  bero
// Fix build with current kdelibs
//
// Revision 1.12  2002/01/21 23:58:55  aseigo
// KProgress updates
//
// Revision 1.11  2001/12/31 14:41:06  harald
// Make it compile
//
// Revision 1.10  2001/12/31 09:38:09  adridg
// Splash patch by Aaron
//
// Revision 1.9  2001/12/29 15:44:16  adridg
// Missing progress slots
//
// Revision 1.8  2001/11/25 22:02:57  adridg
// Save/clear the sync log
//
// Revision 1.7  2001/11/18 16:59:55  adridg
// New icons, DCOP changes
//
