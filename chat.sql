/*
Navicat MySQL Data Transfer

Source Server         : test
Source Server Version : 50719
Source Host           : localhost:3306
Source Database       : chat

Target Server Type    : MYSQL
Target Server Version : 50719
File Encoding         : 65001

Date: 2017-10-23 22:01:40
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for groups
-- ----------------------------
DROP TABLE IF EXISTS `groups`;
CREATE TABLE `groups` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT ' ',
  `owner_id` varchar(64) COLLATE utf8_bin NOT NULL COMMENT '群主',
  `create_time` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=10014 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;


-- ----------------------------
-- Table structure for private_chat
-- ----------------------------
DROP TABLE IF EXISTS `private_chat`;
CREATE TABLE `private_chat` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `sender_id` varchar(64) COLLATE utf8_bin NOT NULL COMMENT '发送人',
  `recver_id` varchar(64) COLLATE utf8_bin NOT NULL COMMENT '接收人',
  `chat_type` int(1) NOT NULL COMMENT '聊天类型',
  `content` varchar(1024) COLLATE utf8_bin NOT NULL COMMENT '聊天内容',
  `send_time` int(11) NOT NULL COMMENT '发送时间(时间戳)',
  `readed` int(1) NOT NULL COMMENT '是否已经阅读',
  PRIMARY KEY (`id`),
  KEY `sender_id` (`sender_id`),
  KEY `recver_id` (`recver_id`),
  KEY `readed` (`readed`,`send_time`) USING BTREE
) ENGINE=MyISAM AUTO_INCREMENT=35 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- ----------------------------
-- Procedure structure for createt_group_chat_if_not_exists
-- ----------------------------
DROP PROCEDURE IF EXISTS `createt_group_chat_if_not_exists`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `createt_group_chat_if_not_exists`(IN `group_id` varchar(64))
BEGIN
	#Routine body goes here...

	set @sqlStr = CONCAT('
	CREATE TABLE IF NOT EXISTS `group_chat_', `group_id`, '` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `sender_id` varchar(64) COLLATE utf8_bin NOT NULL COMMENT "发送人",
  `chat_type` int(1) NOT NULL COMMENT "聊天类型",
  `content` varchar(1024) COLLATE utf8_bin NOT NULL COMMENT "聊天内容",
  `send_time` int(11) NOT NULL COMMENT "发送时间(时间戳)",
  PRIMARY KEY (`id`)
	) ENGINE=MyISAM AUTO_INCREMENT=26 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;');


PREPARE stmt from @sqlStr;
EXECUTE stmt;


END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for create_group
-- ----------------------------
DROP PROCEDURE IF EXISTS `create_group`;
DELIMITER ;;
CREATE DEFINER=`test`@`%` PROCEDURE `create_group`(IN `__owner_id` varchar(64))
BEGIN
	#Routine body goes here...

	DECLARE `__now` INT;
	DECLARE `__id` INT;
	SELECT UNIX_TIMESTAMP(NOW()) INTO `__now`;
	INSERT INTO `groups` (`owner_id`, `create_time`) VALUES (`__owner_id`, `__now`);
	
	SELECT MAX(`id`) INTO `__id` FROM groups WHERE `owner_id` = `__owner_id`;
	CALL create_group_chat(CONCAT(`__id`));
	CALL create_group_member(CONCAT(`__id`), CONCAT(`__owner_id`));

	SELECT `__id`, `__now`;
	
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for create_group_chat
-- ----------------------------
DROP PROCEDURE IF EXISTS `create_group_chat`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `create_group_chat`(IN `group_id` varchar(64))
BEGIN
	#Routine body goes here...

	SET @_drop_grup = CONCAT('DROP TABLE IF EXISTS `group_chat_', `group_id`, '`;');
	PREPARE stmt from @_drop_grup;
	EXECUTE stmt;

	set @sqlStr = CONCAT('
	CREATE TABLE `group_chat_', `group_id`, '` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `sender_id` varchar(64) COLLATE utf8_bin NOT NULL COMMENT "发送人",
  `chat_type` int(1) NOT NULL COMMENT "聊天类型",
  `content` varchar(1024) COLLATE utf8_bin NOT NULL COMMENT "聊天内容",
  `send_time` int(11) NOT NULL COMMENT "发送时间(时间戳)",
  PRIMARY KEY (`id`)
	) ENGINE=MyISAM AUTO_INCREMENT=26 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;');


PREPARE stmt from @sqlStr;
EXECUTE stmt;

END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for create_group_member
-- ----------------------------
DROP PROCEDURE IF EXISTS `create_group_member`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `create_group_member`(IN `group_id` INT, IN `owner_id` varchar(64))
BEGIN
	#Routine body goes here...
	DECLARE `__now` INT;
	SELECT UNIX_TIMESTAMP(NOW()) INTO `__now`;

	SET @_drop_group = CONCAT('DROP TABLE IF EXISTS `group_member_', `group_id`, '`;');
	PREPARE stmt from @_drop_group;
	EXECUTE stmt;


	set @sqlStr = CONCAT('
	CREATE TABLE `group_member_', `group_id`, '` (
  `player_id` varchar(64) COLLATE utf8_bin NOT NULL,
  `power` int(10) unsigned NOT NULL COMMENT "职位(位运算 0:群员 29:管理 30:群主)",
  `join_time` int(11) NOT NULL COMMENT "加入时间",
	PRIMARY KEY (`player_id`)
	) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

	
');

#INSERT INTO `group_member_', `group_id`, '` VALUES ("', `owner_id`,'", 1, ', `__now`,');

	PREPARE stmt from @sqlStr;
	EXECUTE stmt;

	SET @joinchat = CONCAT('INSERT INTO `group_member_', `group_id`,'` VALUES ("', `owner_id`,'", 1610612737, ', `__now`,');');
	PREPARE stmt from @joinchat;
	EXECUTE stmt;

END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for min_id
-- ----------------------------
DROP PROCEDURE IF EXISTS `min_id`;
DELIMITER ;;
CREATE DEFINER=`test`@`%` PROCEDURE `min_id`(IN player_id VARCHAR(5))
BEGIN
	#Routine body goes here...
	DECLARE chat_id INT;
	DECLARE __id INT;
	SELECT MAX(`id`) FROM private_chat WHERE `readed` = 0 AND `recver_id` = `player_id` ORDER BY `id` ASC LIMIT 0, 1 ;


	SELECT MAX(`id`) INTO `chat_id` FROM private_chat WHERE `id` IN (SELECT `id` FROM private_chat WHERE `readed` = 0 AND `recver_id` = `player_id` ORDER BY `id` ASC);
	SELECT `chat_id`;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for test
-- ----------------------------
DROP PROCEDURE IF EXISTS `test`;
DELIMITER ;;
CREATE DEFINER=`test`@`%` PROCEDURE `test`()
BEGIN
	#Routine body goes here...

set @sqlStr=CONCAT('select * from ','private_chat');
PREPARE stmt from @sqlStr;
EXECUTE stmt;
END
;;
DELIMITER ;
