/*
 * The "official" fmc-tdc API
 *
 * Copyright (C) 2012-2013 CERN (www.cern.ch)
 * Author: Tomasz Włostowski <tomasz.wlostowski@cern.ch>
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2 as published by the Free Software Foundation or, at your
 * option, any later version.
 */
#ifndef __FMCTDC_LIB_H__
#define __FMCTDC_LIB_H__

#ifdef __cplusplus
#pragma GCC diagnostic ignored "-Wwrite-strings"
extern "C" {
#endif

#include <stdint.h>

/**
 * Enumeration for all TDC channels
 */
enum fmctdc_channel {
  FMCTDC_CH_1 = 0,
  FMCTDC_CH_2,
  FMCTDC_CH_3,
  FMCTDC_CH_4,
  FMCTDC_CH_5,
  FMCTDC_CH_LAST = FMCTDC_CH_5,
  FMCTDC_NUM_CHANNELS = 5
};

/**
 * Enumeration of all buffer modes
 */
enum fmctdc_buffer_mode {
  FMCTDC_BUFFER_FIFO = 0, /**< FIFO policy: when buffer is full, new
         time-stamps will be dropped */
  FMCTDC_BUFFER_CIRC,     /**< circular buffer policy: when the buffer is
                 full, old time-stamps will be overwritten by
                 new ones */
};

/**
 * Enumeration for all possible status of a channel
 */
enum fmctdc_channel_status {
  FMCTDC_STATUS_DISABLE = 0, /**< The cannel is disable */
  FMCTDC_STATUS_ENABLE,      /**< the channel is enable */
};

/**
 * Opaque data type used as token. Do not try to access.
 */
struct fmctdc_board;

/**
 * FMC-TDC time-stamp descriptor
 */
struct fmctdc_time {
  uint64_t seconds;     /**< TAI seconds. Note this is *not* an UTC time;
               the counter does not support leap seconds. The
               internal counter is also limited to 32 bits
               (2038-error-prone). */
  uint32_t coarse;      /**< number of ticks of 8ns since the beginning of
               the last second*/
  uint32_t frac;        /**< fractional part of an 8 ns tick, rescaled
               to (0..4095) range - i.e. 0 = 0 ns, and
               4095 = 7.999 ns. */
  uint32_t seq_id;      /**< channel sequence number*/
  uint32_t gseq_id;     /**< board sequence number */
  uint32_t ref_gseq_id; /**< board sequence number of the reference
         channel time-stamp */
};

/**
 * @file fmctdc-lib.c
 */
/**
 * @defgroup libutil Utilities
 * Set of library utilities
 * @{
 */
extern char *fmctdc_strerror(int err);
extern int fmctdc_init(void);
extern void fmctdc_exit(void);
/**@}*/

/**
 * @defgroup libboard Board Configuration
 * Set of function to configure TDC board and retrieve information
 * about the current status
 * @{
 */
extern int fmctdc_set_time(struct fmctdc_board *b, struct fmctdc_time *t);
extern int fmctdc_get_time(struct fmctdc_board *b, struct fmctdc_time *t);
extern int fmctdc_set_host_time(struct fmctdc_board *b);
extern int fmctdc_wr_mode(struct fmctdc_board *b, int on);
extern int fmctdc_check_wr_mode(struct fmctdc_board *b);
extern float fmctdc_read_temperature(struct fmctdc_board *b);
/**@}*/

/**
 * @defgroup libchan Channel Configuration
 * Set of function to configure TDC channels and retrieve information
 * about the current status
 * @{
 */
extern int fmctdc_channel_status_set(struct fmctdc_board *userb, unsigned int channel,
                                     enum fmctdc_channel_status status);
extern int fmctdc_channel_enable(struct fmctdc_board *userb, unsigned int channel);
extern int fmctdc_channel_disable(struct fmctdc_board *userb, unsigned int channel);
extern int fmctdc_channel_status_get(struct fmctdc_board *userb, unsigned int channel);
extern int fmctdc_set_termination(struct fmctdc_board *b, unsigned int channel, int enable);
extern int fmctdc_get_termination(struct fmctdc_board *b, unsigned int channel);
extern int fmctdc_get_buffer_mode(struct fmctdc_board *userb, unsigned int channel);
extern int fmctdc_set_buffer_mode(struct fmctdc_board *userb, unsigned int channel, enum fmctdc_buffer_mode mode);
extern int fmctdc_get_buffer_len(struct fmctdc_board *userb, unsigned int channel);
extern int fmctdc_set_buffer_len(struct fmctdc_board *userb, unsigned int channel, unsigned int lenght);
extern int fmctdc_reference_set(struct fmctdc_board *userb, unsigned int ch_target, int ch_reference);
extern int fmctdc_reference_get(struct fmctdc_board *userb, unsigned int ch_target);
extern int fmctdc_reference_clear(struct fmctdc_board *userb, int ch_target);
extern int fmctdc_set_offset_user(struct fmctdc_board *userb, unsigned int channel, int32_t offset);
extern int fmctdc_get_offset_user(struct fmctdc_board *userb, unsigned int channel, int32_t *offset);
/**@}*/

/**
 * @defgroup libacq Time-stamps Acquisition
 * Set of functions to read time-stamps from the board
 * @{
 */
extern struct fmctdc_board *fmctdc_open(int offset, int dev_id);
extern struct fmctdc_board *fmctdc_open_by_lun(int lun);
extern int fmctdc_close(struct fmctdc_board *);
extern int fmctdc_fread(struct fmctdc_board *b, unsigned int channel, struct fmctdc_time *t, int n);
extern int fmctdc_fileno_channel(struct fmctdc_board *b, unsigned int channel);
extern int fmctdc_read(struct fmctdc_board *b, unsigned int channel, struct fmctdc_time *t, int n, int flags);
extern int fmctdc_read_last(struct fmctdc_board *userb, unsigned int channel, struct fmctdc_time *t);
extern int fmctdc_flush(struct fmctdc_board *userb, unsigned int channel);
/**@}*/

/**
 *@file fmctdc-lib-math.c
 */
/**
 * @defgroup libmath Time-Stamp Math
 * Set of mathematical functions on time-stamps
 * @{
 */

extern uint64_t fmctdc_ts_approx_ns(struct fmctdc_time *a);
extern uint64_t fmctdc_ts_ps(struct fmctdc_time *a);
extern void fmctdc_ts_sub(struct fmctdc_time *a, struct fmctdc_time *b);
extern void ft_ts_add(struct fmctdc_time *a, struct fmctdc_time *b);
extern int _fmctdc_tscmp(struct fmctdc_time *a, struct fmctdc_time *b);
/**@}*/

/**
 * libfmctdc version string
 */
extern const char *const libfmctdc_version_s;

/**
 * zio version string used during compilation of libfmctdc
 */
extern const char *const libfmctdc_zio_version_s;

#ifdef __cplusplus
}
#endif

#endif /* __FMCTDC_LIB_H__ */
