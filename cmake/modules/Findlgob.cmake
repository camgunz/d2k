INCLUDE(FindPackageHandleStandardArgs)

IF(NOT LGOB_LIBRARIES)
    IF(CMAKE_SIZEOF_VOID_P EQUAL 8) # CG: What the hack
        FIND_PATH(LGOB_LIBRARIES
            NAMES pangocairo.so
            HINTS $ENV{LGOB_DIR}
            PATH_SUFFIXES lib64/lua/5.2/lgob
        )
    ELSE()
        FIND_PATH(LGOB_LIBRARIES
            NAMES pangocairo.so
            HINTS $ENV{LGOB_DIR}
            PATH_SUFFIXES lib/lua/5.2/lgob
        )
    ENDIF()
ENDIF()

MARK_AS_ADVANCED(LGOB_LIBRARIES)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    lgob
    DEFAULT_MSG
    LGOB_LIBRARIES
)

