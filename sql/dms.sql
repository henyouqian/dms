DROP DATABASE IF EXISTS dms_db;
CREATE DATABASE dms_db DEFAULT CHARACTER SET utf8;
USE dms_db;
						
CREATE TABLE Games (	game_id             INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,
						developer_id        INT UNSIGNED NOT NULL,
                        app_id              INT UNSIGNED,
						name                VARCHAR(20) NOT NULL,
						score_order         BOOLEAN 	NOT NULL,
						constraint uk_name_dev unique(name, developer_id),
                        INDEX idx_app_id (app_id)
						)ENGINE = InnoDB;
						
CREATE TABLE Scores (   user_id         INT UNSIGNED NOT NULL,
						game_id         INT UNSIGNED NOT NULL,
                        date            DATE NOT NULL,
                        time            TIME NOT NULL,
                        score           INT	UNSIGNED NOT NULL,
                        UNIQUE KEY uk_user_date_game (user_id, date, game_id),
                        INDEX idx_date_game (date, game_id)
                        )ENGINE = InnoDB;

						
CREATE TABLE Ranks (	user_id         INT UNSIGNED NOT NULL,
                        user_name       VARCHAR(20),
                        game_id         INT UNSIGNED NOT NULL,
                        date            DATE NOT NULL,
                        user_name       VARCHAR(20),
                        row             INT	UNSIGNED NOT NULL,
                        rank            INT	UNSIGNED NOT NULL,
                        score           INT	UNSIGNED NOT NULL,
                        nationality     SMALLINT,
                        time            TIME	NOT NULL,
						UNIQUE KEY uk_user_date_game (user_id, date, game_id)
						)ENGINE = InnoDB;

