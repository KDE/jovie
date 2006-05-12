#! /usr/bin/env bash

$EXTRACTRC */*.rc */*/*.rc >> rc.cpp || exit 11
$EXTRACTRC */*.ui */*/*.ui >> rc.cpp || exit 12
$EXTRACTRC --tag=name --context=FestivalVoiceName plugins/festivalint/voices >> rc.cpp
$XGETTEXT rc.cpp */*.cpp */*.h */*/*.cpp */*/*.h -o $podir/kttsd.pot

