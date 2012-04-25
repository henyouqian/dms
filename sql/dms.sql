--DROP DATABASE IF EXISTS dms_db;
CREATE DATABASE dms_db DEFAULT CHARACTER SET utf8;
USE dms_db;
						
CREATE TABLE Games (	game_id 			INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,
						developer_id 		INT UNSIGNED NOT NULL,
						name 				VARCHAR(20) NOT NULL,
						score_order         BOOLEAN 	NOT NULL,
						constraint uk_name_dev unique(name, developer_id)
						)ENGINE = InnoDB;
						
CREATE TABLE Matches (	match_id 		INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,
						game_id 		INT UNSIGNED NOT NULL,
						developer_id 	INT UNSIGNED NOT NULL,
						date 			DATE 		NOT NULL,
						name			VARCHAR(100) NOT NULL,
						description		VARCHAR(200),
						score_sum		BIGINT UNSIGNED DEFAULT 0,
						score_num		INT UNSIGNED DEFAULT 0,
						score_best		INT UNSIGNED,
						INDEX idx_developer_id (developer_id),
						INDEX idx_date_id (date)
						)ENGINE = InnoDB;

CREATE TABLE Awards (	award_id 		INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,
						developer_id 	INT UNSIGNED NOT NULL,
						name			VARCHAR(40) NOT NULL,
						description		VARCHAR(200),
						image			VARCHAR(100)
						)ENGINE = InnoDB;

CREATE TABLE Scores (	user_id 		INT UNSIGNED NOT NULL,
						match_id 		INT UNSIGNED NOT NULL,
						score 			INT	UNSIGNED NOT NULL,
						failed			BOOLEAN	NOT NULL,
						time 			TIME 	NOT NULL,
						UNIQUE KEY user_match_uk (user_id, match_id),
						INDEX idx_matchid_score (match_id, score)
						)ENGINE = InnoDB;
						
CREATE TABLE PendingScores (	user_id 		INT UNSIGNED NOT NULL,
								match_id 		INT UNSIGNED NOT NULL,
								secret_key		CHAR(32) 	NOT NULL,
								expire_time		TIME		NOT NULL,
								UNIQUE KEY user_uk (user_id)
								)ENGINE = InnoDB;

						
CREATE TABLE Ranks (	user_id 		INT UNSIGNED NOT NULL,
						match_id 		INT UNSIGNED NOT NULL,
						row				INT	UNSIGNED NOT NULL,
						rank 			INT	UNSIGNED NOT NULL,
						score 			INT	UNSIGNED NOT NULL,
						time			TIME	NOT NULL,
						UNIQUE KEY uk_user_match (user_id, match_id),
						INDEX idx_match_row (match_id, row)
						)ENGINE = InnoDB;

