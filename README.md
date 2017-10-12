# mv2mariadb
### Datenkonverter MediathekView Datenbank => Coolithek SQL
#### Aktuelle Version: 0.4.1

mv2mariadb ist ein unter Linux lauffähiges Tool zum konvertieren der [MediathekView](https://mediathekview.de) Datenbank (Json Format) nach SQL (MariaDB). Die Daten werden für das [Coolithek Plugin](https://wiki.slknet.de/w/Coolithek) benötigt.
Das Programm wird üblicherweise über einen Cron Job zu festgelegten Zeiten gestartet.

## Aufgaben

* Abruf der Liste der Downloadserver (Beim ersten Start und dann per default alle 7 Tage).
* Abrufen der Datenbank-Version auf dem Downloadserver.
* Vergleich der Datenbank-Versionen
* Wenn eine neuere Version vorliegt:
    * Download und entpacken der Daten
    * Einlesen der Json-Daten in eine Sql-Datenbank
    * Bereitstellen der Daten für das Plugin

## Benötigte Libs

* MariaDB Connector/C
    * [connector-c](https://downloads.mariadb.com/Connectors/c/connector-c-3.0.2)
    * oder für openSUSE 42.2 [libmariadb3](https://opensuse.tuxcode.de/repositories/tuxcode/privat/openSUSE_Leap_42.2/tools/x86_64/libmariadb3-3.0.2-1001.0.1.x86_64.rpm) und [libmariadb-devel](https://opensuse.tuxcode.de/repositories/tuxcode/privat/openSUSE_Leap_42.2/tools/x86_64/libmariadb-devel-3.0.2-1001.0.1.x86_64.rpm)
    * oder für openSUSE 42.3 [libmariadb3](https://opensuse.tuxcode.de/repositories/tuxcode/privat/openSUSE_Leap_42.3/tools/x86_64/libmariadb3-3.0.2-1001.0.1.x86_64.rpm) und [libmariadb-devel](https://opensuse.tuxcode.de/repositories/tuxcode/privat/openSUSE_Leap_42.3/tools/x86_64/libmariadb-devel-3.0.2-1001.0.1.x86_64.rpm)
* liblzma
* libcurl
* libpthread
* libexpat

Zum kompilieren des Programms werden auch die jeweiligen -devel Pakete sowie das rapidjson-devel Paket benötigt.
