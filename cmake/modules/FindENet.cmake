INCLUDE(FindPackageHandleStandardArgs)

IF (NOT ENET_INCLUDE_DIR)
    FIND_PATH(ENET_INCLUDE_DIR enet/enet.h HINTS $ENV{ENET_DIR})
ENDIF()
IF (NOT ENET_LIBRARIES)
    FIND_LIBRARY(ENET_LIBRARIES enet HINTS $ENV{ENET_DIR})
ENDIF()

MARK_AS_ADVANCED(ENET_INCLUDE_DIR)
MARK_AS_ADVANCED(ENET_LIBRARIES)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    ENet
    DEFAULT_MSG
    ENET_LIBRARIES
    ENET_INCLUDE_DIR
)

