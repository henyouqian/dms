 DROP DATABASE IF EXISTS account_db;

CREATE DATABASE account_db DEFAULT CHARACTER SET utf8;
USE account_db;

CREATE TABLE Users (	user_id         INT UNSIGNED PRIMARY KEY AUTO_INCREMENT
						,gamecenter_id   VARCHAR(40) NOT NULL
						,email           VARCHAR(40)
						,password        CHAR(64)
						,name            VARCHAR(20) NOT NULL
						,nationality     SMALLINT NOT NULL
						,UNIQUE KEY uk_email (email)
						,UNIQUE KEY uk_gamecenter_id (gamecenter_id)
					)ENGINE = InnoDB;
						
CREATE TABLE Developers (	 developer_id    INT UNSIGNED PRIMARY KEY AUTO_INCREMENT
							,email           VARCHAR(40) NOT NULL
							,password        CHAR(64) 	NOT NULL
							,UNIQUE KEY uk_email (email)
						)ENGINE = InnoDB;