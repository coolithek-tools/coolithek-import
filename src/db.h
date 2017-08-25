
#ifndef __MV2MYSQL_DB_H__
#define __MV2MYSQL_DB_H__

/*
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
*/

//#define TEST_PREFIX
#define TEST_PREFIX "_TEST"

#define VIDEO_DB_X "mediathek_1"

#define VIDEO_DB VIDEO_DB_X TEST_PREFIX
#define VIDEO_DB_TMP_1 VIDEO_DB_X "_tmp_1" TEST_PREFIX
#define VIDEO_DB_TEMPLATE VIDEO_DB_X "_template"

#define VIDEO_TABLE "video"
#define INFO_TABLE "channelinfo"
#define VERSION_TABLE "version"


/*
CREATE DATABASE IF NOT EXISTS `mediathek_1` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `mediathek_1`;

DROP TABLE IF EXISTS `video`;
CREATE TABLE IF NOT EXISTS `video` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `channel` varchar(256) DEFAULT NULL,
  `theme` varchar(256) DEFAULT NULL,
  `title` varchar(1024) DEFAULT NULL,
  `date` varchar(64) DEFAULT NULL,
  `time` varchar(64) DEFAULT NULL,
  `duration` varchar(64) DEFAULT NULL,
  `size_mb` varchar(64) DEFAULT NULL,
  `description` text,
  `url` varchar(1024) DEFAULT NULL,
  `website` varchar(1024) DEFAULT NULL,
  `subtitle` varchar(1024) DEFAULT NULL,
  `url_rtmp` varchar(1024) DEFAULT NULL,
  `url_small` varchar(1024) DEFAULT NULL,
  `url_rtmp_small` varchar(1024) DEFAULT NULL,
  `url_hd` varchar(1024) DEFAULT NULL,
  `url_rtmp_hd` varchar(1024) DEFAULT NULL,
  `date_unix` int(11) DEFAULT NULL,
  `url_history` varchar(1024) DEFAULT NULL,
  `geo` varchar(1024) DEFAULT NULL,
  `new_entry` varchar(1024) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

DROP TABLE IF EXISTS `channelinfo`;
CREATE TABLE IF NOT EXISTS `channelinfo` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `channel` varchar(256) DEFAULT NULL,
  `count` int(11) DEFAULT NULL,
  `lastest` int(11) DEFAULT NULL,
  `oldest` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

DROP TABLE IF EXISTS `version`;
CREATE TABLE IF NOT EXISTS `version` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `version` varchar(256) DEFAULT NULL,
  `vdate` int(11) DEFAULT NULL,
  `mvversion` varchar(256) DEFAULT NULL,
  `mvdate` int(11) DEFAULT NULL,
  `mventrys` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;
*/

#endif // __MV2MYSQL_DB_H__

