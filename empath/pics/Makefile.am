KDE_ICON = empath empath_browse empath_compose empath_confirm_delivery empath_confirm_read \
	empath_filter empath_folder_drafts empath_folder_inbox empath_folder_normal \
	empath_folder_outbox empath_folder_sent empath_folder_trash empath_logo empath_mailbox \
	empath_message_bounce empath_message_compose empath_message_copy empath_message_digsign \
	empath_message_encrypt empath_message_filter empath_message_forward empath_message_move \
	empath_message_print empath_message_remove empath_message_reply empath_message_reply_all \
	empath_message_save empath_message_send empath_message_send_later empath_message_sign \
	empath_message_view empath_point empath_settings_accounts empath_settings_compose \
	empath_settings_display empath_settings_identity empath_settings_sending \
	empath_tree_attachments empath_tree_marked empath_tree_read empath_tree_replied \
	empath_view_attachment empath_view_attachment_active empath_view_hide_read empath_view_thread

install-data-local: 
	$(mkinstalldirs) $(kde_datadir)/empath
	$(INSTALL_DATA) $(srcdir)/empath_logo.png $(kde_datadir)/empath/empath_logo.png

