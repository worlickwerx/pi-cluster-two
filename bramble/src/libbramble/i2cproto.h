/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef _BRAMBLE_I2CPROTO_H
#define _BRAMBLE_I2CPROTO_H

#define I2C_ADDRESS                 0x30

#define I2C_REG_SLOT                10  // slot ID from ga0:4 (read)
#define I2C_REG_VERSION             11  // f/w version number (read)
#define I2C_REG_MATRIX_RAW          12  // 5 column bytes (write)
#define I2C_REG_MATRIX_CHAR         13  // 1 char (write)

#endif /* !_BRAMBLE_I2CPROTO_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
