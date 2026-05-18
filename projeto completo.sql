create database sensor;
use sensor;

create table velocidade(
	id int(11) not null auto_increment primary key,
    velocidade float not null,
    data_hora timestamp default current_timestamp
);