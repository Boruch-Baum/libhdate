#! /usr/bin/perl

# use the hdate perl module
use hdate;

# create a new Hdate object
$h = hdate::new_Hdate();

# print gregorian date
print hdate::Hdate_get_gday($h) . " ";
print hdate::Hdate_get_gmonth($h) . " ";
print hdate::Hdate_get_gyear($h) . "\n";

# print hebrew date
print hdate::Hdate_get_format_date ($h, 0, 0) . "\n";

# delete the hdate object
hdate::delete_Hdate($h);
