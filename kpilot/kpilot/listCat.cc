#include <kdebug.h>
#include <klocale.h>
#include "listCat.moc"

ListCategorizer::ListCategorizer(QWidget *parent, const char *name) :
	KListView(parent,name),
	fStartOpen(false)
{
	setupWidget();
}

ListCategorizer::ListCategorizer(const QStringList& i,
	bool startOpen,
	QWidget *parent,
	const char *name) :
	KListView(parent,name),
	fStartOpen(startOpen)
{
	addCategories(i);
}

void ListCategorizer::addCategories(const QStringList& l)
{
	QStringList::ConstIterator i;

	for (i=l.begin(); i!=l.end(); ++i)
	{
		(void) addCategory(*i);
	}
}

QListViewItem *ListCategorizer::addCategory(const QString& name, 
	const QString& desc)
{
	QListViewItem *m = new QListViewItem(this,name,desc);
	m->setSelectable(false);
	m->setOpen(fStartOpen);
	return m;
}
		
void ListCategorizer::setupWidget()
{
	addColumn(i18n("Category"));
	addColumn(i18n("Description"));
	setItemsMovable(false);
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropVisualizer(false);
	setRootIsDecorated(true);
}

/* virtual */ bool ListCategorizer::acceptDrag(QDropEvent* event) const
{
	if (!(event->source())) return false;
	QListViewItem *p = itemAt(event->pos());
	if (!p) return false;

	return true;
}

/* virtual */ void ListCategorizer::contentsDropEvent (QDropEvent*e)
{
	cleanDropVisualizer();

	if (!acceptDrag(e)) return;
	e->accept();

	QListViewItem *p = itemAt(e->pos());
	QListViewItem *selection = currentItem();
	if (!p) 
	{
		kdDebug()  << "Drop without a category!"
			<< endl;
		return;
	}

	QListViewItem *category = p->parent();
	if (!category)
	{
		category = p;
	}

	moveItem(selection,category,0L);
}

/* virtual */ void ListCategorizer::startDrag()
{
	QListViewItem *p = currentItem();

	if (!p || !p->parent()) return;

	KListView::startDrag();
}

QStringList ListCategorizer::listSiblings(const QListViewItem *p,
	int column) const
{
	QStringList l;

	while(p)
	{
		l.append(p->text(column));
		p = p->nextSibling();
	}

	return l;
}

QListViewItem *ListCategorizer::findCategory(const QString& category) const
{
	QListViewItem *p = firstChild();

	while (p)
	{
		if (p->text(0) == category) return p;
		p = p->nextSibling();
	}

	return 0L;
}

QListViewItem *ListCategorizer::addItem(const QString& category,
	const QString& name,
	const QString& description)
{
	QListViewItem *p = findCategory(category);

	if (!p) return 0L;

	return new QListViewItem(p,name,description);
}


