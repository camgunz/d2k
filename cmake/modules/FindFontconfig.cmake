INCLUDE(FindPackageHandleStandardArgs)

IF (NOT FONTCONFIG_INCLUDE_DIR)
    FIND_PATH(FONTCONFIG_INCLUDE_DIR fontconfig/fontconfig.h
              HINTS $ENV{FONTCONFIG_DIR})
ENDIF()
IF (NOT FONTCONFIG_LIBRARIES)
    FIND_LIBRARY(FONTCONFIG_LIBRARIES fontconfig HINTS $ENV{FONTCONFIG_DIR})
ENDIF()

MARK_AS_ADVANCED(FONTCONFIG_INCLUDE_DIR)
MARK_AS_ADVANCED(FONTCONFIG_LIBRARIES)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    Fontconfig
    DEFAULT_MSG
    FONTCONFIG_LIBRARIES
    FONTCONFIG_INCLUDE_DIR
)

