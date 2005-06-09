#! /bin/sh
export PATH=/bin

InstallPath=$1
SourcePath=$2
FileName="$3"
InstallPath=`cd "$InstallPath"; pwd`
SourcePath=`cd "$SourcePath"; pwd`

if [ "$SourcePath" == "/" ]
then
  SourcePath=""
fi

case "$FileName" in
*-src*) 
  InstallPath="$InstallPath/src"; 
  mkdir -p "$InstallPath";
  ;;
esac

case $FileName in
*.tar.gz) TarFlags=-zxf; ;;
*.tar.bz2) TarFlags=-jxf; ;;
*.tgz) TarFlags=-zxf; ;;
*.tbz2) TarFlags=-jxf; ;;
*) TarFlags=-jxf; ;;
esac

cd "$InstallPath"
tar $TarFlags "$SourcePath/$FileName"
