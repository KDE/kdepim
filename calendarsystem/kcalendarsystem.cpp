// Gregorian calendar system implementation factory for creation of kde calendar systems
// Also default gregorian and factory classes
// Carlos Moro, <cfmoro@correo.uniovi.es>
// GNU-GPL v.2

#include "kcalendarsystem.h"
#include "calhijri.h"
#include <qdatetime.h>
#include <kglobal.h>
#include <klocale.h>
#include <qstring.h>
#include <kdebug.h>


KCalendarGregorian::KCalendarGregorian() {
	kdDebug() << "\nCreated gregorian calendar" << endl;
};

QString KCalendarGregorian::getMonth(const QDate& date) {
	kdDebug() << "Gregorian month..." << endl;
	QString q = KGlobal::locale()->monthName(date.month(),false) ;

	return q;
}

QString KCalendarGregorian::getFormatDate(const QDate& date) {
	kdDebug() << "Gregorian format date..." << endl;
	QString q = KGlobal::locale()->formatDate(date,true) ;

	return q;
}

int KCalendarGregorian::getYear(const QDate& date) {
	kdDebug() << "Gregorian year..." <<  endl;
	return date.year();
}

void KCalendarGregorian::getNextMonthDate(QDate& temp) {
	kdDebug() << "Gregorian next month date..." << endl;
	int day = temp.day();
	if(temp.month()==12) {
		temp.setYMD(temp.year()+1, 1, 1);
	} else {
		temp.setYMD(temp.year(), temp.month()+1, 1);
	}
	if(temp.daysInMonth()<day) {
		temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
	} else {
		temp.setYMD(temp.year(), temp.month(), day);
	}
}

void KCalendarGregorian::getPreviousMonthDate(QDate& temp) {
	kdDebug() << "Gregorian previous month date..." << endl;
	int day = temp.day();
	if(temp.month()==1) {
		temp.setYMD(temp.year()-1, 1, 1);
	} else {
		temp.setYMD(temp.year(), temp.month()-1, 1);
	}
	if(temp.daysInMonth()<day) {
		temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
	} else {
		temp.setYMD(temp.year(), temp.month(), day);
	}
}


void KCalendarGregorian::getNextYearDate(QDate& temp) {
	kdDebug() << "Gregorian next year date..." << endl;
	int day = temp.day();
	temp.setYMD(temp.year()+1, temp.month(), 1);
	if(temp.daysInMonth()<day) {
		temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
	} else {
		temp.setYMD(temp.year(), temp.month(), day);
	}
}


void KCalendarGregorian::getPreviousYearDate(QDate& temp) {
	kdDebug() << "Gregorian previous year date..." << endl;
	int day = temp.day();
	temp.setYMD(temp.year()-1, temp.month(), 1);
	if(temp.daysInMonth()<day) {
		temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
	} else {
		temp.setYMD(temp.year(), temp.month(), day);
	}
}


int KCalendarGregorian::monthsInYear(int year) {

	kdDebug() << "Gregorian monthsInYear" << endl;

	return 12;
}

QString KCalendarGregorian::getMonthName(int month) {

	kdDebug() << "Gregorian getMonthName" << endl;

	return KGlobal::locale()->monthName(month, false);


}


void KCalendarGregorian::constructDateInMonth(QDate& date, int month) {

	int day;
	day = date.day();

	date.setYMD(date.year(), month, 1);
	date.setYMD(date.year(), month, QMIN(day, date.daysInMonth()));

	kdDebug() << "Gregorian constructDateInMonth" << endl;

}

void KCalendarGregorian::constructDateInYear(QDate& date, int year) {
	int day;
	day = date.day();

	date.setYMD(year, date.month(), 1);
	date.setYMD(year, date.month(), QMIN(day, date.daysInMonth()));

	kdDebug() << "Gregorian constructDateInYear" << endl;
}


QDate KCalendarGregorian::parseDate(QString text) {

	kdDebug() << "Gregorian parseDate" << endl;
	return KGlobal::locale()->readDate(text);

}


QString KCalendarGregorian::wDayName(int col) {
	//kdDebug() << "Gregorian wDayName" << endl;
	return KGlobal::locale()->weekDayName(col, true);
}


int KCalendarGregorian::dayOfTheWeek(const QDate& date) {
	kdDebug() << "Gregorian dayOfTheWeek" << endl;
	QDate temp;
	temp.setYMD(date.year(), date.month(), 1);
	return temp.dayOfWeek();
}


int KCalendarGregorian::numberOfDaysInMonth(const QDate& date) {
	kdDebug() << "Gregorian daysInMonth" << endl;
	return date.daysInMonth();
}


int KCalendarGregorian::numberOfDaysPrevMonth(const QDate& date) {
	kdDebug() << "Gregorian daysinprevmonth" << endl;
	QDate temp;
	if(date.month() == 1)
	{
		temp.setYMD(date.year()-1, 12, 1);
	} else {
		temp.setYMD(date.year(), date.month()-1, 1);
	}
	return temp.daysInMonth();
}



int KCalendarGregorian::getMaxValidYear() {
	return 8000; //QDate limit
}


int KCalendarGregorian::getDay(const QDate& date) {
	return date.day();
}



void KCalendarGregorian::printType() {
	kdDebug() << "It's Gregorian!" << endl;
}

KCalendarGregorian::~KCalendarGregorian() {};


KFactoryCalendar::KFactoryCalendar() {
	kdDebug() << "Created factory calendar" << endl;
};



QString KFactoryCalendar::calTy[] = { "gregorian", "hijri" };



KCalendarSystem* KFactoryCalendar::createCalendar(const QString& calType) {
	if(calType==calTy[1])
		return  new KCalendarHijri();
	else
		return  new KCalendarGregorian();
};



KFactoryCalendar::~KFactoryCalendar() {};

// Best regards ;)
