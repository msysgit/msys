#!/bin/sh

cd "$(dirname "$0")" &&

RSYNC_SOURCE=mingw.cvs.sourceforge.net::cvsroot/mingw/msys &&
rsync -av $RSYNC_SOURCE cvs-mirror &&
$HOME/cvs2svn/cvs2git --blobfile=msys.blobs --dumpfile=msys.dump \
	--username=dscho cvs-mirror/ > out.cvs2git 2>&1 &&
cat msys.blobs msys.dump | git fast-import > out.fast-import 2>&1 &&
git push --all origin &&
git push --tags origin ||
echo "Failure importing"
