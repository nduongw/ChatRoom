create table user
(
    id int
    auto_increment, 
    userName varchar
    (100) NOT NULL,
    password varchar
    (100) NOT NULL, 
    status int NOT NULL,
    lastLogin datetime,
    name VARCHAR
    (100) NOT NULL,
    PRIMARY key
    (id) 
);

    INSERT INTO user
        (userName, password, status, lastLogin, name)
    VALUES
        ('admin', 'admin', 1, '2018-01-01 00:00:00', 'Admin');