#ifndef BETTER_FAVORITES_SDL_CONFIG_H
#define BETTER_FAVORITES_SDL_CONFIG_H

/*
 * SDL2 compatibility configuration for Better Favorites.
 *
 * The Miyoo SDL2 port ships a prebuilt SDL2 runtime library but does not
 * include the generated SDL_config.h normally produced by Autotools.
 *
 * This header supplies the platform definitions required by SDL2's public
 * headers when compiling Better Favorites for Miyoo Mini / Mini Plus.
 *
 * It does NOT configure or rebuild SDL2 itself.
 */

#include "SDL_platform.h"

/* Miyoo Mini / Mini Plus userspace is 32-bit ARM. */
#define SIZEOF_VOIDP 4

/* Standard C headers available in the Onion/Miyoo toolchain. */
#define HAVE_STDARG_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STDINT_H 1

/* GCC atomic support. */
#define HAVE_GCC_SYNC_LOCK_TEST_AND_SET 1

/* Miyoo-specific SDL2 drivers used by the prebuilt runtime. */
#define SDL_VIDEO_DRIVER_MINI 1
#define SDL_AUDIO_DRIVER_MINI 1

#endif
