# Copyright (C) 2012 Raphael Kubo da Costa <rakuco@webkit.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND ITS CONTRIBUTORS ``AS
# IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ITS
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_PANGO QUIET pango)

FIND_LIBRARY(PANGO_LIBRARIES
    NAMES pango-1.0
    HINTS ${PC_PANGO_LIBDIR} ${PC_PANGO_LIBRARY_DIRS}
)

FIND_PATH(PANGO_INCLUDE_DIR
    NAMES pango/pango.h
    HINTS ${PC_PANGO_INCLUDEDIR} ${PC_PANGO_INCLUDE_DIRS}
    PATH_SUFFIXES pango-1.0
)

SET(PANGO_INCLUDE_DIRS ${PANGO_INCLUDE_DIR})

# Additional Pango components.  We only look for libraries, as not all of them
# have corresponding headers and all headers are installed alongside the main
# Pango ones.
FOREACH(_component ${PANGO_FIND_COMPONENTS})
    IF(${_component} STREQUAL "pangoft2")
        IF(NOT PANGOFT2_LIBRARIES)
            FIND_LIBRARY(PANGOFT2_LIBRARIES
                NAMES pangoft2-1.0
                HINTS ${_PANGO_LIBRARY_DIR}
            )
            SET(ADDITIONAL_REQUIRED_VARS ${ADDITIONAL_REQUIRED_VARS}
                PANGOFT2_LIBRARIES
            )
        ENDIF()
    ELSEIF(${_component} STREQUAL "pangocairo")
        IF(NOT PANGOCAIRO_LIBRARIES)
            FIND_LIBRARY(PANGOCAIRO_LIBRARIES
                NAMES pangocairo-1.0
                HINTS ${_PANGO_LIBRARY_DIR}
            )
            SET(ADDITIONAL_REQUIRED_VARS ${ADDITIONAL_REQUIRED_VARS}
                PANGOCAIRO_LIBRARIES
            )
        ENDIF()
    ELSEIF(${_component} STREQUAL "pangoxft")
        IF(NOT PANGOXFT_LIBRARIES)
            FIND_LIBRARY(PANGOXFT_LIBRARIES
                NAMES pangoxft-1.0
                HINTS ${_PANGO_LIBRARY_DIR}
            )
            SET(ADDITIONAL_REQUIRED_VARS ${ADDITIONAL_REQUIRED_VARS}
                PANGOXFT_LIBRARIES
            )
        ENDIF()
    ELSEIF(${_component} STREQUAL "pangowin32")
        IF(NOT PANGOWIN32_LIBRARIES)
            FIND_LIBRARY(PANGOWIN32_LIBRARIES
                NAMES pangowin32-1.0
                HINTS ${_PANGO_LIBRARY_DIR}
            )
            SET(ADDITIONAL_REQUIRED_VARS ${ADDITIONAL_REQUIRED_VARS}
                PANGOWIN32_LIBRARIES
            )
        ENDIF()
ENDFOREACH()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    Pango REQUIRED_VARS
    PANGO_INCLUDE_DIRS
    PANGO_LIBRARIES
    ${ADDITIONAL_REQUIRED_VARS}
    VERSION_VAR
    PANGO_VERSION
)

MARK_AS_ADVANCED(
    PANGOCONFIG_INCLUDE_DIR
    PANGOFT2_LIBRARIES
    PANGOCAIRO_LIBRARIES
    PANGOXFT_LIBRARIES
    PANGOWIN32_LIBRARIES
    PANGO_INCLUDE_DIR
    PANGO_INCLUDE_DIRS
    PANGO_LIBRARIES
)
