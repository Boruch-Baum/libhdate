#! /usr/bin/perl

# use the hdate perl module
use hdate;

#
# !! SOME SWIG VERSIONS WILL CREATE hdatec NAMESPACE !!
#
# use hdate; # just hdate
#
# $h = hdatec::new_Hdate();
# print hdatec::Hdate_get_format_date ($h, 0, 0) . "\n"; # use hdatec !
#

# create a new Hdate object
$h = hdate::new_Hdate();

# print gregorian date
print "Today is:\n";
print hdate::Hdate_get_gday($h) . "," . hdate::Hdate_get_gmonth($h) . "," . hdate::Hdate_get_gyear($h) . "\n";

# print hebrew date: 0 - israely holidays, 0 - long format
print hdate::Hdate_get_format_date ($h, 0, 0) . "\n";

# delete the hdate object
hdate::delete_Hdate($h);
