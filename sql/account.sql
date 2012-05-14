# DROP DATABASE IF EXISTS account_db;

CREATE DATABASE account_db DEFAULT CHARACTER SET utf8;
USE account_db;

CREATE TABLE Users (	user_id         INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,
						gamecenter_id   VARCHAR(40),
						email           VARCHAR(40),
						password        CHAR(64),
						name            VARCHAR(20),
						nationality     SMALLINT,
						facebook_id     INT UNSIGNED,
						UNIQUE KEY email_unique (email),
						UNIQUE KEY facebook_id_unique (facebook_id),
						UNIQUE KEY gamecenter_id_unique (gamecenter_id)
						)ENGINE = InnoDB;
						
CREATE TABLE Developers (	developer_id    INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,
							email           VARCHAR(40) NOT NULL,
							password        CHAR(64) 	NOT NULL,
							secret_key      CHAR(32) 	NOT NULL,
							UNIQUE KEY email_unique (email),
							UNIQUE KEY secret_unique (secret_key)
							)ENGINE = InnoDB;