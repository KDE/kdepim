	void
EmpathComposeWindow::setupMenuBar()
{
	empathDebug("setting up menu bar");

	fileMenu_	= new QPopupMenu();
	CHECK_PTR(fileMenu_);

	editMenu_	= new QPopupMenu();
	CHECK_PTR(editMenu_);

	messageMenu_	= new QPopupMenu();
	CHECK_PTR(messageMenu_);
	
	helpMenu_	= new QPopupMenu();
	CHECK_PTR(helpMenu_);

	// File menu
	empathDebug("setting up file menu");
	
	fileMenu_->insertItem(empathIcon("mini-send.png"), i18n("&Send Message"),
		this, SLOT(s_fileSendMessage()));
	
	fileMenu_->insertItem(empathIcon("mini-sendlater.png"), i18n("Send &Later"),
		this, SLOT(s_fileSendLater()));
	
	fileMenu_->insertSeparator();

	fileMenu_->insertItem(empathIcon("blank.png"), i18n("&Close"),
		this, SLOT(s_fileClose()));

	// Edit menu
	
	empathDebug("setting up edit menu");

	editMenu_->insertItem(i18n("&Undo"),
		this, SLOT(s_editUndo()));
	
	editMenu_->insertItem(i18n("&Redo"),
		this, SLOT(s_editRedo()));
	
	editMenu_->insertSeparator();

	editMenu_->insertItem(empathIcon("empath-cut.png"), i18n("Cu&t"),
		this, SLOT(s_editCut()));
	
	editMenu_->insertItem(empathIcon("empath-copy.png"), i18n("&Copy"),
		this, SLOT(s_editCopy()));
	
	editMenu_->insertItem(empathIcon("empath-paste.png"), i18n("&Paste"),
		this, SLOT(s_editPaste()));
	
	editMenu_->insertItem(empathIcon("blank.png"), i18n("&Delete"),
		this, SLOT(s_editDelete()));

	editMenu_->insertSeparator();
	
	editMenu_->insertItem(empathIcon("blank.png"), i18n("&Select All"),
		this, SLOT(s_editSelectAll()));
	
	editMenu_->insertSeparator();
	
	editMenu_->insertItem(empathIcon("blank.png"), i18n("Find..."),
		this, SLOT(s_editFind()));
	
	editMenu_->insertItem(empathIcon("blank.png"), i18n("Find &Again"),
		this, SLOT(s_editFindAgain()));
	
	// Message Menu
	empathDebug("setting up message menu");

	messageMenu_->insertItem(empathIcon("mini-compose.png"), i18n("&New"),
		this, SLOT(s_messageNew()));

	messageMenu_->insertItem(empathIcon("mini-save.png"), i18n("Save &As"),
		this, SLOT(s_messageSaveAs()));

	messageMenu_->insertItem(empathIcon("copy.png"), i18n("&Copy to..."),
		this, SLOT(s_messageCopyTo()));
	
	helpMenu_->insertItem(i18n("&Contents"),
		this, SLOT(s_help()));

	helpMenu_->insertItem(
		i18n("&About Empath"),
		this, SLOT(s_aboutEmpath()));

	helpMenu_->insertSeparator();
	
	helpMenu_->insertItem(
		i18n("About &Qt"),
		this, SLOT(s_aboutQt()));
	
	helpMenu_->insertItem(i18n("About &KDE"),
			kapp, SLOT(aboutKDE()));
	
	menuBar()->insertItem(i18n("&File"), fileMenu_);
	menuBar()->insertItem(i18n("&Edit"), editMenu_);
	menuBar()->insertItem(i18n("&Message"), messageMenu_);
	menuBar()->insertSeparator();
	menuBar()->insertItem(i18n("&Help"), helpMenu_);
}


