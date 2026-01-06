.load ./libyarts
.mode box
create virtual table todos using fetch (
    url text default 'https://jsonplaceholder.typicode.com/todos',
    id int,
    "userId" int,
    title text,
    completed text
);
select * from todos limit 5;
