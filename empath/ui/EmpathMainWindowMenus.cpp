	void
EmpathMainWindow::setupMenuBar()
{
	empathDebug("setting up menu bar");

	fileMenu_		= new QPopupMenu;
	CHECK_PTR(fileMenu_);

	editMenu_		= new QPopupMenu;
	CHECK_PTR(editMenu_);

	folderMenu_		= new QPopupMenu;
	CHECK_PTR(folderMenu_);

	messageMenu_	= new QPopupMenu;
	CHECK_PTR(messageMenu_);
	
	optionsMenu_	= new QPopupMenu;
	CHECK_PTR(optionsMenu_);

	helpMenu_		= new QPopupMenu;
	CHECK_PTR(helpMenu_);

	// File menu
	
	fileMenu_->insertItem(empathIcon("send.png"), i18n("&Send Pending Mail"),
		this, SLOT(s_fileSendNew()));

//	fileMenu_->insertSeparator();

//	fileMenu_->insertItem(i18n("Address&book..."),
//		this, SLOT(s_fileAddressBook()));
	
//	fileMenu_->insertSeparator();

	fileMenu_->insertItem(i18n("E&xit"),
		this, SLOT(s_fileQuit()));

	// Edit menu
	
	editMenu_->insertItem(empathIcon("tree-marked.png"), i18n("Select &Tagged"),
		this, SLOT(s_editSelectTagged()));
	
	editMenu_->insertItem(empathIcon("tree-read.png"), i18n("Select &Read"),
		this, SLOT(s_editSelectRead()));
	
	editMenu_->insertItem(i18n("Select &All"),
		this, SLOT(s_editSelectAll()));
	
	editMenu_->insertItem(i18n("&Invert Selection"),
		this, SLOT(s_editInvertSelection()));
	
	// Folder menu

	folderMenu_->insertItem(empathIcon("mini-folder-grey.png"),
		i18n("&New") + "...",
		this, SLOT(s_folderNew()));

	folderMenu_->insertItem(
		i18n("&Properties") + "...",
		this, SLOT(s_folderEdit()));

	folderMenu_->insertItem(empathIcon("mini-folder-outbox.png"),
		i18n("&Clear") + "...",
		this, SLOT(s_folderClear()));

	folderMenu_->insertItem(
		i18n("Delete") + "...",
		this, SLOT(s_folderDelete()));

	
	messageMenu_->insertItem(empathIcon("mini-view.png"),
		i18n("&View"),
		this, SLOT(s_messageView()));
	
	messageMenu_->insertSeparator();

	messageMenu_->insertItem(empathIcon("mini-compose.png"),
		i18n("&New"),
		this, SLOT(s_messageNew()));

	messageMenu_->insertItem(empathIcon("mini-reply.png"),
		i18n("&Reply"),
		this, SLOT(s_messageReply()));

	messageMenu_->insertItem(empathIcon("mini-reply.png"),
		i18n("Reply to &All"),
		this, SLOT(s_messageReplyAll()));

	messageMenu_->insertItem(empathIcon("mini-forward.png"),
		i18n("&Forward"),
		this, SLOT(s_messageForward()));

	messageMenu_->insertItem(empathIcon("mini-bounce.png"),
		i18n("&Bounce"),
		this, SLOT(s_messageBounce()));

	messageMenu_->insertItem(empathIcon("mini-delete.png"),
		i18n("&Delete"),
		this, SLOT(s_messageDelete()));

	messageMenu_->insertItem(empathIcon("mini-save.png"),
		i18n("Save &As"),
		this, SLOT(s_messageSaveAs()));
	
	messageMenu_->insertItem(empathIcon("copy.png"),
		i18n("&Copy to..."),
		this, SLOT(s_messageCopyTo()));
	
	messageMenu_->insertItem(empathIcon("move.png"),
		i18n("&Move to..."),
		this, SLOT(s_messageMoveTo()));
	
	messageMenu_->insertSeparator();
		
	messageMenu_->insertItem(empathIcon("empath-print.png"),
		i18n("&Print") + "...",
		this, SLOT(s_messagePrint()));
	
//	messageMenu_->insertItem(
//		i18n("Fil&ter"),
//		this, SLOT(s_messageFilter()));

	optionsMenu_->insertItem(empathIcon("settings-display.png"),
		i18n("&Display"),
		empath, SLOT(s_setupDisplay()));
	
	optionsMenu_->insertItem(empathIcon("settings-identity.png"),
		i18n("&Identity"),
		empath, SLOT(s_setupIdentity()));
	
	optionsMenu_->insertItem(empathIcon("settings-compose.png"),
		i18n("&Compose"),
		empath, SLOT(s_setupComposing()));
	
	optionsMenu_->insertItem(empathIcon("settings-sending.png"),
		i18n("&Sending"),
		empath, SLOT(s_setupSending()));
	
	optionsMenu_->insertItem(empathIcon("settings-accounts.png"),
		i18n("&Accounts"),
		empath, SLOT(s_setupAccounts()));
	
	optionsMenu_->insertItem(empathIcon("filter.png"),
		i18n("&Filters"),
		empath, SLOT(s_setupFilters()));
	
	helpMenu_->insertItem(
		i18n("&Contents"),
		this, SLOT(s_help()));

	helpMenu_->insertItem(i18n("&About Empath"),
		empath, SLOT(s_about()));

	helpMenu_->insertSeparator();
	
	helpMenu_->insertItem(
		i18n("About &Qt"),
		this, SLOT(s_aboutQt()));
	
	helpMenu_->insertItem(
		i18n("About &KDE"),
		kapp, SLOT(aboutKDE()));
	
	menu_->insertItem(i18n("&File"), fileMenu_);
	menu_->insertItem(i18n("&Edit"), editMenu_);
//	menu_->insertItem(i18n("F&older"), folderMenu_);
	menu_->insertItem(i18n("&Message"), messageMenu_);
	menu_->insertItem(i18n("&Options"), optionsMenu_);
	menu_->insertSeparator();
	menu_->insertItem(i18n("&Help"), helpMenu_);
}



