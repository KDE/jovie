// The purpose of this file is to fix a conflict between KDE and Festival.
// Both use the define HAVE_CONFIG.  The files EST_String.h and EST_Chunk.h
// both use define HAVE_CONFIG, and if this symbol is defined, want to include
// a non-existent file called est_string_config.h.  This file satisfies that
// requirement.
// 
#    define HAVE_WALLOC_H (1)

