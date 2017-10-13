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

Zum kompilieren des Programms werden auch die jeweiligen -devel Pakete sowie das [rapidjson](https://github.com/Tencent/rapidjson)-devel Paket benötigt.

## Kommandozeilen-Optionen

<table style="border: 0;">
<tr>
<td style="border: 0; padding: 4px; vertical-align: top; width: 2%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 5%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 20%;"><strong>Usage:</strong></td>
<td style="border: 0; padding: 4px; vertical-align: top;"><strong>mv2mariadb [OPTION]</strong></td>
</tr>

<tr>
<td style="border: 0; padding: 4px; vertical-align: top;" style="padding:0px;" colspan="4">&nbsp;</td>
</tr>

<tr>
<td style="border: 0; padding: 4px; vertical-align: top; width: 2%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 5%;">-e</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 20%;">--epoch xxx</td>
<td style="border: 0; padding: 4px; vertical-align: top;">Use not older entrys than 'xxx' days (default all data)</td>
</tr>

<tr>
<td style="border: 0; padding: 4px; vertical-align: top; width: 2%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 5%;">-f</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 20%;">--force-convert</td>
<td style="border: 0; padding: 4px; vertical-align: top;">Data also convert, when movie list is up-to-date</td>
</tr>

<tr>
<td style="border: 0; padding: 4px; vertical-align: top; width: 2%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 5%;">-c</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 20%;">--cron-mode xxx</td>
<td style="border: 0; padding: 4px; vertical-align: top;">'xxx' = time in minutes. Specifies the period during which no new version check is performed after the last download</td>
</tr>

<tr>
<td style="border: 0; padding: 4px; vertical-align: top; width: 2%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 5%;">-C</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 20%;">--cron-mode-echo</td>
<td style="border: 0; padding: 4px; vertical-align: top;">Output message during --cron-mode to the log (Default: no output)</td>
</tr>

<tr>
<td style="border: 0; padding: 4px; vertical-align: top; width: 2%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 5%;">-D</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 20%;">--diff-mode</td>
<td style="border: 0; padding: 4px; vertical-align: top;">Use difference list instead of the complete movie list</td>
</tr>

<tr>
<td style="border: 0; padding: 4px; vertical-align: top; width: 2%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 5%;">-n</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 20%;">--no-indexes</td>
<td style="border: 0; padding: 4px; vertical-align: top;">Don't create indexes for database</td>
</tr>

<tr>
<td style="border: 0; padding: 4px; vertical-align: top; width: 2%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 5%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 20%;">--update</td>
<td style="border: 0; padding: 4px; vertical-align: top;">Create new config file and new template database, then exit</td>
</tr>

<tr>
<td style="border: 0; padding: 4px; vertical-align: top; width: 2%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 5%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 20%;">--download-only</td>
<td style="border: 0; padding: 4px; vertical-align: top;">Download only (Don't convert to sql database)</td>
</tr>

<tr>
<td style="border: 0; padding: 4px; vertical-align: top; width: 2%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 5%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 20%;">--load-serverlist</td>
<td style="border: 0; padding: 4px; vertical-align: top;">Load new serverlist and exit</td>
</tr>

<tr>
<td style="border: 0; padding: 4px; vertical-align: top;" colspan="4">&nbsp;</td>
</tr>

<tr>
<td style="border: 0; padding: 4px; vertical-align: top; width: 2%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 5%;">-d</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 20%;">--debug-print</td>
<td style="border: 0; padding: 4px; vertical-align: top;">Print debug info</td>
</tr>

<tr>
<td style="border: 0; padding: 4px; vertical-align: top; width: 2%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 5%;">-v</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 20%;">--version</td>
<td style="border: 0; padding: 4px; vertical-align: top;">Display versions info and exit</td>
</tr>

<tr>
<td style="border: 0; padding: 4px; vertical-align: top; width: 2%;">&nbsp;</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 5%;">-h</td>
<td style="border: 0; padding: 4px; vertical-align: top; width: 20%;">--help</td>
<td style="border: 0; padding: 4px; vertical-align: top;">Display the help screen and exit</td>
</tr>
</table>
