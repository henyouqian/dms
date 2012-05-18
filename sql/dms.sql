#DROP DATABASE IF EXISTS dms_db;

CREATE DATABASE dms_db DEFAULT CHARACTER SET utf8;
USE dms_db;

CREATE TABLE Apps  (     app_id             INT UNSIGNED PRIMARY KEY AUTO_INCREMENT
                        ,developer_id       INT UNSIGNED NOT NULL
                        ,name               VARCHAR(20) NOT NULL
                        ,secret_key         CHAR(32)    NOT NULL
                        ,constraint uk_dev_name unique(developer_id, name)
                        ,INDEX idx_dev (developer_id)
                        ,UNIQUE KEY uk_secret (secret_key)
                    )ENGINE = InnoDB;

CREATE TABLE Games  (    game_id             INT UNSIGNED PRIMARY KEY AUTO_INCREMENT
                        ,developer_id        INT UNSIGNED NOT NULL
                        ,app_id              INT UNSIGNED
                        ,name                VARCHAR(20) NOT NULL
                        ,score_order         BOOLEAN 	NOT NULL
                        ,constraint uk_name_dev_app unique(name, developer_id, app_id)
                        ,INDEX idx_app_id (app_id)
                    )ENGINE = InnoDB;

CREATE TABLE Scores (    user_id            INT UNSIGNED NOT NULL
                        ,game_id            INT UNSIGNED NOT NULL
                        ,date               DATE NOT NULL
                        ,time               TIME NOT NULL
                        ,score              INT UNSIGNED NOT NULL
                        ,idx_app_user       INT UNSIGNED NOT NULL
                        ,UNIQUE KEY uk_user_date_game (user_id, date, game_id)
                        ,INDEX idx_date_game (date, game_id)
                    )ENGINE = InnoDB;

CREATE TABLE Ranks  (    user_id            INT UNSIGNED NOT NULL
                        ,game_id            INT UNSIGNED NOT NULL
                        ,app_id             INT UNSIGNED NOT NULL
                        ,date               DATE NOT NULL
                        ,row                INT UNSIGNED NOT NULL
                        ,rank               INT UNSIGNED NOT NULL
                        ,score              INT UNSIGNED NOT NULL
                        ,time               TIME NOT NULL
                        ,user_name          VARCHAR(20)
                        ,nationality        SMALLINT
                        ,idx_app_user       INT UNSIGNED NOT NULL
                        ,UNIQUE KEY uk_user_date_game (user_id, date, game_id)
                        ,INDEX idx_app (app_id)
                        ,INDEX idx_row (row)
                        ,INDEX idx_idx_app_user (idx_app_user)
                    )ENGINE = InnoDB;

CREATE TABLE AppUserDatas  ( user_id        INT UNSIGNED NOT NULL
                            ,app_id         INT UNSIGNED NOT NULL
                            ,last_read      INT NOT NULL
                            ,last_write     INT NOT NULL
                            ,UNIQUE KEY uk_user_app (user_id, app_id)
                        )ENGINE = InnoDB;

CREATE TABLE UserDatas  (    user_id        INT UNSIGNED PRIMARY KEY
                            ,cash           INT UNSIGNED
                        )ENGINE = InnoDB;