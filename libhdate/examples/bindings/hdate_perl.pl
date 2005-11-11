#! /usr/bin/perl

# Use the hdate perl module
use hdate;

# Create a new Hdate object
$h = hdatec::new_Hdate();

# Print gregorian date
print "Today is:\n";
print hdatec::Hdate_get_gday($h) . "," . hdatec::Hdate_get_gmonth($h) . "," . hdatec::Hdate_get_gyear($h) . "\n";

# Print hebrew date: 0 - long format
print hdatec::Hdate_get_format_date ($h, 0) . "\n";
