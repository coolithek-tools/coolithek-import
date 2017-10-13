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

<table>
<tr>
<td>&nbsp;</td>
<td>&nbsp;</td>
<td><strong>Usage:</strong></td>
<td><strong>mv2mariadb [OPTION]</strong></td>
</tr>

<tr>
<td style="padding:0px;" colspan="4">&nbsp;</td>
</tr>

<tr>
<td>&nbsp;</td>
<td>-e</td>
<td>--epoch xxx</td>
<td>Use not older entrys than 'xxx' days (default all data)</td>
</tr>

<tr>
<td>&nbsp;</td>
<td>-f</td>
<td>--force-convert</td>
<td>Data also convert, when movie list is up-to-date</td>
</tr>

<tr>
<td>&nbsp;</td>
<td>-c</td>
<td>--cron-mode xxx</td>
<td>'xxx' = time in minutes. Specifies the period during which no new version check is performed after the last download</td>
</tr>

<tr>
<td>&nbsp;</td>
<td>-C</td>
<td>--cron-mode-echo</td>
<td>Output message during --cron-mode to the log (Default: no output)</td>
</tr>

<tr>
<td>&nbsp;</td>
<td>-D</td>
<td>--diff-mode</td>
<td>Use difference list instead of the complete movie list</td>
</tr>

<tr>
<td>&nbsp;</td>
<td>-n</td>
<td>--no-indexes</td>
<td>Don't create indexes for database</td>
</tr>

<tr>
<td>&nbsp;</td>
<td>&nbsp;</td>
<td>--update</td>
<td>Create new config file and new template database, then exit</td>
</tr>

<tr>
<td>&nbsp;</td>
<td>&nbsp;</td>
<td>--download-only</td>
<td>Download only (Don't convert to sql database)</td>
</tr>

<tr>
<td>&nbsp;</td>
<td>&nbsp;</td>
<td>--load-serverlist</td>
<td>Load new serverlist and exit</td>
</tr>

<tr>
<td colspan="4">&nbsp;</td>
</tr>

<tr>
<td>&nbsp;</td>
<td>-d</td>
<td>--debug-print</td>
<td>Print debug info</td>
</tr>

<tr>
<td>&nbsp;</td>
<td>-v</td>
<td>--version</td>
<td>Display versions info and exit</td>
</tr>

<tr>
<td>&nbsp;</td>
<td>-h</td>
<td>--help</td>
<td>Display the help screen and exit</td>
</tr>
</table>
