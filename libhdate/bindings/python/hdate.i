// file : hdate.i
%module hdate
%{
#include "../../src/hdate.h"
%}

%include ../../src/hdate.h

// on linux do:
// swig -python hdate.i
// gcc -fpic -c hdate_wrap.c -I/usr/include/python2.3
// gcc -shared ../../src/*.o hdate_wrap.o -o _hdate.so
