lib_LTLIBRARIES = libkdgantt.la

libkdgantt_la_SOURCES = \
			KDGanttView.cpp			\
			KDGanttViewEventItem.cpp	\
			KDGanttViewItem.cpp		\
			KDGanttViewSubwidgets.cpp	\
			KDGanttViewSummaryItem.cpp	\
			KDGanttViewTaskItem.cpp		\
			KDGanttViewTaskLink.cpp		\
			KDGanttViewTaskLinkGroup.cpp	\
			KDMinimizeSplitter.cpp		\
			KDSemiSizingControl.cpp		\
			KDSizingControl.cpp		\
			KDXMLTools.cpp			\
			itemAttributeDialog.ui

noinst_HEADERS =	\
			KDGanttViewSubwidgets.h		\
			KDMinimizeSplitter.h		\
			KDSemiSizingControl.h		\
			KDSizingControl.h		\
			KDGanttViewEventItem.h		\
			KDGanttView.h			\
			KDGanttViewItem.h		\
			KDGanttViewSummaryItem.h	\
			KDGanttViewTaskItem.h		\
			KDGanttViewTaskLinkGroup.h	\
			KDGanttViewTaskLink.h		\
			KDXMLTools.h			\
			itemAttributeDialog.ui.h	\
			qlayoutengine_p.h

METASOURCES = AUTO

INCLUDES  = $(all_includes)

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kdgantt.pot
