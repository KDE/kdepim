// Derived hijri kde calendar class
// Just a schema. It must be a plugin
// It depends on hdate routines
// Carlos Moro  <cfmoro@correo.uniovi.es>
// GNU-GPL v.2

#include "calhijri.h"
#include <qdatetime.h>
#include <kglobal.h>
#include <klocale.h>
#include <qstring.h>
#include <qstringlist.h>
#include "hconv.h"
#include <kdebug.h>


KCalendarHijri::KCalendarHijri()  {
	kdDebug() << "\nCreated hijri calendar" << endl;
};

QString KCalendarHijri::getMonth(const QDate& date) {
	kdDebug() << "Arabic month..." <<  endl;
	// firstly, get hijri date from gregorian one
	SDATE	*sd;
	sd = hdate(date.year(), date.month(), date.day());
	return QString(hmname[sd->mon-1]);
}

int KCalendarHijri::getYear(const QDate& date) {
	kdDebug() << "Arabic year..." <<  endl;
	// firstly, get hijri date from gregorian one
	SDATE	*sd;
	sd = hdate(date.year(), date.month(), date.day());
	if (sd->year>0)
		return (sd->year);
	else
		return (-sd->year);

}

QString KCalendarHijri::getFormatDate(const QDate& date) {
	kdDebug() << "Arabic format date..." << endl;
	// firstly, get hijri date from gregorian one
	SDATE	*sd;
	sd = hdate(date.year(), date.month(), date.day());
	return QString::number(sd->day) + QString("/") +
		QString::number(sd->mon) + QString("/")+
		QString::number(sd->year);
}


void KCalendarHijri::getNextMonthDate(QDate& date) {
	// firstly, get hijri date from gregorian one
	SDATE	*sd, *gd;
	sd = hdate(date.year(), date.month(), date.day());
	// hijri + 1 month
	int day = sd->day;
	if(sd->mon==12) {
		//setYMD(temp.year()+1, 1, 1);
		sd->year+=1;
		sd->mon=1;
		sd->day=1;
	} else {
		//temp.setYMD(temp.year(), temp.month()+1, 1);
		sd->mon+=1;
	}
	if(hndays(sd->mon, sd->year) < day) {
		//temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
		sd->day = hndays(sd->mon, sd->year);
	} else {
		//temp.setYMD(temp.year(), temp.month(), day);
		sd->day = day;
	}
	// get back gregorian date from new hijri
	gd = gdate( sd -> year, sd -> mon, sd -> day);
	date.setYMD(gd->year, gd->mon, gd->day);
	kdDebug() << "Arabic next month date..." << endl;
}

void KCalendarHijri::getPreviousMonthDate(QDate& date) {
	kdDebug() << "Arabic previous month date..." << endl;
	// firstly, get hijri date from gregorian one
	SDATE	*sd, *gd;
	sd = hdate(date.year(), date.month(), date.day());
	// hijri - 1 month
	int day = sd->day;
	if(sd->mon==1) {
		//temp.setYMD(temp.year()-1, 1, 1);
		sd->year-=1;
		sd->mon = 1;
		sd->day = 1;
	} else {
		//temp.setYMD(temp.year(), temp.month()-1, 1);
		sd->mon-=1;
		sd->day = 1;
	}
	if(hndays(sd->mon, sd->year)<day) {
		//temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
		sd->day = hndays(sd->mon, sd->year);
	} else {
		//temp.setYMD(temp.year(), temp.month(), day);
		sd->day = day;
	}
	// get back gregorian date from new hijri
	gd = gdate( sd -> year, sd -> mon, sd -> day);
	date.setYMD(gd->year, gd->mon, gd->day);
}


void KCalendarHijri::getNextYearDate(QDate& date) {
	kdDebug() << "Arabic next year date..." << endl;
	// firstly, get hijri date from gregorian one
	SDATE	*sd, *gd;
	sd = hdate(date.year(), date.month(), date.day());
	// hijri + 1 year
	int day = sd->day;
	//temp.setYMD(temp.year()+1, temp.month(), 1);
	sd->year+=1;
	sd->day=1;
	if(hndays(sd->mon, sd->year)<day) {
		//temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
	} else {
		//temp.setYMD(temp.year(), temp.month(), day);
		sd->day = day;
	}
	// get back gregorian date from new hijri
	gd = gdate( sd -> year, sd -> mon, sd -> day);
	date.setYMD(gd->year, gd->mon, gd->day);
}

void KCalendarHijri::getPreviousYearDate(QDate& date) {
	kdDebug() << "Arabic previous year date..." << endl;
	// firstly, get hijri date from gregorian one
	SDATE	*sd, *gd;
	sd = hdate(date.year(), date.month(), date.day());
	// hijri - 1 year
	int day = sd->day;
	//temp.setYMD(temp.year()-1, temp.month(), 1);
	sd->year-=1;
	sd->day=1;
	if(hndays(sd->mon, sd->year)<day) {
		//temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
	} else {
		//temp.setYMD(temp.year(), temp.month(), day);
		sd->day = day;
	}
	// get back gregorian date from new hijri
	gd = gdate( sd -> year, sd -> mon, sd -> day);
	date.setYMD(gd->year, gd->mon, gd->day);
}

int KCalendarHijri::monthsInYear(int year) {
	kdDebug() << "Arabic monthsInYear" << endl;
	return 12;
}


QString KCalendarHijri::getMonthName(int month) {

	kdDebug() << "Arabic getMonthName " << hmname[month-1] <<endl;
	//return KGlobal::locale()->monthName(month, false);
	return QString(hmname[month-1]);
}



void KCalendarHijri::constructDateInMonth(QDate& date, int month) {
	kdDebug() << "Arabic constructDateInMonth" << endl;
	// firstly, get hijri date from gregorian one
	SDATE	*sd, *gd;
	sd = hdate(date.year(), date.month(), date.day());
	// construct
	int day;
	day = sd->day;

	//date.setYMD(date.year(), month, 1);
	sd->mon = month;
	sd->day = 1;
	//date.setYMD(date.year(), month, QMIN(day, date.daysInMonth()));
	sd->mon = month;
	sd->day = QMIN(day, hndays(sd->mon, sd->year));
	// get back gregorian date from new hijri
	gd = gdate( sd -> year, sd -> mon, sd -> day);
	date.setYMD(gd->year, gd->mon, gd->day);
}

void KCalendarHijri::constructDateInYear(QDate& date, int year) {
	kdDebug() << "Hijri constructDateInYear" << endl;
	// firstly, get hijri date from gregorian one
	SDATE	*sd, *gd;
	sd = hdate(date.year(), date.month(), date.day());
	// construct
	int day;
	day = date.day();

	//date.setYMD(year, date.month(), 1);
	sd->year = year;
	sd->day =1;
	//date.setYMD(year, date.month(), QMIN(day, date.daysInMonth()));
	sd->year = year;
	sd->day = QMIN(day, hndays(sd->mon, sd->year));
	// get back gregorian date from new hijri
	gd = gdate( sd -> year, sd -> mon, sd -> day);
	date.setYMD(gd->year, gd->mon, gd->day);
}


QDate KCalendarHijri::parseDate(QString text) {
	kdDebug() << "Hijri parseDate" << endl;
	// entered values belong to hijri date...
	kdDebug() << "parseDate text " << text << endl;
	int day, month, year;
	QStringList d = QStringList::split("/", text);
	QStringList::Iterator it;
	QString a;

	it = d.at(0);
	a = *it;
	day = a.toInt();

	it = d.at(1);
	a = *it;
	month = a.toInt();

	it = d.at(2);
	a = *it;
	year = a.toInt();

	// convert to gregorian...
	SDATE *gd;
	gd = gdate(year, month, day);
	QDate enteredDate;
	enteredDate.setYMD(gd->year, gd->mon, gd->day);
	kdDebug() << "DEBUG year: " << enteredDate.year() << endl;
	return enteredDate;

}


QString KCalendarHijri::wDayName(int day) {
	//kdDebug() << "Hijri wDayName" << endl;
	return QString( sdow[day-1] );
}


int KCalendarHijri::dayOfTheWeek(const QDate& date) {
	//kdDebug() << "Hijri dayOfTheWeek" << endl;
	SDATE	*sd, *gd, *sd1;
	// firstly, we get hijri date from actual gregorian date
	sd = hdate(date.year(), date.month(), date.day());
	//kdDebug() << "day " << sd->day << " dayOfWeek " << sd->dw << endl;
	// then lets, create gregorian date for the first day of that hijri month
	gd = gdate(sd->year, sd->mon, 1);
	// ...and oooops, there's the first day of month hijri date
	sd1 = hdate(gd->year, gd->mon, gd->day);
	//kdDebug() << "day " << sd1->day << " dayOfWeek " << sd1->dw << endl;
	return (sd1->dw)+1;
}

int KCalendarHijri::numberOfDaysInMonth(const QDate& date) {
	kdDebug() << "Hijri daysInMonth" << endl;
	SDATE	*sd;
	// firstly, we get hijri date from actual gregorian date
	sd = hdate(date.year(), date.month(), date.day());
	return hndays(sd->mon, sd->year);
}

// Not overloaded, just in this calendar system class
int KCalendarHijri::hndays(int mon,int year) {
	SDATE fd, ld;
	int nd = 666;
	fd = *gdate(year, mon, 1);
	ld = *gdate(year, mon+1, 1);
	ld = *caldate(julianday(ld.year, ld.mon, ld.day, 0.0) -1.0);
	if (fd.mon == ld.mon)
		nd = ld.day - fd.day +1;
	else
		nd = ndays(fd.mon,fd.year) - fd.day + ld.day +1;

	return nd;

}


int KCalendarHijri::numberOfDaysPrevMonth(const QDate& date) {
	kdDebug() << "Hijri daysinprevmonth" << endl;
	SDATE	*sd, *gd, *sd1;
	// firstly, we get hijri date from actual gregorian date
	sd = hdate(date.year(), date.month(), date.day());

	if(sd->mon == 1)
	{
		//temp.setYMD(date.year()-1, 12, 1);
		gd = gdate((sd->year)-1, 12, 1);

	} else {
		//temp.setYMD(date.year(), date.month()-1, 1);
		gd = gdate(sd->year, (sd->mon)-1, 1);
	}
	sd1 = hdate(gd->year, gd->mon, gd->day);
	return hndays( sd1->mon, sd1->year);
}

// Max valid year that may be converted to QDate
int KCalendarHijri::getMaxValidYear() {
	QDate date(8000, 1, 1);

	SDATE	*sd;
	// firstly, we get hijri date from gregorian date
	sd = hdate(date.year(), date.month(), date.day());

	return sd->year;
}


int KCalendarHijri::getDay(const QDate& date) {
	SDATE	*sd;
	// firstly, we get hijri date from actual gregorian date
	sd = hdate(date.year(), date.month(), date.day());

	return sd->day;
}

void KCalendarHijri::printType() {
	kdDebug() << "It's hijri!" << endl;
}


KCalendarHijri::~KCalendarHijri() {};


// Best regards ;)
