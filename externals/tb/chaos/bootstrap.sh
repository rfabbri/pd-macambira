#! /bin/sh

aclocal \
&& automake-1.8 --gnu --add-missing \
&& autoconf