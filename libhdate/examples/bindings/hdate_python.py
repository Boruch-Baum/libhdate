#! /usr/bin/python

# import the hdate library
from hdate import *;

# create a new Hdate object
h = Hdate();

# print gregorian date
print h.get_gday (), h.get_gmonth(), h.get_gyear ()

# print hebrew date
print h.get_format_date (0, 0);
