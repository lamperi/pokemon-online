#!/bin/bash
#
# A script to pack data files inside .app bundle in Mac
# Run this after compiling Teambuilder and plugins

function bundle_mac_app() {
   app=$1
   bundle_list=$2
   toDir=$app/Contents/Resources
   if [ "$1" == "--skip-resources" ] || [ "$1" == "-s" ]
   then
       SKIP=1
   fi
   if [ -z "$SKIP" ]
   then
       if [ -z "$toDir" ] || [ ! -d "$toDir" ]
       then
         mkdir -p $toDir
       fi

       [ -d $toDir/Music ] && echo rm -r $toDir/Music && rm -r $toDir/Music
       [ -d $toDir/db ] && echo rm -r $toDir/db && rm -r $toDir/db
       [ -d $toDir/Themes ] && echo rm -r $toDir/db && rm -r $toDir/Themes

       for file in $2
       do
           echo cp -r $file $toDir
           cp -r $file $toDir
       done
   fi # end of SKIP

   if [ $app = "Pokemon-Online.app" ]
   then
      # Find QMAKE with two good fallbacks
      QMAKE=$(grep "^QMAKE " ../src/Teambuilder/Makefile | cut -d= -f 2)
      : ${QMAKE:=$(grep "^QMAKE " ../src/Server/Makefile | cut -d= -f 2)}
      : ${QMAKE:=qmake}
      IMPORTS=$(qmake -query QT_INSTALL_IMPORTS)
      PLUGINS=$(qmake -query QT_INSTALL_PLUGINS)
      if [ -n "$IMPORTS" ]
      then
         mkdir -p $app/Contents/imports/Qt/labs
         cp -r $IMPORTS/Qt/labs/particles $app/Contents/imports/Qt/labs/
         cp -r $IMPORTS/Qt/labs/shaders $app/Contents/imports/Qt/labs/
      else
         echo 'Could not find QML Plugins, new battle window does not work'
      fi
      if [ -n "$PLUGINS" ]
      then
	 echo 'Removing extra plugins'
	 rm -rf $app/Contents/PlugIns/sqldrivers
	 rm -rf $app/Contents/PlugIns/printsupport
      else
         echo 'Could not find Qt Plugins, sounds do not work'
      fi
   fi
}

if [ -d Pokemon-Online.app ]
then
    bundle_mac_app Pokemon-Online.app "languages.txt trans Music db Themes qml myplugins"
fi
if [ -d Server.app ]
then
    bundle_mac_app Server.app "languages.txt serverplugins"
fi
