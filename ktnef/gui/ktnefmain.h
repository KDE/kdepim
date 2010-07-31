/*
    ktnefmain.h

    Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef KTNEFMAIN_H
#define	KTNEFMAIN_H

#include <kmainwindow.h>
#include <tqstring.h>
#include <tqlistview.h>

class KTNEFView;
class KTNEFParser;
class KTNEFAttach;

class KTNEFMain : public KMainWindow
{
	Q_OBJECT

public:
	KTNEFMain(TQWidget *parent = 0, const char *name = 0);
	~KTNEFMain();

	void loadFile(const TQString& filename);

protected slots:
	void openFile();
	void viewFile();
	void viewFileAs();
	void extractFile();
	void extractFileTo();
	void propertiesFile();
	void optionDefaultDir();
	void extractAllFiles();
	void slotEditToolbars();
	void slotNewToolbarConfig();
	void slotShowMessageProperties();
	void slotShowMessageText();
	void slotSaveMessageText();

	void viewSelectionChanged();
	void viewRightButtonPressed(TQListViewItem *item, const TQPoint& p, int c);
	void viewDoubleClicked(TQListViewItem*);
	void viewDragRequested( const TQValueList<KTNEFAttach*>& list );
    void slotConfigureKeys();
//protected:
//	void closeEvent(TQCloseEvent *e);

private:
	void setupStatusbar();
	void setupActions();
	void setupTNEF();
	void enableExtractAll(bool on = true);
	void enableSingleAction(bool on = true);
	void cleanup();

	void extractTo(const TQString& dirname);
	TQString extractTemp(KTNEFAttach *att);

private:
	KTNEFView	*view_;
	KTNEFParser	*parser_;
	QString		filename_;
	QString		defaultdir_;
	QString		lastdir_;
};

#endif
