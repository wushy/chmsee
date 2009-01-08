#! /bin/sh

set -x
autoreconf -vif
intltoolize --automake -c
