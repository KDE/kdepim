
KDE_LANG = en
KDE_DOCS = AUTO

translation.po: german_original.docbook
	xml2pot german_original.docbook > german.pot
	msgmerge -o translation.po translation.po german.pot
	rm german.pot
