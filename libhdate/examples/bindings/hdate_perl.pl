#! /usr/bin/perl

# hdate_perl.pl
# example file for libhdate.
#

#
# run: perl hdate_perl.pl
#

#
#  Copyright (C) 2008  Yaacov Zamir <kzamir@walla.co.il>
#  
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# Use the hdate perl module
use hdate;

# Create a new Hdate object
$h = hdatec::new_Hdate();

# Print gregorian date
print "Today is:\n";
print hdatec::Hdate_get_gday($h) . "," . hdatec::Hdate_get_gmonth($h) . "," . hdatec::Hdate_get_gyear($h) . "\n";

# Print hebrew date: 0 - long format
print hdatec::Hdate_get_format_date ($h, 0) . "\n";
