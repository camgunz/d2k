INCLUDE(FindPackageHandleStandardArgs)

FIND_PATH(ICONV_INCLUDE_DIR iconv.h HINTS $ENV{ICONV_DIR})
FIND_LIBRARY(ICONV_LIBRARIES iconv HINTS $ENV{ICONV_DIR})
MARK_AS_ADVANCED(ICONV_LIBRARY)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  Iconv
  DEFAULT_MSG
  ICONV_INCLUDE_DIR
)

IF(ICONV_FOUND)
  IF(ICONV_LIBRARIES)
    SET(ICONV_STANDALONE TRUE CACHE BOOL
      "Whether or not LibIconv is standalone"
    )
  ELSE()
    CHECK_FUNCTION_EXISTS("iconv" ICONV_IN_LIBC)
    IF(ICONV_IN_LIBC)
      SET(ICONV_STANDALONE FALSE CACHE BOOL
        "Whether or not LibIconv is standalone"
      )
    ELSE()
      MESSAGE(WARNING
        "LibIconv header found, but accompanying library not found"
      )
      SET(ICONV_FOUND FALSE CACHE BOOL "Whether or not LibIconv was found")
    ENDIF()
  ENDIF()
ENDIF()

