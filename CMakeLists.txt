CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(rgs)

SET(RGS_RELEASE_DIR ${rgs_SOURCE_DIR}/release/${CMAKE_SYSTEM_NAME})
SET(CMAKE_INSTALL_PREFIX ${RGS_RELEASE_DIR})

FIND_PACKAGE(SDL REQUIRED)

IF( NOT SDL_FOUND )
   MESSAGE( FATAL_ERROR "SDL not found!" )
ENDIF(NOT SDL_FOUND )

INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} "${rgs_SOURCE_DIR}/include" "${rgs_SOURCE_DIR}/festalon" "${rgs_SOURCE_DIR}/festalon/drivers" "${rgs_SOURCE_DIR}/festalon/ext" "${rgs_SOURCE_DIR}/festalon/fir")

SET(rgs_SRCS
	src/BFont.c
	src/rgs_screen.c
	src/rgs_shape.c
	src/rgs_sound.c
	src/rgs_ui.c
	src/snes_ntsc.c
	src/isutil.c
	src/rgs_core.c
	src/rgs_label.c
	src/rgs_sprite.c
	src/rgs_surface.c
	src/rgs_tilemap.c
	festalon/cart.c
	festalon/filter.c
	festalon/nsf.c
	festalon/sound.c
	festalon/x6502.c
	festalon/ext/ay.c
	festalon/ext/emu2413.c
	festalon/ext/fds.c
	festalon/ext/mmc5.c
	festalon/ext/n106.c
	festalon/ext/vrc6.c
	festalon/ext/vrc7.c
	festalon/fir/toh.c
)

SET(rgs_HDRS
	include/bigfont.h
	include/rgs_screen.h
	include/rgs_shape.h
	include/rgs_sound.h
	include/rgs_surface.h
	include/rgs_ui.h
	include/snes_ntsc.h
	include/snes_ntsc_config.h
	include/snes_ntsc_impl.h
	include/BFont.h
	include/isutil.h
	include/rgs.h
	include/rgs_core.h
	include/rgs_data.h
	include/rgs_label.h
	include/rgs_sprite.h
	include/rgs_tilemap.h
	include/smallfont.h
)

SET(CMAKE_DEBUG_POSTFIX "_d")
ADD_LIBRARY(rgs ${rgs_SRCS} ${rgs_HDRS})


INSTALL(FILES ${rgs_HDRS} DESTINATION include)
INSTALL(TARGETS rgs EXPORT rgs-targets DESTINATION lib)

INSTALL(FILES LICENSE.txt DESTINATION ./)

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "9")
SET(CPACK_PACKAGE_VERSION_PATCH "0")

IF(WIN32)
	SET(CPACK_GENERATOR "ZIP")
ELSE(WIN32)
	SET(CPACK_GENERATOR "TGZ")
ENDIF(WIN32)

INCLUDE(CPack)
