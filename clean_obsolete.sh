#! /bin/sh

# This script cleans obsolete KTTS files from your system.
# You should run this if you have been downloading KTTS and installing
# prior to the indicated dates.

# You would normally run this after running configure and before
# make install, i.e.,
#   cd kdenonbeta
#   echo kttsd>inst-apps
#   make -f Makefile.cvs
#   ./configure
#   cd kttsd
#   ./clean_obsolete.sh
#   make install

PREFIX=$(kde-config --prefix)
LIBTOOL="../libtool"

if [ -z "$PREFIX" ]; then
    echo "KDE prefix not found.  Do you have kde-config installed?"
    exit
fi

if [ ! -x $LIBTOOL ]; then
    echo "libtool was not found.  Did you run configure?"
    exit
fi

set -x

# libktts removed.  See kdeaccessibility/kttsd/kcmkttsmgr/Makefile.am
# for example how to build without it.
# on or about 20 Dec 2004.
$LIBTOOL --mode=uninstall $PREFIX/lib/kde3/libktts
$LIBTOOL --mode=uninstall $PREFIX/lib/libktts

# ServiceType kttsd.desktop renamed to kttsd_synthplugin.desktop,
# which distinquishes it from kttsd.desktop in the services dir
# and more accurately reflects its purpose
# on or about 8 Dec 2004.
rm -f $PREFIX/share/servicetypes/kttsd.desktop

# kcm_kttsmgr removed.  Use kcm_kttsd instead.
# Change made on or about 18 Dec 2004.
$LIBTOOL --mode=uninstall $PREFIX/lib/kde3/kcm_kttsmgr
rm -f $PREFIX/share/applnk/Settings/Accessibility/kcmkttsmgr.desktop
rm -f $PREFIX/share/applications/kde/kcmkttsmgr.desktop

# Renamed libkttsjobmgr to libkttsjobmgrpart per kdelibs/NAMING convention
# on or about 19 Oct 2004:

$LIBTOOL --mode=uninstall $PREFIX/lib/kde3/libkttsjobmgr

# The following installed files were renamed
# on or about 19 Oct 2004:
#  In $KDEDIR/share/services/:
#    festival.desktop         -> kttsd_festivalplugin.desktop
#    festivalint.desktop      -> kttsd_festivalintplugin.desktop
#    command.desktop          -> kttsd_commandplugin.desktop
#    hadifix.desktop          -> kttsd_hadifixplugin.desktop
#    flite.desktop            -> kttsd_fliteplugin.desktop
#    epos-kttsdplugin.desktop -> kttsd_eposplugin.desktop
#    freetts.desktop          -> kttsd_freettsplugin.desktop
#  In $KDEDIR/lib/kde3/:
#    libfestivalplugin        -> libkttsd_festivalplugin
#    libfestivalintplugin     -> libkttsd_festivalintplugin
#    libcommandplugin         -> libkttsd_commandplugin
#    libhadifixplugin         -> libkttsd_hadifixplugin
#    libfliteplugin           -> libkttsd_fliteplugin
#    libeposkttsdplugin       -> libkttsd_eposplugin
#    libfreettsplugin         -> libkttsd_freettsplugin

rm -f $PREFIX/share/services/festival.desktop
rm -f $PREFIX/share/services/festivalint.desktop
rm -f $PREFIX/share/services/command.desktop
rm -f $PREFIX/share/services/hadifix.desktop
rm -f $PREFIX/share/services/flite.desktop
rm -f $PREFIX/share/services/epos-kttsdplugin.desktop
rm -f $PREFIX/share/services/freetts.desktop

$LIBTOOL --mode=uninstall $PREFIX/lib/kde3/libfestivalplugin
$LIBTOOL --mode=uninstall $PREFIX/lib/kde3/libfestivalintplugin
$LIBTOOL --mode=uninstall $PREFIX/lib/kde3/libcommandplugin
$LIBTOOL --mode=uninstall $PREFIX/lib/kde3/libhadifixplugin
$LIBTOOL --mode=uninstall $PREFIX/lib/kde3/libfliteplugin
$LIBTOOL --mode=uninstall $PREFIX/lib/kde3/libeposkttsdplugin
$LIBTOOL --mode=uninstall $PREFIX/lib/kde3/libfreettsplugin

# The following library was changed from unversioned to versioned
# on or about 13 Oct 2004, 

$LIBTOOL --mode=uninstall $PREFIX/lib/libktts

# The hadifax plugin was renamed to hadifix
# on or about 4 Sep 2004.

rm -f $PREFIX/share/services/hadifax.desktop
$LIBTOOL --mode=uninstall /lib/kde3/libhadifaxplugin

# Clean up the library cache.

$LIBTOOL --mode=finish -n $PREFIX/lib
$LIBTOOL --mode=finish -n $PREFIX/lib/kde3/

