create extension weighted_mean;

create temp table test as (
  select a::numeric, b::numeric 
  from 
    generate_series(1, 100) as a(a),
    generate_series(1, 100) as b(b));

select weighted_mean(a,b) from test;

update test set b = 0;

select weighted_mean(a,b) from test;

