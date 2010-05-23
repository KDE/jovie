#! /usr/bin/env bash

$EXTRACTRC */*.rc */*/*.rc >> rc.cpp || exit 11
$EXTRACTRC */*.ui */*/*.ui >> rc.cpp || exit 12
$XGETTEXT rc.cpp */*.cpp */*.h */*/*.cpp */*/*.h -o $podir/jovie.pot
rm -f rc.cpp
