#include <kmenubar.h>

void
EmpathComposeWindow::setupMenuBar()
{
    fileMenu_    = new QPopupMenu();
    CHECK_PTR(fileMenu_);

    editMenu_    = new QPopupMenu();
    CHECK_PTR(editMenu_);
    
    attachmentMenu_    = new QPopupMenu();
    CHECK_PTR(attachmentMenu_);

    messageMenu_    = new QPopupMenu();
    CHECK_PTR(messageMenu_);
    
    helpMenu_    = new QPopupMenu();
    CHECK_PTR(helpMenu_);

    // File menu
    
    fileMenu_->insertItem(empathIcon("mini-send"), i18n("&Send Message"),
        this, SLOT(s_fileSendMessage()));
    
    fileMenu_->insertItem(empathIcon("mini-sendlater"), i18n("Send &Later"),
        this, SLOT(s_fileSendLater()));
    
    fileMenu_->insertSeparator();

    fileMenu_->insertItem(empathIcon("blank"), i18n("&Close"),
        this, SLOT(s_fileClose()));

    // Edit menu
    
#if 0
    editMenu_->insertItem(i18n("&Undo"),
        this, SLOT(s_editUndo()));
    
    editMenu_->insertItem(i18n("&Redo"),
        this, SLOT(s_editRedo()));
    
    editMenu_->insertSeparator();
#endif
    editMenu_->insertItem(empathIcon("empath-cut"), i18n("Cu&t"),
        this, SLOT(s_editCut()));
    
    editMenu_->insertItem(empathIcon("empath-copy"), i18n("&Copy"),
        this, SLOT(s_editCopy()));
    
    editMenu_->insertItem(empathIcon("empath-paste"), i18n("&Paste"),
        this, SLOT(s_editPaste()));
    
//    editMenu_->insertItem(empathIcon("blank"), i18n("&Delete"),
//        this, SLOT(s_editDelete()));

    editMenu_->insertSeparator();
    
    editMenu_->insertItem(empathIcon("blank"), i18n("&Select All"),
        this, SLOT(s_editSelectAll()));
    
#if 0
    editMenu_->insertSeparator();
    
    editMenu_->insertItem(empathIcon("blank"), i18n("Find..."),
        this, SLOT(s_editFind()));
    
    editMenu_->insertItem(empathIcon("blank"), i18n("Find &Again"),
        this, SLOT(s_editFindAgain()));
#endif
    
    // Attachment menu
    
    attachmentMenu_->insertItem(empathIcon(""), i18n("&Add attachment"),
        composeWidget_, SLOT(s_addAttachment()));
    
    attachmentMenu_->insertItem(empathIcon(""), i18n("&Edit attachment"),
        composeWidget_, SLOT(s_editAttachment()));
    
    attachmentMenu_->insertItem(empathIcon(""), i18n("&Remove attachment"),
        composeWidget_, SLOT(s_removeAttachment()));
    
    // Message Menu
    messageMenu_->insertItem(empathIcon("mini-compose"), i18n("&New"),
        empath, SLOT(s_compose()));

    messageMenu_->insertItem(empathIcon("mini-save"), i18n("Save &As"),
        this, SLOT(s_messageSaveAs()));

    messageMenu_->insertItem(empathIcon("copy"), i18n("&Copy to..."),
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
    menuBar()->insertItem(i18n("&Attachment"), attachmentMenu_);
    menuBar()->insertItem(i18n("&Message"), messageMenu_);
    menuBar()->insertSeparator();
    menuBar()->insertItem(i18n("&Help"), helpMenu_);
}


// vim:ts=4:sw=4:tw=78
