#! /usr/bin/ruby

# Import the hdate library
require 'hdate'

# Create a new Hdate object
h = Hdate::Hdate.new

# Print hebrew date: 0 - long format
puts h.get_format_date(0);
