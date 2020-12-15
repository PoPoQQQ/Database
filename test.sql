CREATE DATABASE orderDB;
SHOW DATABASES;

USE orderDB;
--DROP DATABASE orderDB;
CREATE TABLE restaurant (
  id INT(10) NOT NULL,
  name VARCHAR(25) NOT NULL,
  address VARCHAR(100),
  phone VARCHAR(20),
  rate FLOAT,
  PRIMARY KEY (id)
);

CREATE TABLE customer(
	id INT(10) NOT NULL,
	name VARCHAR(25) NOT NULL,
	gender VARCHAR(1) NOT NULL,
	PRIMARY KEY (id)
);

CREATE TABLE food(
	id INT(10) NOT NULL,
	restaurant_id INT(10),
	name VARCHAR(100) NOT NULL,
	price FLOAT NOT NULL,
	PRIMARY KEY (id),
	FOREIGN KEY (restaurant_id) REFERENCES restaurant(id)
);

CREATE TABLE orders(
	id INT(10) NOT NULL,
	customer_id INT(10) NOT NULL,
	food_id INT(10) NOT NULL,
	date DATE,
	quantity INT(10),
	PRIMARY KEY (id),
	FOREIGN KEY (customer_id) REFERENCES customer(id),
	FOREIGN KEY (food_id) REFERENCES food(id)
);

SHOW TABLES;

INSERT INTO customer VALUES (300001,'CHAD CABELLO','F'),(300002,'FAUSTO VANNORMAN','F');
--INSERT INTO customer VALUES (NULL,'CHAD CABELLO','F');
