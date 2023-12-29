/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

#ifndef _BRAMBLE_VERSION_H
#define _BRAMBLE_VERSION_H

/* The VERSION_STRING may include a "-N-hash" suffix from git describe
 * if this snapshot is not tagged.  This is not reflected in VERSION_PATCH.
 */
#define BRAMBLE_VERSION_STRING    "0.1.0-350-g6fff23f"
#define BRAMBLE_VERSION_MAJOR     0
#define BRAMBLE_VERSION_MINOR     1
#define BRAMBLE_VERSION_PATCH     0

/* The version in 3 bytes, for numeric comparison.
 */
#define BRAMBLE_VERSION_HEX       ((BRAMBLE_VERSION_MAJOR << 16) | \
                                   (BRAMBLE_VERSION_MINOR << 8) | \
                                   (BRAMBLE_VERSION_PATCH << 0))


#endif /* !_BRAMBLE_VERSION_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
