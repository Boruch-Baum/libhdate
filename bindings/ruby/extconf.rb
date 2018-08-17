require 'mkmf'
$LIBS+=" -lhdate -lstdc++"
$CFLAGS+=" -I../../src"
create_makefile("hdate")

