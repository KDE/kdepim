//Added by qt3to4:
#include <QGridLayout>
/* KPilot
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/
static const char *logw_id =
	"$Id$";

#include "options.h"

#include <qfile.h>
#include <qlayout.h>
#include <q3textedit.h>

#include <qdatetime.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <q3hbox.h>
#include <qtextstream.h>
#include <qpainter.h>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kprogressbar.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include <pi-version.h>

#ifndef PILOT_LINK_PATCH
#define PILOT_LINK_PATCH "unknown"
#endif

#include "logWidget.moc"

#if QT_VERSION < 0x030100
#define TE_EOL "<br/>"
#else
#define TE_EOL "\n"
#endif


LogWidget::LogWidget(QWidget * parent) :
	DCOPObject("LogIface"),
	PilotComponent(parent, "component_log", QString::null),
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

	fLog = new Q3TextEdit(this);
	fLog->setReadOnly(true);
	fLog->setWordWrap(Q3TextEdit::WidgetWidth);
	fLog->setWrapPolicy(Q3TextEdit::AtWordOrDocumentBoundary);
#if QT_VERSION < 0x030100
	/* nothing, use AutoText */
#else
	fLog->setTextFormat(Qt::LogText);
#endif

	fLog->setWhatsThis( i18n("<qt>This lists all the messages received "
			"during the current HotSync</qt>"));
	grid->addMultiCellWidget(fLog, 1, 1,1,2);


	QString initialText ;

	initialText.append(CSL1("<b>Version:</b> KPilot %1" TE_EOL)
		.arg(QString::fromLatin1(KPILOT_VERSION)));
	initialText.append(CSL1("<b>Version:</b> pilot-link %1.%2.%3%4" TE_EOL)
		.arg(PILOT_LINK_VERSION)
		.arg(PILOT_LINK_MAJOR)
		.arg(PILOT_LINK_MINOR)
#ifdef PILOT_LINK_PATCH
		.arg(QString::fromLatin1(PILOT_LINK_PATCH))
#else
		.arg(QString())
#endif
		);
#ifdef KDE_VERSION_STRING
	initialText.append(CSL1("<b>Version:</b> KDE %1" TE_EOL)
		.arg(QString::fromLatin1(KDE_VERSION_STRING)));
#endif
#ifdef QT_VERSION_STR
	initialText.append(CSL1("<b>Version:</b> Qt %1" TE_EOL)
		.arg(QString::fromLatin1(QT_VERSION_STR)));
#endif

	initialText.append(CSL1(TE_EOL));
	initialText.append(i18n("<qt><b>HotSync Log</b></qt>"));
	initialText.append(CSL1(TE_EOL));


	fLog->setText(initialText);
	fLog->scrollToBottom();

	Q3HBox *h = new Q3HBox(this);
	h->setSpacing(SPACING);
	QPushButton *b = new QPushButton(
		i18n("Clear the text of HotSync messages","Clear Log"),
		h);
	b->setWhatsThis(i18n("<qt>Clears the list of messages from the "
		"current HotSync.</qt>"));
	connect(b,SIGNAL(clicked()),this,SLOT(clearLog()));

	b = new QPushButton(i18n("Save Log..."),h);
	b->setWhatsThis(i18n("<qt>You can save the list of messages received "
		"during this HotSync to a file (for example for use in a "
		"bug report) by clicking here.</qt>"));
	connect(b,SIGNAL(clicked()),this,SLOT(saveLog()));

	fButtonBox = h;

	grid->addMultiCellWidget(h,2,2,1,2);

	fLabel = new QLabel(i18n("Sync progress:"),this);
	grid->addWidget(fLabel,3,1);
	fProgress = new KProgress(this);
	fProgress->setWhatsThis(i18n("<qt>The (estimated) percentage "
		"completed in the current HotSync.</qt>"));
	grid->addWidget(fProgress,3,2);


	QString splashPath =
		KGlobal::dirs()->findResource("data",
			CSL1("kpilot/kpilot-splash.png"));

	if (!splashPath.isEmpty() && QFile::exists(splashPath))
	{
		fLog->hide();
		fLabel->hide();
		fProgress->hide();

		QPixmap splash(splashPath);
		QPainter painter(&splash);
		painter.setPen(QColor(0, 255, 0));

		// This latin1() is ok; KPILOT_VERSION is a #define
		// of a constant string.
		int textWidth =fontMetrics().width(
			QString::fromLatin1(KPILOT_VERSION)) ;
		int textHeight = fontMetrics().height();

#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Using text size "
			<< textWidth << "x" << textHeight
			<< endl;
#endif

		painter.fillRect(splash.width() -  28 - textWidth,
			splash.height() - 6 - textHeight,
			textWidth + 6,
			textHeight + 4,
			Qt::black);
		painter.drawText(splash.width() -  25 - textWidth,
			splash.height() - 8,
			QString::fromLatin1(KPILOT_VERSION));
		fSplash = new QLabel(this);
		fSplash->setPixmap(splash);
		fSplash->setAlignment(Qt::AlignCenter);
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

	if (fShowTime)
	{
		t.append(CSL1("<b>"));
		t.append(QTime::currentTime().toString());
		t.append(CSL1("</b> "));
	}

	t.append(s);

#if QT_VERSION < 0x030100
	t.append(TE_EOL);
	fLog->setText(fLog->text() + t);
#else
	fLog->append(t);
#endif
	fLog->scrollToBottom();
}

void LogWidget::addError(const QString & s)
{
	FUNCTIONSETUP;

	if (s.isEmpty()) return;

	kWarning() << "KPilot error: " << s << endl;

	if (!fLog) return;

	QString t;

	t.append(CSL1("<i>"));
	t.append(s);
	t.append(CSL1("</i>"));

	addMessage(t);
}

void LogWidget::addProgress(const QString &s,int i)
{
	FUNCTIONSETUP;

	if (!s.isEmpty()) logMessage(s);

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

	addMessage(i18n("<b>HotSync Finished.</b>"));
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

/* DCOP */ ASYNC LogWidget::logError(QString s)
{
	addError(s);
}

/* DCOP */ ASYNC LogWidget::logProgress(QString s, int i)
{
	addProgress(s,i);
}

/* DCOP */ ASYNC LogWidget::logStartSync()
{
}

/* DCOP */ ASYNC LogWidget::logEndSync()
{
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
			CSL1("*.log"),     /* show log files by default */
			this,
			i18n("Save Log"));

		if (saveFileName.isEmpty()) return;
		if (QFile::exists(saveFileName))
		{
			int r = KMessageBox::warningYesNoCancel(
				this,
				i18n("The file exists. Do you want to "
					"overwrite it?"),
				i18n("File Exists"), i18n("Overwrite"), i18n("Do Not Overwrite"));
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
	if (!f.open(QIODevice::WriteOnly))
	{
		int r = KMessageBox::questionYesNo(this,
			i18n("<qt>Cannot open the file &quot;%1&quot; "
				"for writing; try again?</qt>"),
			i18n("Cannot Save"), i18n("Try Again"), i18n("Do Not Try"));

		if (r==KMessageBox::Yes) return false;
		return true;
	}
	else
	{
		QTextStream t(&f);
		t << fLog->text();
	}

	f.close();
	return true;
}

