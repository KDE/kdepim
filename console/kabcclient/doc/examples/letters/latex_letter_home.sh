#!/bin/bash
#
# Script for creating an empty LaTeX letter filling in the receipient's
# address from a KDE addressbook query
# Uses the kabcclient application to access the KDE addressbook.
# This script is usually part of the kabcclient distribution but you can
# also download it here: http://www.kde-apps.org/content/show.php?content=25480
#

# check if the script got the address book lookup parameter
if [ $# -lt 1 ]; then
echo Not enough parameters!
echo
echo Usage: $0 addressbook lookup string
echo
echo Examples:
echo $0 \"Doe, John\"
echo $0 Doe
exit 1;
fi;

# perform the lookup using kabcclient
FIELDS=$(kabcclient -S "$1" -of csv -of-opts home_address)
RET=$?
if [ $RET -ge 1 ]; then
echo Addressbook lookup failed with return code $RET
exit 1;
fi;


NAME=$(echo $FIELDS | cut -d '|' -f 1)
STREET=$(echo $FIELDS | cut -d '|' -f 2)
POSTAL=$(echo $FIELDS | cut -d '|' -f 3)
CITY=$(echo $FIELDS | cut -d '|' -f 4)
COUNTRY=$(echo $FIELDS | cut -d '|' -f 5)

echo "\documentclass[a4paper,11pt]{letter}"
echo "\begin{document}"
echo "    \begin{letter}{$NAME\\\\$STREET\\\\$POSTAL $CITY\\\\$COUNTRY}"
echo "        \address{\$\$ADDRESS\$\$}"
echo "         \opening{\$\$SALUTATION\$\$}"
echo "         \signature{SenderName\\\\\$\$TITLE\$\$}"
echo "         \closing{\$\$CLOSING\$\$}"
echo "     \end{letter}"
echo "\end{document}"

                                         
                                         