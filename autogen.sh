#!/bin/sh
autoreconf -i -v && ./configure --enable-maintainer-mode "$@"
