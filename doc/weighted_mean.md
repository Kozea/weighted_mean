Weighted Mean extension for PostgreSQL
======================================

This extension provides a "weighted mean" aggregate function for postgreql.

The weighted mean is defined as a two columns aggregation function, one
containing the measure to be averaged, the second containing the weight of each
measure.

This extensions is implemented as a C library.


Installation
============

You can either install it with the pgxn client:

  pgxn install weighted_mean

Or from source:

  git clone git://github.com/Kozea/weighted_mean.git
  make && sudo make install

Then, create the extension in the specific database:

  CREATE extension weighted_mean;


Usage
=====

  select weighted_mean(unitprice, quantity) from sales;

This is equivalent to:

  select 
  case 
    when sum(quantity) = 0 then 0
    else sum(unitprice * quantity) / sum(quantity) 
  end
  from sales;
