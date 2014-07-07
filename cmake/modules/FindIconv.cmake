INCLUDE(FindPackageHandleStandardArgs)

IF (NOT ICONV_INCLUDE_DIR)
    FIND_PATH(ICONV_INCLUDE_DIR iconv.h HINTS $ENV{ICONV_DIR})
ENDIF()

IF (NOT ICONV_LIBRARIES)
    FIND_LIBRARY(ICONV_LIBRARIES iconv HINTS $ENV{ICONV_DIR})
ENDIF()

MARK_AS_ADVANCED(ICONV_INCLUDE_DIR)
MARK_AS_ADVANCED(ICONV_LIBRARIES)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    Iconv
    DEFAULT_MSG
    ICONV_INCLUDE_DIR
)

IF(ICONV_FOUND)
    IF(ICONV_LIBRARIES)
        SET(ICONV_STANDALONE TRUE CACHE BOOL
            "Whether or not LibIconv is separate from the C library"
        )
    ELSE()
        CHECK_FUNCTION_EXISTS("iconv" ICONV_IN_LIBC)
        IF(ICONV_IN_LIBC)
            SET(ICONV_STANDALONE FALSE CACHE BOOL
                "Whether or not LibIconv is separate from the C library"
            )
        ELSE()
            MESSAGE(WARNING
                "LibIconv header found, but accompanying library not found"
            )
          SET(ICONV_FOUND FALSE CACHE BOOL "Whether or not LibIconv was found")
        ENDIF()
    ENDIF()
ENDIF()

