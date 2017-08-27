#!/bin/bash

LIST_PATH=dl
LIST_NAME=Filmliste-akt.xz

URL=http://download10.onlinetvrecorder.com/mediathekview
#URL=http://mediathekview.jankal.me

#URL=http://verteiler4.mediathekview.de
#URL=http://verteiler5.mediathekview.de
#URL=http://verteiler6.mediathekview.de
#URL=http://verteiler1.mediathekview.de
#URL=http://verteiler2.mediathekview.de
#URL=http://verteiler3.mediathekview.de

P=$(realpath $(dirname $0))
S=$(basename $0)
echo "Execute $P/$S"
cd $P

mkdir -p $LIST_PATH
cd $LIST_PATH

MD_akt_old=$(md5sum $LIST_NAME 2> /dev/null)
rm -f $LIST_NAME
IPV=-4
echo "wget $IPV -c -N $URL/$LIST_NAME"
wget $IPV -c -N $URL/$LIST_NAME > /dev/null 2>&1

if [ ! -e $LIST_NAME ]; then
	echo "Download error, exit."
	exit 1
fi

MD_akt_new=$(md5sum $LIST_NAME)
if [ "$MD_akt_old" = "$MD_akt_new" ]; then
	echo "No changes, exit."
	exit
fi

cd $P

EPOCH=-e240
echo "./mv2mariadb -f $LIST_PATH/$LIST_NAME $EPOCH"
./mv2mariadb -f $LIST_PATH/$LIST_NAME $EPOCH

