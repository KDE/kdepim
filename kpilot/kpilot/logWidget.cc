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
#include <qtooltip.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <qhbox.h>
#include <qtextstream.h>

#include <kglobal.h>
#include <kstddirs.h>
#include <kprogress.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include "logWidget.moc"

LogWidget::LogWidget(QWidget * parent) :
	PilotComponent(parent, "component_log", QString::null),
	DCOPObject("KPilotIface"),
	fLog(0L), 
	fShowTime(false),
	fSplash(0L),
	fLabel(0L),
	fProgress(0L)
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
	QToolTip::add(fLog, i18n("<qt>This lists all the messages received "
			"during the current HotSync</qt>"));
	grid->addMultiCellWidget(fLog, 1, 1,1,2);

	fLog->setText(i18n("<qt><B>HotSync Log</B></qt>"));

	QHBox *h = new QHBox(this);
	h->setSpacing(SPACING);
	QPushButton *b = new QPushButton(
		i18n("Clear the text of HotSync messages","Clear Log"),
		h);
	QToolTip::add(b,i18n("<qt>Clears the list of messages from the "
		"current HotSync.</qt>"));
	connect(b,SIGNAL(clicked()),this,SLOT(clearLog()));

	b = new QPushButton(i18n("Save Log"),h);
	QToolTip::add(b,i18n("<qt>You can save the list of messages received "
		"during this HotSync to a file (for example for use in a "
		"bug report) by clicking here.</qt>"));
	connect(b,SIGNAL(clicked()),this,SLOT(saveLog()));

	grid->addMultiCellWidget(h,2,2,1,2);

	fLabel = new QLabel(i18n("Sync Progress:"),this);
	grid->addWidget(fLabel,3,1);
	fProgress = new KProgress(0,100,0,KProgress::Horizontal,this);
	QToolTip::add(fProgress,i18n("<qt>The (estimated) percentage "
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

		fSplash = new QLabel(this);
		fSplash->setPixmap(QPixmap(splashPath));
		QTimer::singleShot(3000,this,SLOT(hideSplash()));
		grid->addMultiCellWidget(fSplash,1,3,1,2);
	}

	(void) logw_id;
}

void LogWidget::addMessage(const QString & s)
{
	FUNCTIONSETUP;

	if (fShowTime)
	{
		QString t;

		t = QTime::currentTime().toString();
		t.append("  ");
		t.append(s);
		fLog->append(t);
	}
	else
	{
		fLog->append(s);
	}
}

void LogWidget::addError(const QString & s)
{
	FUNCTIONSETUP;

	QString t("<qt><B>");

	if (fShowTime)
	{

		t = QTime::currentTime().toString();
		t.append("  ");
	}

	t.append(s);
	t.append("</B></qt>");

	fLog->append(s);
}

void LogWidget::addProgress(const QString &s,int i)
{
	FUNCTIONSETUP;
	
	logMessage(s);

	if ((i >= 0) && (i <= 100))
	{
		fProgress->setValue(i);
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
	FUNCTIONSETUP;

	if (fLog)
	{
		QTime t = QTime::currentTime();
		QString s1 = t.toString();

		s1.append("  ");
		s1.append(s);
		fLog->append(s1);
	}
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
// Revision 1.8  2001/11/25 22:02:57  adridg
// Save/clear the sync log
//
// Revision 1.7  2001/11/18 16:59:55  adridg
// New icons, DCOP changes
//
