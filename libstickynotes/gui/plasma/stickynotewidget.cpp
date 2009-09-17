#include "stickynotewidget.h"

#include <StickyNotes/AbstractNoteItem>

#include <Plasma/Label>

#include <QGraphicsLinearLayout>
#include <QGraphicsSceneResizeEvent>
#include <QLabel>
#include <QPainter>

#include "stickynoteitem.h"

StickyNoteWidget::StickyNoteWidget(StickyNoteItem &_item)
: Plasma::Applet(0, QVariantList()), m_content(0), m_item(&_item),
    m_layout(0), m_readonly(true), m_subject(0), m_theme(0)
{
	setAcceptsHoverEvents(true);
	setAspectRatioMode(Plasma::IgnoreAspectRatio);
	setBackgroundHints(Plasma::Applet::NoBackground);
  
	connect(m_item, SIGNAL(appliedAttribute(const QString &, const QVariant &)),
	    this, SLOT(on_item_appliedAttribute(const QString &, const QVariant &)));
	connect(m_item, SIGNAL(appliedContent(const QString &)),
	    this, SLOT(on_item_appliedContent(const QString &)));
	connect(m_item, SIGNAL(appliedSubject(const QString &)),
	    this, SLOT(on_item_appliedSubject(const QString &)));
	connect(m_item, SIGNAL(bound(void)),
	    this, SLOT(on_item_bound(void)));

	setupUi();
}
 
StickyNoteWidget::~StickyNoteWidget(void)
{
	delete m_theme;
	delete m_subject;
	delete m_content;
}

void
StickyNoteWidget::destroy(void)
{
	m_item->setAttribute("hidden", true);

	hide();
}
 
void
StickyNoteWidget::init(void)
{
}
 
void
StickyNoteWidget::paintInterface(QPainter *_painter,
    const QStyleOptionGraphicsItem *_option, const QRect &_rect)
{
	Q_UNUSED(_option);
	Q_UNUSED(_rect);

	_painter->setRenderHint(QPainter::SmoothPixmapTransform); 
	_painter->setRenderHint(QPainter::Antialiasing); 

	_painter->save();
	m_theme->paintFrame(_painter);
	_painter->restore();
}

bool
StickyNoteWidget::eventFilter(QObject *_watched, QEvent *_event)
{
	QMouseEvent *me;

	if ((me = dynamic_cast<QMouseEvent *>(_event))) {
			switch (me->type()) {
				case QEvent::MouseButtonDblClick:
					if (!m_readonly)
						m_item->edit();
					break;
				default: break;
			}
	} else {
		return (Plasma::Applet::eventFilter(_watched, _event));
	}

	return (false);
}

void
StickyNoteWidget::resizeEvent(QGraphicsSceneResizeEvent *_event) 
{
	Plasma::Applet::resizeEvent(_event);
	m_theme->resizeFrame(_event->newSize());
}

void
StickyNoteWidget::setupUi(void)
{
	m_content = new Plasma::Label(this);
	m_content->nativeWidget()->installEventFilter(this);
	m_content->nativeWidget()->setTextFormat(Qt::RichText);
	m_content->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_content->setScaledContents(true);
	{
		QPalette cp = m_content->nativeWidget()->palette();

		cp.setColor(QPalette::Active, QPalette::WindowText,
		    QColor(55,55,0));
		cp.setColor(QPalette::Inactive, QPalette::WindowText,
		    QColor(105,105,4));

		m_content->nativeWidget()->setPalette(cp);
	}

	m_subject = new Plasma::Label(this);
	m_subject->nativeWidget()->installEventFilter(this);
	m_subject->nativeWidget()->setTextFormat(Qt::RichText);
	m_subject->setAlignment(Qt::AlignRight | Qt::AlignBottom);
	{
		QFont cf    = m_subject->nativeWidget()->font();
		QPalette cp = m_subject->nativeWidget()->palette();

		cf.setPointSize(cf.pointSize() - 2);
		cp.setColor(QPalette::Active, QPalette::WindowText,
		    QColor(105,105,4));
		cp.setColor(QPalette::Inactive, QPalette::WindowText,
		    QColor(185,185,84));

		m_subject->nativeWidget()->setFont(cf);
		m_subject->nativeWidget()->setPalette(cp);
	}

	m_theme = new Plasma::FrameSvg(this);
	m_theme->setImagePath("widgets/stickynote");
	m_theme->setEnabledBorders(Plasma::FrameSvg::AllBorders);

	m_layout = new QGraphicsLinearLayout;
	m_layout->setContentsMargins(9,9,9,9);
	m_layout->setOrientation(Qt::Vertical);
	m_layout->setSpacing(6);
	m_layout->addItem(m_subject);
	m_layout->addItem(m_content);
	m_layout->setStretchFactor(m_subject, 50);
	m_layout->setStretchFactor(m_content, 220);

	setLayout(m_layout);

	hide();

	// TODO: Size and Position should come out of the meta data
	resize(200, 200);
}

void
StickyNoteWidget::on_item_appliedAttribute(const QString &_name, const QVariant &_value)
{
	if (0 == _name.compare("hidden"))
		setVisible(!_value.toBool());
	else if (0 == _name.compare("readonly"))
		m_readonly = _value.toBool();
}

void
StickyNoteWidget::on_item_appliedContent(const QString &_content)
{
	m_content->setText(_content);
}

void
StickyNoteWidget::on_item_appliedSubject(const QString &_subject)
{
	m_subject->setText(_subject);
}

void
StickyNoteWidget::on_item_bound(void)
{
	m_readonly = m_item->attribute("readonly").toBool();

	m_subject->setText(m_item->subject());
	m_content->setText(m_item->content());

	// Temporary fix (no data on labels no mouse events)
	if (m_subject->text().isEmpty())
		m_subject->setText("&nbsp;");
	if (m_content->text().isEmpty())
		m_content->setText("&nbsp;");

	setVisible(!m_item->attribute("hidden").toBool());
}

void
StickyNoteWidget::on_item_unbound(void)
{
	hide();
}

