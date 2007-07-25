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

#include "options.h"


#include <qdatetime.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qtextstream.h>
#include <qtimer.h>

#include <kfiledialog.h>
#include <kglobal.h>
#include <khbox.h>
#include <kmessagebox.h>
#include <kprogressdialog.h>
#include <kstandarddirs.h>

#include <pi-version.h>

#include "logWidget.moc"
#include "loggeradaptorgui.h"

#define TE_EOL "<br/>\n"


LogWidget::LogWidget(QWidget * parent) :
	PilotComponent(parent, "component_log", QString::null),
	fLog(0L),
	fShowTime(false),
	fLabel(0L),
	fProgress(0L),
	fButtonBox(0L)
{
	FUNCTIONSETUP;

	new LoggerAdaptorGUI(this);
	QDBusConnection::sessionBus().registerObject("/Logger", this);

	QGridLayout *grid = new QGridLayout(this);
	grid->setSpacing(SPACING);

	grid->addRowSpacing(0, SPACING);
	grid->addRowSpacing(1, 100);
	grid->addColSpacing(2, 100);
	grid->addRowSpacing(3, SPACING);
	grid->addColSpacing(0, SPACING);
	grid->addColSpacing(3, SPACING);
	grid->setRowStretch(1, 50);
	grid->setColStretch(2, 50);

	fLog = new QTextEdit(this);
	fLog->setReadOnly(true);
	fLog->setWordWrapMode(QTextOption::WordWrap);
	fLog->setTextFormat(Qt::LogText);

	fLog->setWhatsThis( i18n("<qt>This lists all the messages received "
			"during the current HotSync</qt>"));
	grid->addMultiCellWidget(fLog, 1, 1,1,2);


	fInitialText.append(CSL1("<b>Version:</b> KPilot %1" TE_EOL)
		.arg(QString::fromLatin1(KPILOT_VERSION)));
	fInitialText.append(CSL1("<b>Version:</b> pilot-link %1.%2.%3%4" TE_EOL)
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
	fInitialText.append(CSL1("<b>Version:</b> KDE %1" TE_EOL)
		.arg(QString::fromLatin1(KDE_VERSION_STRING)));
#endif
#ifdef QT_VERSION_STR
	fInitialText.append(CSL1("<b>Version:</b> Qt %1" TE_EOL)
		.arg(QString::fromLatin1(QT_VERSION_STR)));
#endif

	fInitialText.append(CSL1(TE_EOL));
	fInitialText.append(i18n("<qt><b>HotSync Log</b></qt>"));
	fInitialText.append(CSL1(TE_EOL));

	fLog->setText(fInitialText);
	fLog->moveCursor( QTextCursor::End );

	KHBox *h = new KHBox(this);
	h->setSpacing(SPACING);
	QPushButton *b = new QPushButton(
		i18nc("Clear the text of HotSync messages","Clear Log"),
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
	fProgress = new QProgressBar(this);
	fProgress->setWhatsThis(i18n("<qt>The (estimated) percentage "
		"completed in the current HotSync.</qt>"));
	grid->addWidget(fProgress,3,2);
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

	fLog->append(t);
	fLog->moveCursor(QTextCursor::End);
}

void LogWidget::addError(const QString & s)
{
	FUNCTIONSETUP;

	if (s.isEmpty()) return;

	WARNINGKPILOT << "KPilot error: " << s << endl;

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
		fProgress->setValue(i);
	}
}

void LogWidget::syncDone()
{
	FUNCTIONSETUP;

	addMessage(i18n("<b>HotSync Finished.</b>"));
}

void LogWidget::logMessage(QString s)
{
	addMessage(s);
}

void LogWidget::logError(QString s)
{
	addError(s);
}

void  LogWidget::logProgress(QString s, int i)
{
	addProgress(s,i);
}

void LogWidget::logStartSync()
{
}

void LogWidget::logEndSync()
{
}

/* slot */ void LogWidget::clearLog()
{
	FUNCTIONSETUP;

	if (fLog)
	{
		fLog->setText(fInitialText);
	}
}

/* slot */ void LogWidget::saveLog()
{
	FUNCTIONSETUP;

	bool finished = false;

	while (!finished)
	{
		QString saveFileName = KFileDialog::getSaveFileName(
			KUrl(),     /* default */
			CSL1("*.log"),     /* show log files by default */
			this,
			i18n("Save Log"));

		if (saveFileName.isEmpty())
		{
			return;
		}
		if (QFile::exists(saveFileName))
		{
			int r = KMessageBox::warningYesNoCancel(
				this,
				i18n("The file exists. Do you want to "
					"overwrite it?"),
				i18n("File Exists"), KGuiItem(i18n("Overwrite")), KGuiItem(i18n("Do Not Overwrite")));
			if (r==KMessageBox::Yes)
			{
				finished=saveFile(saveFileName);
			}
			if (r==KMessageBox::Cancel)
			{
				return;
			}
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
				"for writing; try again?</qt>", saveFileName),
			i18n("Cannot Save"), KGuiItem(i18n("Try Again")), KGuiItem(i18n("Do Not Try")));

		return !(r==KMessageBox::Yes);
	}
	else
	{
		QTextStream t(&f);
		t << fLog->toPlainText();
	}

	f.close();
	return true;
}

