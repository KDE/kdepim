
    buttonBox_  = new KButtonBox(this);
    
    pb_help_    = buttonBox_->addButton(i18n("&Help"));    
    pb_default_ = buttonBox_->addButton(i18n("&Default"));    
    buttonBox_->addStretch();
    pb_OK_      = buttonBox_->addButton(i18n("&OK"));
    pb_apply_   = buttonBox_->addButton(i18n("&Apply"));
    pb_cancel_  = buttonBox_->addButton(i18n("&Cancel"));
    
    pb_OK_->setDefault(true);

    buttonBox_->layout();
    
    buttonBox_->setFixedHeight(buttonBox_->sizeHint().height());

    QObject::connect(pb_OK_,        SIGNAL(clicked()),  SLOT(s_OK()));
    QObject::connect(pb_default_,   SIGNAL(clicked()),  SLOT(s_default()));
    QObject::connect(pb_apply_,     SIGNAL(clicked()),  SLOT(s_apply()));
    QObject::connect(pb_cancel_,    SIGNAL(clicked()),  SLOT(s_cancel()));
    QObject::connect(pb_help_,      SIGNAL(clicked()),  SLOT(s_help()));
