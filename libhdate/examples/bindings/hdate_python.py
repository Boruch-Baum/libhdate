#! /usr/bin/python

# import the hdate library
from hdate import *;

# create a new Hdate object
h = Hdate();

# print gregorian date
print "Today is:"
print str (h.get_gday ()) + "," + str (h.get_gmonth()) + "," + str (h.get_gyear ())

# print hebrew date: 0 - israely holidays, 0 - long format
print h.get_format_date (0, 0);
