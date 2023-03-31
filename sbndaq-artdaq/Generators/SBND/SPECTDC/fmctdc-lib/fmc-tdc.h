/*
 * fmc-tdc (a.k.a) FmcTdc1ns5cha main header.
 *
 * Copyright (C) 2012-2013 CERN (www.cern.ch)
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2 as published by the Free Software Foundation or, at your
 * option, any later version.
 */

#ifndef __FMC_TDC_H__
#define __FMC_TDC_H__

#define FT_VERSION_MAJ 2 /* version of the driver */
#define FT_VERSION_MIN 1

/* default gatewares */
#define FT_GATEWARE_SVEC "fmc/svec-fmc-tdc.bin"
#define FT_GATEWARE_SPEC "fmc/spec-fmc-tdc.bin"

#define FT_BUFFER_EVENTS 256

#define FT_CH_1 1
#define FT_NUM_CHANNELS 5

enum ft_zattr_dev_idx {
  FT_ATTR_DEV_VERSION = 0,
  FT_ATTR_DEV_SECONDS,
  FT_ATTR_DEV_COARSE,
  FT_ATTR_DEV_SEQUENCE,
  FT_ATTR_DEV_COMMAND, /* see below for commands */
  FT_ATTR_DEV_ENABLE_INPUTS,
  FT_ATTR_DEV_RESERVE_7,
  FT_ATTR_DEV__LAST,
};

enum ft_zattr_in_idx {
  /* PLEASE check "NOTE:" above if you edit this */
  FT_ATTR_TDC_SECONDS = FT_ATTR_DEV__LAST,
  FT_ATTR_TDC_COARSE,
  FT_ATTR_TDC_FRAC,
  FT_ATTR_TDC_TERMINATION,
  FT_ATTR_TDC_ZERO_OFFSET,
  FT_ATTR_TDC_USER_OFFSET,
  FT_ATTR_TDC_DELAY_REF,
  FT_ATTR_TDC_DELAY_REF_SEQ,
  FT_ATTR_TDC_WR_OFFSET,
  FT_ATTR_TDC__LAST,
};

enum ft_zattr_paremeters {
  FT_ATTR_PARAM_TEMP = FT_ATTR_TDC__LAST,
};

enum ft_command {
  FT_CMD_WR_ENABLE = 0, /* Enable White Rabbit */
  FT_CMD_WR_DISABLE,    /* Disable it */
  FT_CMD_WR_QUERY,      /* Check if WR is locked */
  FT_CMD_SET_HOST_TIME, /* Set board time to current host time */
  FT_CMD_IDENTIFY_ON,   /* Identify card by blinking status LEDs, reserved for future use. */
  FT_CMD_IDENTIFY_OFF
};

#endif  // __FMC_TDC_H__
