#!/bin/sh -e

cd src
mkdir -p lib

for LIBDIR in Libraries/* ; do
	make -C $LIBDIR
done

cp -u lib/* ~/config/non-packaged/lib/

for APPDIR in EventEditor EventServer PreferencesPreflet ; do
	make -C $APPDIR
done
