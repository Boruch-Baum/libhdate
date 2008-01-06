#! /usr/bin/python

# Import the hdate library
from hdate import *;

# Create a new Hdate object
h = Hdate ();

# Print gregorian date
print "Today is:"
print str (h.get_gday ()) + "," + str (h.get_gmonth ()) + "," + str (h.get_gyear ())

# Print hebrew date: 0 - long format
print h.get_format_date (0);
