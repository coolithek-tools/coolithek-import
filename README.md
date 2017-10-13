# mv2mariadb
### Datenkonverter MediathekView Datenbank => Coolithek SQL
#### Aktuelle Version: 0.4.1

<br />
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

<style>
.table1 {
border: 0;
}
.td1a, .td1b, .td1c, .td1d {
border: 0;
padding: 4px;
vertical-align: top;
}
.td1a {
width: 2%;
}
.td1b {
width: 5%;
}
.td1c {
width: 20%;
}
</style>

<table class="table1">
<tr>
<td class="td1a">&nbsp;</td>
<td class="td1b">&nbsp;</td>
<td class="td1c"><strong>Usage:</strong></td>
<td class="td1d"><strong>mv2mariadb [OPTION]</strong></td>
</tr>

<tr>
<td class="td1d" style="padding:0px;" colspan="4">&nbsp;</td>
</tr>

<tr>
<td class="td1a">&nbsp;</td>
<td class="td1b">-e</td>
<td class="td1c">--epoch xxx</td>
<td class="td1d">Use not older entrys than 'xxx' days (default all data)</td>
</tr>

<tr>
<td class="td1a">&nbsp;</td>
<td class="td1b">-f</td>
<td class="td1c">--force-convert</td>
<td class="td1d">Data also convert, when movie list is up-to-date</td>
</tr>

<tr>
<td class="td1a">&nbsp;</td>
<td class="td1b">-c</td>
<td class="td1c">--cron-mode xxx</td>
<td class="td1d">'xxx' = time in minutes. Specifies the period during which no new version check is performed after the last download</td>
</tr>

<tr>
<td class="td1a">&nbsp;</td>
<td class="td1b">-C</td>
<td class="td1c">--cron-mode-echo</td>
<td class="td1d">Output message during --cron-mode to the log (Default: no output)</td>
</tr>

<tr>
<td class="td1a">&nbsp;</td>
<td class="td1b">-D</td>
<td class="td1c">--diff-mode</td>
<td class="td1d">Use difference list instead of the complete movie list</td>
</tr>

<tr>
<td class="td1a">&nbsp;</td>
<td class="td1b">-n</td>
<td class="td1c">--no-indexes</td>
<td class="td1d">Don't create indexes for database</td>
</tr>

<tr>
<td class="td1a">&nbsp;</td>
<td class="td1b">&nbsp;</td>
<td class="td1c">--update</td>
<td class="td1d">Create new config file and new template database, then exit</td>
</tr>

<tr>
<td class="td1a">&nbsp;</td>
<td class="td1b">&nbsp;</td>
<td class="td1c">--download-only</td>
<td class="td1d">Download only (Don't convert to sql database)</td>
</tr>

<tr>
<td class="td1a">&nbsp;</td>
<td class="td1b">&nbsp;</td>
<td class="td1c">--load-serverlist</td>
<td class="td1d">Load new serverlist and exit</td>
</tr>

<tr>
<td class="td1d" colspan="4">&nbsp;</td>
</tr>

<tr>
<td class="td1a">&nbsp;</td>
<td class="td1b">-d</td>
<td class="td1c">--debug-print</td>
<td class="td1d">Print debug info</td>
</tr>

<tr>
<td class="td1a">&nbsp;</td>
<td class="td1b">-v</td>
<td class="td1c">--version</td>
<td class="td1d">Display versions info and exit</td>
</tr>

<tr>
<td class="td1a">&nbsp;</td>
<td class="td1b">-h</td>
<td class="td1c">--help</td>
<td class="td1d">Display the help screen and exit</td>
</tr>
</table>
