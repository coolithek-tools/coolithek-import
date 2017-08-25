#!/bin/bash

LIST=Filmliste-akt

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
mkdir -p ../dl/work
cd ../dl

MD_akt_old=$(md5sum ${LIST}.xz 2> /dev/null)
rm -f ${LIST}.xz
IPV=-4
echo "wget $IPV -c -N $URL/${LIST}.xz"
wget $IPV -c -N $URL/${LIST}.xz > /dev/null 2>&1

if [ ! -e ${LIST}.xz ]; then
	echo "Download error, exit."
	exit 1
fi

MD_akt_new=$(md5sum ${LIST}.xz)
if [ "$MD_akt_old" = "$MD_akt_new" ]; then
	echo "No changes, exit."
	exit
fi

## ###########################################################################

cp ${LIST}.xz work
cd work

rm -f ${LIST}
xz -d ${LIST}.xz
if [ ! "$?" = "0" ]; then
	echo "${LIST}.xz xz error, exit."
	exit 1
fi
touch -r ../${LIST}.xz ${LIST}

## ###########################################################################

cd $P

LOG=convert.log
echo -n "Start : "  > $P/$LOG
date               >> $P/$LOG

EPOCH=-e240
echo "./mv2mariadb -f ../dl/work/${LIST} $EPOCH"
./mv2mariadb -f ../dl/work/${LIST} $EPOCH

echo -n "End   : " >> $P/$LOG
date               >> $P/$LOG
