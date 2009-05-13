#!/bin/sh

cd "$(dirname "$0")" &&

CVSROOT=:pserver:anonymous@mingw.cvs.sourceforge.net:/cvsroot/mingw &&
mkdir -p cvsclone &&
for i in $(sed -n 's/^\/cvsclone\///p' < .gitignore | grep -v '^out.\*$')
do
	(cd cvsclone/ &&
	 $HOME/cvsclone/cvsclone -d $CVSROOT $i > out.cvsclone.$i 2>&1) ||
	break
done &&
$HOME/cvs2svn/cvs2git --blobfile=msys.blobs --dumpfile=msys.dump \
	--username=dscho cvsclone/ > out.cvs2git 2>&1 &&
cat msys.blobs msys.dump | git fast-import > out.fast-import 2>&1 &&
git push --all origin &&
git push --tags origin ||
echo "Failure importing"
