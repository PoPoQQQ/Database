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

-- SHOW TABLES;
-- DESC restaurant;
-- desc customer;
-- DESC food;
-- desc orders;

INSERT INTO customer VALUES (300001,'CHAD CABELLO','F'),(300002,'FAUSTO VANNORMAN','F');
INSERT INTO restaurant VALUES (1, 'main', 'dalian', NULL, NULL), (2, 'sub', 'beijing', '74513', NULL);
--INSERT INTO customer VALUES (NULL,'CHAD CABELLO','F');
select id, name from customer where id = 300001;
select name from customer;
update customer set id=300003, name='apsodiaopsdjopai', gender='M' where id = 300001;
select * from customer;
select * from customer, restaurant;
-- select id from customer where id IS NOT NULL and name > '1234';
