CREATE DATABASE orderDB;
SHOW DATABASES;

USE orderDB;
/*
CREATE TABLE restaurant (
  id INT(10) NOT NULL,
  name VARCHAR(25) NOT NULL,
  address VARCHAR(100),
  phone VARCHAR(20),
  rate FLOAT,
  PRIMARY KEY (id)
);*/
CREATE TABLE customer(
	id INT(10) NOT NULL,
	name VARCHAR(25) NOT NULL,
	gender VARCHAR(1) NOT NULL--,
	--PRIMARY KEY (id)
);
/*
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
	--customer_id INT(10) NOT NULL,
	--food_id INT(10) NOT NULL,
	date DATE,
	quantity INT(10),
	PRIMARY KEY (id)--,
	--FOREIGN KEY (customer_id) REFERENCES customer(id),
	--FOREIGN KEY (food_id) REFERENCES food(id)
);*/

-- SHOW TABLES;
-- DESC restaurant;
-- desc customer;
-- DESC food;

INSERT INTO customer VALUES (300001,'CHAD CABELLO','F'),(300001,'FAUSTO VANNORMAN','F');
ALTER TABLE customer ADD CONSTRAINT pk PRIMARY KEY (id);
--UPDATE customer SET id = 300002 where id = 300001;
--SELECT * FROM customer;
--SELECT * FROM customer where id = 300001;
--INSERT INTO orders VALUES(1, '2018-2-29', 100);