/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/


void Kab3MainWidgetBase::setReadOnly( bool rostate )
{
    if(rostate!=m_ro)
    {
	m_ro=rostate;
	emit(readonlyChanged(m_ro));
    }
}

void Kab3MainWidgetBase::slotContactSelected( QListViewItem *)
{

}


void Kab3MainWidgetBase::slotSearchModified( const QString & )
{
    pbClearSearch->setEnabled(!leSearch->text().isEmpty());
}

void Kab3MainWidgetBase::slotClearSearch()
{
    leSearch->clear();
    pbClearSearch->setEnabled(false);
}


void Kab3MainWidgetBase::slotPatternEntered( const QString & )
{

}

