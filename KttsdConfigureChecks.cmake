include(CheckIncludeFile)
include(CheckIncludeFiles)

check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files("sys/time.h;time.h" TIME_WITH_SYS_TIME)
