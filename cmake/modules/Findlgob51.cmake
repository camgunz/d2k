INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_LUA51 QUIET lua5.1)

IF (NOT LGOB_LIBRARIES)
    FIND_PATH(LGOB_LIBRARIES
        NAMES pangocairo.so
        HINTS $ENV{LGOB_DIR} ${PC_LUA51_LIBDIR}
        PATH_SUFFIXES lua/5.1/lgob
    )
ENDIF()

MARK_AS_ADVANCED(LGOB_LIBRARIES)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    lgob
    DEFAULT_MSG
    LGOB_LIBRARIES
)

