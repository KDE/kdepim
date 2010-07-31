/*
 *  spinbox2.cpp  -  spin box with extra pair of spin buttons (for Qt 3)
 *  Program:  kalarm
 *  Copyright © 2001-2005,2008 by David Jarvie <djarvie@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <tqglobal.h>

#include <stdlib.h>

#include <tqstyle.h>
#include <tqobjectlist.h>
#include <tqapplication.h>
#include <tqpixmap.h>
#include <tqcursor.h>
#include <tqtimer.h>
#include <tqwmatrix.h>

#include "spinbox2.moc"
#include "spinbox2private.moc"


/*  List of styles which need to display the extra pair of spin buttons as a
 *  left-to-right mirror image. This is only necessary when, for example, the
 *  corners of widgets are rounded. For most styles, it is better not to mirror
 *  the spin widgets so as to keep the normal lighting/shading on either side.
 */
static const char* mirrorStyles[] = {
	"PlastikStyle",
	0     // list terminator
};
static bool mirrorStyle(const TQStyle&);


int SpinBox2::mReverseLayout = -1;

SpinBox2::SpinBox2(TQWidget* parent, const char* name)
	: TQFrame(parent, name),
	  mReverseWithLayout(true)
{
	mUpdown2Frame = new TQFrame(this);
	mSpinboxFrame = new TQFrame(this);
	mUpdown2 = new ExtraSpinBox(mUpdown2Frame, "updown2");
//	mSpinbox = new MainSpinBox(0, 1, 1, this, mSpinboxFrame);
	mSpinbox = new MainSpinBox(this, mSpinboxFrame);
	init();
}

SpinBox2::SpinBox2(int minValue, int maxValue, int step, int step2, TQWidget* parent, const char* name)
	: TQFrame(parent, name),
	  mReverseWithLayout(true)
{
	mUpdown2Frame = new TQFrame(this);
	mSpinboxFrame = new TQFrame(this);
	mUpdown2 = new ExtraSpinBox(minValue, maxValue, step2, mUpdown2Frame, "updown2");
	mSpinbox = new MainSpinBox(minValue, maxValue, step, this, mSpinboxFrame);
	setSteps(step, step2);
	init();
}

void SpinBox2::init()
{
	if (mReverseLayout < 0)
		mReverseLayout = TQApplication::reverseLayout() ? 1 : 0;
	mMinValue      = mSpinbox->minValue();
	mMaxValue      = mSpinbox->maxValue();
	mLineStep      = mSpinbox->lineStep();
	mLineShiftStep = mSpinbox->lineShiftStep();
	mPageStep      = mUpdown2->lineStep();
	mPageShiftStep = mUpdown2->lineShiftStep();
	mSpinbox->setSelectOnStep(false);    // default
	mUpdown2->setSelectOnStep(false);    // always false
	setFocusProxy(mSpinbox);
	mUpdown2->setFocusPolicy(TQWidget::NoFocus);
	mSpinMirror = new SpinMirror(mUpdown2, mUpdown2Frame, this);
	if (!mirrorStyle(style()))
		mSpinMirror->hide();    // hide mirrored spin buttons when they are inappropriate
	connect(mSpinbox, TQT_SIGNAL(valueChanged(int)), TQT_SLOT(valueChange()));
	connect(mSpinbox, TQT_SIGNAL(valueChanged(int)), TQT_SIGNAL(valueChanged(int)));
	connect(mSpinbox, TQT_SIGNAL(valueChanged(const TQString&)), TQT_SIGNAL(valueChanged(const TQString&)));
	connect(mUpdown2, TQT_SIGNAL(stepped(int)), TQT_SLOT(stepPage(int)));
	connect(mUpdown2, TQT_SIGNAL(styleUpdated()), TQT_SLOT(updateMirror()));
}

void SpinBox2::setReadOnly(bool ro)
{
	if (static_cast<int>(ro) != static_cast<int>(mSpinbox->isReadOnly()))
	{
		mSpinbox->setReadOnly(ro);
		mUpdown2->setReadOnly(ro);
		mSpinMirror->setReadOnly(ro);
	}
}

void SpinBox2::setReverseWithLayout(bool reverse)
{
	if (reverse != mReverseWithLayout)
	{
		mReverseWithLayout = reverse;
		setSteps(mLineStep, mPageStep);
		setShiftSteps(mLineShiftStep, mPageShiftStep);
	}
}

void SpinBox2::setEnabled(bool enabled)
{
	TQFrame::setEnabled(enabled);
	updateMirror();
}

void SpinBox2::setWrapping(bool on)
{
	mSpinbox->setWrapping(on);
	mUpdown2->setWrapping(on);
}

TQRect SpinBox2::up2Rect() const
{
	return mUpdown2->upRect();
}

TQRect SpinBox2::down2Rect() const
{
	return mUpdown2->downRect();
}

void SpinBox2::setLineStep(int step)
{
	mLineStep = step;
	if (reverseButtons())
		mUpdown2->setLineStep(step);   // reverse layout, but still set the right buttons
	else
		mSpinbox->setLineStep(step);
}

void SpinBox2::setSteps(int line, int page)
{
	mLineStep = line;
	mPageStep = page;
	if (reverseButtons())
	{
		mUpdown2->setLineStep(line);   // reverse layout, but still set the right buttons
		mSpinbox->setLineStep(page);
	}
	else
	{
		mSpinbox->setLineStep(line);
		mUpdown2->setLineStep(page);
	}
}

void SpinBox2::setShiftSteps(int line, int page)
{
	mLineShiftStep = line;
	mPageShiftStep = page;
	if (reverseButtons())
	{
		mUpdown2->setLineShiftStep(line);   // reverse layout, but still set the right buttons
		mSpinbox->setLineShiftStep(page);
	}
	else
	{
		mSpinbox->setLineShiftStep(line);
		mUpdown2->setLineShiftStep(page);
	}
}

void SpinBox2::setButtonSymbols(TQSpinBox::ButtonSymbols newSymbols)
{
	if (mSpinbox->buttonSymbols() == newSymbols)
		return;
	mSpinbox->setButtonSymbols(newSymbols);
	mUpdown2->setButtonSymbols(newSymbols);
}

int SpinBox2::bound(int val) const
{
	return (val < mMinValue) ? mMinValue : (val > mMaxValue) ? mMaxValue : val;
}

void SpinBox2::setMinValue(int val)
{
	mMinValue = val;
	mSpinbox->setMinValue(val);
	mUpdown2->setMinValue(val);
}

void SpinBox2::setMaxValue(int val)
{
	mMaxValue = val;
	mSpinbox->setMaxValue(val);
	mUpdown2->setMaxValue(val);
}

void SpinBox2::valueChange()
{
	int val = mSpinbox->value();
	bool blocked = mUpdown2->signalsBlocked();
	mUpdown2->blockSignals(true);
	mUpdown2->setValue(val);
	mUpdown2->blockSignals(blocked);
}

/******************************************************************************
* Called when the widget is about to be displayed.
* (At construction time, the spin button widths cannot be determined correctly,
*  so we need to wait until now to definitively rearrange the widget.)
*/
void SpinBox2::showEvent(TQShowEvent*)
{
	arrange();
}

TQSize SpinBox2::sizeHint() const
{
	getMetrics();
	TQSize size = mSpinbox->sizeHint();
	size.setWidth(size.width() - xSpinbox + wUpdown2 + wGap);
	return size;
}

TQSize SpinBox2::minimumSizeHint() const
{
	getMetrics();
	TQSize size = mSpinbox->minimumSizeHint();
	size.setWidth(size.width() - xSpinbox + wUpdown2 + wGap);
	return size;
}

void SpinBox2::styleChange(TQStyle&)
{
	if (mirrorStyle(style()))
		mSpinMirror->show();    // show rounded corners with Plastik etc.
	else
		mSpinMirror->hide();    // keep normal shading with other styles
	arrange();
}

/******************************************************************************
* Called when the extra pair of spin buttons has repainted after a style change. 
* Updates the mirror image of the spin buttons.
*/
void SpinBox2::updateMirror()
{
	mSpinMirror->setNormalButtons(TQPixmap::grabWidget(mUpdown2Frame, 0, 0));
}

/******************************************************************************
* Set the positions and sizes of all the child widgets. 
*/
void SpinBox2::arrange()
{
	getMetrics();
	TQRect arrowRect = TQStyle::visualRect(TQRect(0, 0, wUpdown2, height()), this);
	mUpdown2Frame->setGeometry(arrowRect);
	mUpdown2->setGeometry(-xUpdown2, 0, mUpdown2->width(), height());
	mSpinboxFrame->setGeometry(TQStyle::visualRect(TQRect(wUpdown2 + wGap, 0, width() - wUpdown2 - wGap, height()), this));
	mSpinbox->setGeometry(-xSpinbox, 0, mSpinboxFrame->width() + xSpinbox, height());
	mSpinMirror->resize(wUpdown2, mUpdown2->height());
	mSpinMirror->setGeometry(arrowRect);
//mSpinMirror->setGeometry(TQStyle::visualRect(TQRect(0, 11, wUpdown2, height()), this));
	mSpinMirror->setNormalButtons(TQPixmap::grabWidget(mUpdown2Frame, 0, 0));
}

/******************************************************************************
* Calculate the width and position of the extra pair of spin buttons.
* Style-specific adjustments are made for a better appearance. 
*/
void SpinBox2::getMetrics() const
{
	TQRect rect = mUpdown2->style().querySubControlMetrics(TQStyle::CC_SpinWidget, mUpdown2, TQStyle::SC_SpinWidgetButtonField);
	if (style().inherits("PlastikStyle"))
		rect.setLeft(rect.left() - 1);    // Plastik excludes left border from spin widget rectangle
	xUpdown2 = mReverseLayout ? 0 : rect.left();
	wUpdown2 = mUpdown2->width() - rect.left();
	xSpinbox = mSpinbox->style().querySubControlMetrics(TQStyle::CC_SpinWidget, mSpinbox, TQStyle::SC_SpinWidgetEditField).left();
	wGap = 0;

	// Make style-specific adjustments for a better appearance
	if (style().inherits("TQMotifPlusStyle"))
	{
		xSpinbox = 0;      // show the edit control left border
		wGap = 2;          // leave a space to the right of the left-hand pair of spin buttons
	}
}

/******************************************************************************
* Called when the extra pair of spin buttons is clicked to step the value.
* Normally this is a page step, but with a right-to-left language where the
* button functions are reversed, this is a line step.
*/
void SpinBox2::stepPage(int step)
{
	if (abs(step) == mUpdown2->lineStep())
		mSpinbox->setValue(mUpdown2->value());
	else
	{
		// It's a shift step
		int oldValue = mSpinbox->value();
		if (!reverseButtons())
		{
			// The button pairs have the normal function.
			// Page shift stepping - step up or down to a multiple of the
			// shift page increment, leaving unchanged the part of the value
			// which is the remainder from the page increment.
			if (oldValue >= 0)
				oldValue -= oldValue % mUpdown2->lineStep();
			else
				oldValue += (-oldValue) % mUpdown2->lineStep();
		}
		int adjust = mSpinbox->shiftStepAdjustment(oldValue, step);
		if (adjust == -step
		&&  ((step > 0  &&  oldValue + step >= mSpinbox->maxValue())
		  || (step < 0  &&  oldValue + step <= mSpinbox->minValue())))
			adjust = 0;    // allow stepping to the minimum or maximum value
		mSpinbox->addValue(adjust + step);
	}
	bool focus = mSpinbox->selectOnStep() && mUpdown2->hasFocus();
	if (focus)
		mSpinbox->selectAll();

	// Make the covering arrows image show the pressed arrow
	mSpinMirror->redraw(TQPixmap::grabWidget(mUpdown2Frame, 0, 0));
}


/*=============================================================================
= Class SpinBox2::MainSpinBox
=============================================================================*/

/******************************************************************************
* Return the initial adjustment to the value for a shift step up or down, for
* the main (visible) spin box.
* Normally this is a line step, but with a right-to-left language where the
* button functions are reversed, this is a page step.
*/
int SpinBox2::MainSpinBox::shiftStepAdjustment(int oldValue, int shiftStep)
{
	if (owner->reverseButtons())
	{
		// The button pairs have the opposite function from normal.
		// Page shift stepping - step up or down to a multiple of the
		// shift page increment, leaving unchanged the part of the value
		// which is the remainder from the page increment.
		if (oldValue >= 0)
			oldValue -= oldValue % lineStep();
		else
			oldValue += (-oldValue) % lineStep();
	}
	return SpinBox::shiftStepAdjustment(oldValue, shiftStep);
}


/*=============================================================================
= Class ExtraSpinBox
=============================================================================*/

/******************************************************************************
* Repaint the widget.
* If it's the first time since a style change, tell the parent SpinBox2 to
* update the SpinMirror with the new unpressed button image. We make the
* presumably reasonable assumption that when a style change occurs, the spin
* buttons are unpressed.
*/
void ExtraSpinBox::paintEvent(TQPaintEvent* e)
{
	SpinBox::paintEvent(e);
	if (mNewStylePending)
	{
		mNewStylePending = false;
		emit styleUpdated();
	}
}


/*=============================================================================
= Class SpinMirror
=============================================================================*/

SpinMirror::SpinMirror(SpinBox* spinbox, TQFrame* spinFrame, TQWidget* parent, const char* name)
	: TQCanvasView(new TQCanvas, parent, name),
	  mSpinbox(spinbox),
	  mSpinFrame(spinFrame),
	  mReadOnly(false)
{
	setVScrollBarMode(TQScrollView::AlwaysOff);
	setHScrollBarMode(TQScrollView::AlwaysOff);
	setFrameStyle(TQFrame::NoFrame);

	// Find the spin widget which is part of the spin box, in order to
	// pass on its shift-button presses.
	TQObjectList* spinwidgets = spinbox->queryList("TQSpinWidget", 0, false, true);
	mSpinWidget = (SpinBox*)spinwidgets->getFirst();
	delete spinwidgets;
}

void SpinMirror::setNormalButtons(const TQPixmap& px)
{
	mNormalButtons = px;
	redraw(mNormalButtons);
}

void SpinMirror::redraw()
{
	redraw(TQPixmap::grabWidget(mSpinFrame, 0, 0));
}

void SpinMirror::redraw(const TQPixmap& px)
{
	TQCanvas* c = canvas();
	c->setBackgroundPixmap(px);
	c->setAllChanged();
	c->update();
}

void SpinMirror::resize(int w, int h)
{
	canvas()->resize(w, h);
	TQCanvasView::resize(w, h);
	resizeContents(w, h);
	setWorldMatrix(TQWMatrix(-1, 0, 0, 1, w - 1, 0));  // mirror left to right
}

/******************************************************************************
* Pass on all mouse events to the spinbox which we're covering up.
*/
void SpinMirror::contentsMouseEvent(TQMouseEvent* e)
{
	if (mReadOnly)
		return;
	TQPoint pt = contentsToViewport(e->pos());
	pt.setX(pt.x() + mSpinbox->upRect().left());
	TQApplication::postEvent(mSpinWidget, new TQMouseEvent(e->type(), pt, e->button(), e->state()));

	// If the mouse button has been pressed or released, refresh the spin buttons
	switch (e->type())
	{
		case TQEvent::MouseButtonPress:
		case TQEvent::MouseButtonRelease:
			TQTimer::singleShot(0, this, TQT_SLOT(redraw()));
			break;
		default:
			break;
	}
}

/******************************************************************************
* Pass on all mouse events to the spinbox which we're covering up.
*/
void SpinMirror::contentsWheelEvent(TQWheelEvent* e)
{
	if (mReadOnly)
		return;
	TQPoint pt = contentsToViewport(e->pos());
	pt.setX(pt.x() + mSpinbox->upRect().left());
	TQApplication::postEvent(mSpinWidget, new TQWheelEvent(pt, e->delta(), e->state(), e->orientation()));
}

/******************************************************************************
* Pass on to the main spinbox events which are needed to activate mouseover and
* other graphic effects when the mouse cursor enters and leaves the widget.
*/
bool SpinMirror::event(TQEvent* e)
{
	switch (e->type())
	{
		case TQEvent::Leave:
		case TQEvent::Enter:
			TQApplication::postEvent(mSpinWidget, new TQEvent(e->type()));
			TQTimer::singleShot(0, this, TQT_SLOT(redraw()));
			break;
		case TQEvent::FocusIn:
			mSpinbox->setFocus();
			TQTimer::singleShot(0, this, TQT_SLOT(redraw()));
			break;
		default:
			break;
	}
	return TQCanvasView::event(e);
}


/*=============================================================================
= Local functions
=============================================================================*/

/******************************************************************************
* Determine whether the extra pair of spin buttons needs to be mirrored
* left-to-right in the specified style.
*/
static bool mirrorStyle(const TQStyle& style)
{
	for (const char** s = mirrorStyles;  *s;  ++s)
		if (style.inherits(*s))
			return true;
	return false;
}
