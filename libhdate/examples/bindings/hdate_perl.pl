#! /usr/bin/perl

# use the hdate perl module
use hdate;

#
# !! SOME SWIG VERSIONS WILL CREATE hdatec NAMESPACE OTHERS hdate !!
#
# use hdate; # just hdate
#
# $h = hdate::new_Hdate();
# print hdatec:Hdate_get_format_date ($h, 0, 0) . "\n"; # use hdate !
#

# create a new Hdate object
$h = hdatec::new_Hdate();

# print gregorian date
print "Today is:\n";
print hdatec::Hdate_get_gday($h) . "," . hdatec::Hdate_get_gmonth($h) . "," . hdatec::Hdate_get_gyear($h) . "\n";

# print hebrew date: 0 - israely holidays, 0 - long format
print hdatec::Hdate_get_format_date ($h, 0, 0) . "\n";

