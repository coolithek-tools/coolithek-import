#!/bin/bash

[ "$1" = "" -o "$2" = "" -o "$3" = "" -o "$4" = "" ] && { echo ""; echo "[$(basename $0)] No parameter, exit."; exit 1; }

SQL=$1
PROGNAME=$2
SQL_USER=$3
SQL_PW=$4
SQLNAME=$(basename $SQL)
SHNAME="/var/lib/mysql.docker/db.sh"
SH="/var/lib/mysql/db.sh"

echo ""
CMD="cp -f $SQL /var/lib/mysql.docker/$SQLNAME"
echo "[$PROGNAME] $CMD"
$CMD

echo "#!/bin/bash"							 > $SHNAME
echo "cd /var/lib/mysql"						>> $SHNAME
echo "echo \"[docker] mysql --user=XXX --password=YYY < $SQLNAME\""	>> $SHNAME
echo "mysql --user=$SQL_USER --password=$SQL_PW < $SQLNAME"		>> $SHNAME
chmod 0755 $SHNAME

CMD="docker exec -t slknet-mariadb bash -c $SH"
echo "[$PROGNAME] $CMD"
$CMD

rm -f $SHNAME
rm -f /var/lib/mysql.docker/$SQLNAME
