/*
 * The fmc-tdc (a.k.a. FmcTdc1ns5cha) library.
 *
 * Copyright (C) 2012-2015 CERN (www.cern.ch)
 * Author: Federico Vaga <federico.vaga@cern.ch
 * Author: Tomasz Włostowski <tomasz.wlostowski@cern.ch>
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2 as published by the Free Software Foundation or, at your
 * option, any later version.
 */
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "zio-user.h"

// clang-format off
#include "fmctdc-lib.h"
#include "fmctdc-lib-private.h"
// clang-format on

//#define GIT_VERSION "aa"
//#define ZIO_GIT_VERSION "aa"

const char *const libfmctdc_version_s = "libfmctdc version: " GIT_VERSION;
const char *const libfmctdc_zio_version_s = "libfmctdc is using zio version: " ZIO_GIT_VERSION;

#define NSAMPLE 1 /* fake number of samples for the TDC */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static struct __fmctdc_board *ft_boards;      /**< list of available boards */
static int ft_nboards;                        /**< number of available boards */
static char *names[] = {"seconds", "coarse"}; /**< names used to retrive
             time-stamps from sysfs */

/**
 * It returns the error message associated to the given error code
 * @param[in] err error code
 */
char *fmctdc_strerror(int err) { return strerror(err); }

/**
 * Init the library. You must call this function before use any other
 * library function. If your system is plug-and-play and TDC devices may
 * appear and disappear at any moment in time, you have to close
 * (fmctdc_exit()) and initialize again (fmctdc_init()) the library in order
 * to get the correct status. Of course, if your applications are using a
 * device and you remove it you will get a library crash.
 * @return the number of boards found
 */
int fmctdc_init(void) {
  glob_t glob_dev, glob_sys;
  struct __fmctdc_board *b;
  int32_t i;
  uint32_t j;
  uint32_t v;

  /* Look for boards in /dev: old and new pathnames: only one matches */
  glob("/dev/tdc-1n5c-*-0-0-ctrl", 0, NULL, &glob_dev);
  glob("/dev/zio/tdc-1n5c-*-0-0-ctrl", GLOB_APPEND, NULL, &glob_dev);
  glob("/dev/zio-tdc-1n5c-*-0-0-ctrl", GLOB_APPEND, NULL, &glob_dev);
  glob("/dev/zio/zio-tdc-1n5c-*-0-0-ctrl", GLOB_APPEND, NULL, &glob_dev);

  /* And look in /sys as well */
  glob("/sys/bus/zio/devices/tdc-1n5c-*", 0, NULL, &glob_sys);
  glob("/sys/bus/zio/devices/zio-tdc-1n5c-*", GLOB_APPEND, NULL, &glob_sys);
  assert(glob_dev.gl_pathc == glob_sys.gl_pathc);

  /* Allocate as needed */
  ft_nboards = glob_dev.gl_pathc;
  if (!ft_nboards) {
    ft_boards = NULL;
    return 0;
  }
  ft_boards = (__fmctdc_board *)calloc(glob_dev.gl_pathc, sizeof(ft_boards[0]));
  if (!ft_boards) {
    globfree(&glob_dev);
    globfree(&glob_sys);
    return -1;
  }

  for (i = 0, b = ft_boards; i < ft_nboards; i++, b++) {
    b->sysbase = strdup(glob_sys.gl_pathv[i]);
    b->devbase = strdup(glob_dev.gl_pathv[i]);
    /* trim the "-0-0-ctrl" at the end */
    b->devbase[strlen(b->devbase) - strlen("-0-0-ctrl")] = '\0';
    /* extract dev_id */
    uint32_t dev_id;
    sscanf(b->sysbase, "%*[^t]tdc-1n5c-%x", &dev_id);
    b->dev_id = dev_id;
    for (j = 0; j < ARRAY_SIZE(b->fdc); j++) {
      b->fdc[j] = -1;
      b->fdd[j] = -1;
    }
    if (fmctdc_is_verbose()) {
      fprintf(stderr, "%s: %04x %s %s\n", __func__, b->dev_id, b->sysbase, b->devbase);
    }
  }
  globfree(&glob_dev);
  globfree(&glob_sys);

  /* Now, if at least one board is there, check the version */
  if (ft_nboards == 0) return 0;

  if (fmctdc_sysfs_get(ft_boards, "version", &v) < 0) return -1;

  if (v != FT_VERSION_MAJ) {
    fprintf(stderr, "%s: version mismatch, lib(%i) != drv(%i)\n", __func__, FT_VERSION_MAJ, v);
    errno = EIO;
    return -1;
  }
  return ft_nboards;
}

/**
 * It releases all the resources used by the library and allocated
 * by fmctdc_init(). Once you call this function you cannot use other function
 * from this library.
 */
void fmctdc_exit(void) {
  struct __fmctdc_board *b;
  int err;
  int32_t i;
  uint32_t j;

  for (i = 0, err = 0, b = ft_boards; i < ft_nboards; i++, b++) {
    for (j = 0; j < ARRAY_SIZE(b->fdc); j++) {
      if (b->fdc[j] >= 0) {
        close(b->fdc[j]);
        b->fdc[j] = -1;
        err++;
      }
      if (b->fdd[j] >= 0) {
        close(b->fdd[j]);
        b->fdd[j] = -1;
        err++;
      }
    }
    if (err) fprintf(stderr, "%s: device %s was still open\n", __func__, b->devbase);
    free(b->sysbase);
    free(b->devbase);
  }
  if (ft_nboards) free(ft_boards);
}

/**
 * It opens one specific device. -1 arguments mean "not installed"
 * @param[in] offset board enumeration offset [0, N]. -1 to ignore it and
 *                   use only dev_id
 * @param[in] dev_id FMC device id. -1 to ignore it and use only the offset
 * @return an instance token, otherwise NULL and errno is appripriately set.
 *         ENODEV if the device was not found. EINVAL there is a mismatch with
 *         the arguments
 */
struct fmctdc_board *fmctdc_open(int offset, int dev_id) {
  struct __fmctdc_board *b = NULL;
  uint32_t nsamples = NSAMPLE;
  char path[128];
  int i;

  if (offset >= ft_nboards) {
    errno = ENODEV;
    return NULL;
  }
  if (offset >= 0) {
    b = ft_boards + offset;
    if (dev_id >= 0 && dev_id != b->dev_id) {
      errno = EINVAL;
      return NULL;
    }
    goto found;
  }
  if (dev_id < 0) {
    errno = EINVAL;
    return NULL;
  }
  for (i = 0, b = ft_boards; i < ft_nboards; i++, b++)
    if (b->dev_id == dev_id) goto found;
  errno = ENODEV;
  return NULL;

found:
  /* Trim all block sizes to 1 sample (i.e. 4 bytes) */
  fmctdc_sysfs_set(b, "ft-ch1/trigger/post-samples", &nsamples);
  fmctdc_sysfs_set(b, "ft-ch2/trigger/post-samples", &nsamples);
  fmctdc_sysfs_set(b, "ft-ch3/trigger/post-samples", &nsamples);
  fmctdc_sysfs_set(b, "ft-ch4/trigger/post-samples", &nsamples);
  fmctdc_sysfs_set(b, "ft-ch5/trigger/post-samples", &nsamples);

  for (i = 0; i < FMCTDC_NUM_CHANNELS; i++) {
    b->fdc[i] = -1;
    b->fdd[i] = -1;
    b->fdcc[i] = -1;

    /* Open Control */
    snprintf(path, sizeof(path), "%s-%d-0-ctrl", b->devbase, i);
    b->fdc[i] = open(path, O_RDONLY | O_NONBLOCK);
    if (b->fdc[i] < 0) goto error;
    /* Open Data - even if not really used for the time being */
    snprintf(path, sizeof(path), "%s-%d-0-data", b->devbase, i);
    b->fdd[i] = open(path, O_RDONLY | O_NONBLOCK);
    if (b->fdd[i] < 0) goto error;
    /* Open Current Control */
    snprintf(path, sizeof(path), "%s/ft-ch%d/chan0/current-control", b->sysbase, i + 1);
    b->fdcc[i] = open(path, O_RDONLY);
    if (b->fdcc[i] < 0) goto error;
  }

  return (fmctdc_board *)b;

error:
  while (i--) {
    if (b->fdc[i] >= 0) close(b->fdc[i]);
    if (b->fdd[i] >= 0) close(b->fdd[i]);
    if (b->fdcc[i] >= 0) close(b->fdcc[i]);
  }
  return NULL;
}

/**
 * It opens one specific device by logical unit number (CERN/BE-CO-like).
 * The function uses a symbolic link in /dev that points to the standard device.
 * The link is created by the local installation procedure, and it allows to get
 * the device id according to the LUN.
 * Read also fmctdc_open() documentation.
 * @param[in] lun Logical Unit Number
 * @return an instance token, otherwise NULL and errno is appripriately set
 */
struct fmctdc_board *fmctdc_open_by_lun(int lun) {
  ssize_t ret;
  char dev_id_str[4];
  char path_pattern[] = "/dev/fmc-tdc.%d";
  char path[sizeof(path_pattern) + 1];
  uint32_t dev_id;

  ret = snprintf(path, sizeof(path), path_pattern, lun);
  if (ret < 0 || (size_t)ret >= sizeof(path)) {
    errno = EINVAL;
    return NULL;
  }
  ret = readlink(path, dev_id_str, sizeof(dev_id_str));
  if (sscanf(dev_id_str, "%4x", &dev_id) != 1) {
    errno = ENODEV;
    return NULL;
  }

  return fmctdc_open(-1, int(dev_id));
}

/**
 * It closes a TDC instance opened with fmctdc_open() or fmctdc_open_by_lun()
 * @param[in] userb TDC board instance token
 * @return 0 on success, otherwise -1 and errno is set appropriately
 */
int fmctdc_close(struct fmctdc_board *userb) {
  __define_board(b, userb);
  uint32_t j;

  for (j = 0; j < ARRAY_SIZE(b->fdc); j++) {
    if (b->fdc[j] >= 0) close(b->fdc[j]);
    b->fdc[j] = -1;
    if (b->fdd[j] >= 0) close(b->fdd[j]);
    b->fdd[j] = -1;
    if (b->fdcc[j] >= 0) close(b->fdcc[j]);
    b->fdcc[j] = -1;
  }
  return 0;
}

/**
 * It reads the current temperature of a TDC device
 * @param[in] userb TDC board instance token
 * @return temperature
 */
float fmctdc_read_temperature(struct fmctdc_board *userb) {
  uint32_t t;
  __define_board(b, userb);

  fmctdc_sysfs_get(b, "temperature", &t);
  return (float)t / 16.0;
}

/**
 * The function enables/disables the 50 Ohm termination of the given channel.
 * Termination may be changed anytime.
 * @param[in] userb TDC board instance token
 * @param[in] channel to use
 * @param[in] on status of the termination to set
 * @return 0 on success, otherwise a negative errno code is set
 *         appropriately
 */
int fmctdc_set_termination(struct fmctdc_board *userb, unsigned int channel, int on) {
  __define_board(b, userb);
  uint32_t val;
  char attr[32];

  if (channel >= FMCTDC_NUM_CHANNELS) {
    errno = EINVAL;
    return -1;
  }

  snprintf(attr, sizeof(attr), "ft-ch%d/termination", channel + 1);

  val = on ? 1 : 0;
  return fmctdc_sysfs_set(b, attr, &val);
}

/**
 * The function returns current temrmination status: 0 if the given channel
 * is high-impedance and positive if it is 50 Ohm-terminated.
 * @param[in] userb TDC board instance token
 * @param[in] channel to use
 * @return termination status, otherwise a negative errno code is set
 *         appropriately
 */
int fmctdc_get_termination(struct fmctdc_board *userb, unsigned int channel) {
  __define_board(b, userb);
  uint32_t val;
  char attr[32];
  int ret;

  if (channel >= FMCTDC_NUM_CHANNELS) {
    errno = EINVAL;
    return -1;
  }

  snprintf(attr, sizeof(attr), "ft-ch%d/termination", channel + 1);

  ret = fmctdc_sysfs_get(b, attr, &val);
  if (ret) return ret;
  return val;
}

/**
 * It gets the acquisition status of a TDC channel
 * @param[in] userb TDC board instance token
 * @param[in] channel channel to which we want read the status
 * @return the acquisition status (0 disabled, 1 enabled), otherwise -1 and
 *         errno is set appropriately
 */
int fmctdc_channel_status_get(struct fmctdc_board *userb, unsigned int channel) {
  __define_board(b, userb);
  uint32_t val;
  char attr[64];
  int ret;

  if (channel >= FMCTDC_NUM_CHANNELS) {
    errno = EINVAL;
    return -1;
  }

  snprintf(attr, sizeof(attr), "ft-ch%d/enable", channel + 1);

  ret = fmctdc_sysfs_get(b, attr, &val);
  if (ret) return ret;
  return val;
}

/**
 * The function enables/disables timestamp acquisition for the given channel.
 * @param[in] userb TDC board instance token
 * @param[in] channel channel to which we want change status
 * @param[in] status enable status to set
 * @return 0 on success, otherwise -1 and errno is set appropriately
 */
int fmctdc_channel_status_set(struct fmctdc_board *userb, unsigned int channel, enum fmctdc_channel_status status) {
  __define_board(b, userb);
  uint32_t val = status;
  char attr[64];

  if (channel >= FMCTDC_NUM_CHANNELS) {
    errno = EINVAL;
    return -1;
  }

  snprintf(attr, sizeof(attr), "ft-ch%d/enable", channel + 1);

  return fmctdc_sysfs_set(b, attr, &val);
}

/**
 * It enables a given channel.
 * NOTE: it is just a wrapper of fmctdc_channel_status_set()
 * @param[in] userb TDC board instance token
 * @param[in] channel channel to which we want change status
 * @return 0 on success, otherwise -1 and errno is set appropriately
 */
int fmctdc_channel_enable(struct fmctdc_board *userb, unsigned int channel) {
  return fmctdc_channel_status_set(userb, channel, FMCTDC_STATUS_ENABLE);
}

/**
 * It disable a given channel.
 * NOTE: it is just a wrapper of fmctdc_channel_status_set()
 * @param[in] userb TDC board instance token
 * @param[in] channel channel to which we want change status
 * @return 0 on success, otherwise -1 and errno is set appropriately
 */
int fmctdc_channel_disable(struct fmctdc_board *userb, unsigned int channel) {
  return fmctdc_channel_status_set(userb, channel, FMCTDC_STATUS_DISABLE);
}

/**
 * The function returns current buffer mode: 0 for FIFO, 1 for circular buffer.
 * @param[in] userb TDC board instance token
 * @param[in] channel to use
 * @return buffer mode, otherwise a negative errno code is set
 *         appropriately
 */
int fmctdc_get_buffer_mode(struct fmctdc_board *userb, unsigned int channel) {
  __define_board(b, userb);
  uint32_t val;
  char attr[64];
  int ret;

  if (channel >= FMCTDC_NUM_CHANNELS) {
    errno = EINVAL;
    return -1;
  }

  snprintf(attr, sizeof(attr), "ft-ch%d/chan0/buffer/prefer-new", channel + 1);

  ret = fmctdc_sysfs_get(b, attr, &val);
  if (ret) return ret;
  return val;
}

/**
 * The function sets the buffer mode for a channel
 * @param[in] userb TDC board instance token
 * @param[in] channel to use
 * @param[in] mode buffer mode to use
 * @return 0 on success, otherwise a negative errno code is set
 *         appropriately
 */
int fmctdc_set_buffer_mode(struct fmctdc_board *userb, unsigned int channel, enum fmctdc_buffer_mode mode) {
  __define_board(b, userb);
  uint32_t val;
  char attr[64];

  if (channel >= FMCTDC_NUM_CHANNELS) {
    errno = EINVAL;
    return -1;
  }

  snprintf(attr, sizeof(attr), "ft-ch%d/chan0/buffer/prefer-new", channel + 1);

  val = mode;
  return fmctdc_sysfs_set(b, attr, &val);
}

/**
 * The function returns current buffer lenght (number of timestamps)
 * @param[in] userb TDC board instance token
 * @param[in] channel to use
 * @return buffer lenght, otherwise a negative errno code is set
 *         appropriately
 */
int fmctdc_get_buffer_len(struct fmctdc_board *userb, unsigned int channel) {
  __define_board(b, userb);
  uint32_t val;
  char attr[64];
  int ret;

  if (channel >= FMCTDC_NUM_CHANNELS) {
    errno = EINVAL;
    return -1;
  }

  snprintf(attr, sizeof(attr), "ft-ch%d/chan0/buffer/max-buffer-len", channel + 1);

  ret = fmctdc_sysfs_get(b, attr, &val);
  if (ret) return ret;
  return val;
}

/**
 * The function set the buffer lenght
 * @param[in] userb TDC board instance token
 * @param[in] channel to use
 * @param[in] lenght maximum number of timestamps to store
 * @return 0 on success, otherwise a negative errno code is set
 *         appropriately
 */
int fmctdc_set_buffer_len(struct fmctdc_board *userb, unsigned int channel, unsigned int lenght) {
  __define_board(b, userb);
  uint32_t val;
  char attr[64];

  if (channel >= FMCTDC_NUM_CHANNELS) {
    errno = EINVAL;
    return -1;
  }

  snprintf(attr, sizeof(attr), "ft-ch%d/chan0/buffer/max-buffer-len", channel + 1);

  val = lenght;
  return fmctdc_sysfs_set(b, attr, &val);
}

/**
 * It get the file descriptor of a TDC channel. So, for example, you can
 * poll(2) and select(2).
 * Note that, the file descriptor is the file-descriptor of a
 * ZIO control char-device.
 * @param[in] userb TDC board instance token
 * @param[in] channel channel to use
 * @return a file descriptor, otherwise -1 and errno is set appropriately
 */
int fmctdc_fileno_channel(struct fmctdc_board *userb, unsigned int channel) {
  __define_board(b, userb);

  return b->fdc[channel];
}

/**
 * It reads the very last time-stamp produced by a given channel. Note that
 * the last time-stamp is the last produced by the hardware and not the last
 * read by the user. Between the hardware and the user there is a buffer.
 * @param[in] userb TDC board instance token
 * @param[in] channel channel to use [0, 4]
 * @param[out] t where to write the time-stamps
 * @return number of acquired time-stamps, otherwise -1 and errno is set
 */
int fmctdc_read_last(struct fmctdc_board *userb, unsigned int channel, __attribute__((unused)) struct fmctdc_time *t) {
  __define_board(b, userb);
  struct zio_control ctrl;
  int n;

  if (channel >= FMCTDC_NUM_CHANNELS) {
    errno = EINVAL;
    return -1;
  }

  n = read(b->fdcc[channel], &ctrl, sizeof(struct zio_control));
  if (n != sizeof(struct zio_control)) return -1;
  return 1;
}

/**
 * It reads a given number of time-stamps from the driver. It will wait at
 * most once and return the number of samples that it received from a given
 * input channel. According to the 'mode' in use the meaning of the time-stamp
 * is different.
 *
 * When there is no channel reference for the given channel, the time-stamp
 * is the time-stamp according to the base time.
 *
 * When the reference is setted for the given channel, the time-stamp is the
 * time difference between the the current pulse on the given channel and the
 * last pulse of the reference channel.
 *
 * This "read" behaves like the system call and obeys O_NONBLOCK
 * @param[in] userb TDC board instance token
 * @param[in] channel channel to use [0, 4]
 * @param[out] t array of time-stamps
 * @param[in] n number of elements to save in the array
 * @param[in] flags tune the behaviour of the function.
 *                      O_NONBLOCK - do not block
 * @return number of acquired time-stamps, otherwise -1 and errno is set
 *         appropriately.
 *         - EINVAL for invalid arguments
 *         - EIO for invalid IO transfer
 *         - EAGAIN if nothing ready to read in NONBLOCK mode
 */
int fmctdc_read(struct fmctdc_board *userb, unsigned int channel, struct fmctdc_time *t, int n, int flags) {
  __define_board(b, userb);
  struct zio_control ctrl;
  uint32_t *attrs, data[NSAMPLE]; /* ssize is 4 => uint32_t for data */
  int i, j;
  fd_set set;

  if (channel >= FMCTDC_NUM_CHANNELS) {
    errno = EINVAL;
    return -1;
  }

  for (i = 0; i < n;) {
    j = read(b->fdc[channel], &ctrl, sizeof(ctrl));
    if (j < 0 && errno != EAGAIN) return -1;
    if (j == sizeof(ctrl)) {
      /* one sample: pick it */
      attrs = ctrl.attr_channel.ext_val;
      t[i].seconds = ctrl.tstamp.secs;
      t[i].coarse = ctrl.tstamp.ticks;
      t[i].frac = ctrl.tstamp.bins;
      t[i].seq_id = ctrl.seq_num;
      t[i].gseq_id = attrs[FT_ATTR_DEV_SEQUENCE];
      t[i].ref_gseq_id = attrs[FT_ATTR_TDC_DELAY_REF_SEQ];
      i++;

      /* Consume also the data even if it is empty,
         so it will keep clear the ZIO buffer */
      j = read(b->fdd[channel], data, ctrl.nsamples * ctrl.ssize);
      if ((unsigned int)j == ctrl.nsamples * ctrl.ssize) continue; /* Everything is fine */
                                                                   /* We are not ok here because the data side has
                                                                      something wrong */
    }
    if (j > 0) {
      errno = EIO;
      return -1;
    }
    /* so, it's EAGAIN: if we already got something, we are done */
    if (i) return i;
    /* EAGAIN at first sample */
    if (j < 0 && flags == O_NONBLOCK) return -1;

    /* So, first sample and blocking read. Wait.. */
    FD_ZERO(&set);
    FD_SET(b->fdc[channel], &set);
    if (select(b->fdc[channel] + 1, &set, NULL, NULL, NULL) < 0) return -1;
    continue;
  }
  return i;
}

/**
 * this "fread" behaves like stdio: it reads all the samples. Read fmctdc_read()
 * for more details about the function.
 * @param[in] userb TDC board instance token
 * @param[in] channel channel to use
 * @param[out] t array of time-stamps
 * @param[in] n number of elements to save in the array
 * @return number of acquired time-stamps, otherwise -1 and errno is set
 *         appropriately
 */
int fmctdc_fread(struct fmctdc_board *userb, unsigned int channel, struct fmctdc_time *t, int n) {
  int i, loop;

  for (i = 0; i < n;) {
    loop = fmctdc_read(userb, channel, t + i, n - i, 0);
    if (loop < 0) return -1;
    i += loop;
  }
  return i;
}

/**
 * It sets the TDC base-time according to the given time-stamp.
 * Note that, for the time being, it sets only seconds.
 * Note that, you can set the time only when the acquisition is disabled.
 * @param[in] userb TDC board instance token
 * @param[in] t time-stamp
 * @return 0 on success, otherwise -1 and errno is set
 */
int fmctdc_set_time(struct fmctdc_board *userb, struct fmctdc_time *t) {
  __define_board(b, userb);
  uint32_t attrs[ARRAY_SIZE(names)];
  int i, ret;

  attrs[0] = t->seconds & 0xffffffff;
  attrs[1] = t->coarse;

  for (i = ARRAY_SIZE(names) - 1; i >= 0; i--) {
    ret = fmctdc_sysfs_set(b, names[i], attrs + i);
    if (ret < 0) return ret;
  }
  return 0;
}

/**
 * It gets the base-time of a TDC device.
 * Note that, for the time being, it gets only seconds.
 * @param[in] userb TDC board instance token
 * @param[out] t time-stamp
 * @return 0 on success, otherwise -1 and errno is set
 */
int fmctdc_get_time(struct fmctdc_board *userb, struct fmctdc_time *t) {
  __define_board(b, userb);
  uint32_t attrs[ARRAY_SIZE(names)];
  int ret;
  uint32_t i;

  for (i = 0; i < ARRAY_SIZE(names); i++) {
    ret = fmctdc_sysfs_get(b, names[i], attrs + i);
    if (ret < 0) return ret;
  }

  t->seconds = attrs[0];
  t->coarse = attrs[1];
  t->frac = 0;

  return 0;
}

/**
 * It sets the TDC base-time according to the host time
 * @param[in] userb TDC board instance token
 * @return 0 on success, otherwise -1 and errno is set appropriately
 */
int fmctdc_set_host_time(struct fmctdc_board *userb) {
  __define_board(b, userb);

  return __fmctdc_command(b, FT_CMD_SET_HOST_TIME);
}

/**
 * It enables/disables the WhiteRabbit timing system on a TDC device
 * @param[in] userb TDC board instance token
 * @param[in] on white-rabbit status to set
 * @return 0 on success, otherwise an error code
 */
int fmctdc_wr_mode(struct fmctdc_board *userb, int on) {
  __define_board(b, userb);

  if (on)
    __fmctdc_command(b, FT_CMD_WR_ENABLE);
  else
    __fmctdc_command(b, FT_CMD_WR_DISABLE);
  return errno;
}

/**
 * It check the current status of the WhiteRabbit timing system on a TDC device
 * @param[in] userb TDC board instance token
 * @return 0 if it properly works, -1 on error and errno is set appropriately.
 *         - ENOLINK if it is not synchronized and
 *         - ENODEV if it is not enabled
 */
extern int fmctdc_check_wr_mode(struct fmctdc_board *userb) {
  __define_board(b, userb);

  if (__fmctdc_command(b, FT_CMD_WR_QUERY) == 0) return 0;
  return -1;
}

/**
 * It assigns a time reference to a target channel. After you set a reference,
 * you will read, from the target channel, the time-stamp difference between
 * the last reference pulse and the target pulse.
 * @param[in] userb TDC board instance token
 * @param[in] ch_target target channel [0, 4]
 * @param[in] ch_reference reference channel [0, 4]. Use -1 to remove reference
 * @return 0 on success, otherwise -1 and errno is set appropriately
 */
int fmctdc_reference_set(struct fmctdc_board *userb, unsigned int ch_target, int ch_reference) {
  __define_board(b, userb);
  uint32_t ch_ref = ch_reference;
  char path[64];

  if (ch_target >= FMCTDC_NUM_CHANNELS || ch_reference >= FMCTDC_NUM_CHANNELS) {
    errno = EINVAL;
    return -1;
  }

  snprintf(path, sizeof(path), "ft-ch%d/diff-reference", ch_target + 1);
  ch_ref++; /* for the driver channel interval is [1, 5] */
  return fmctdc_sysfs_set(b, path, &ch_ref);
}

/**
 * It removes the time reference from a target channel
 * @param[in] userb TDC board instance token
 * @param[in] ch_target target channel [1, 5]
 * @return 0 on success, otherwise -1 and errno is set appropriately
 */
int fmctdc_reference_clear(struct fmctdc_board *userb, int ch_target) {
  return fmctdc_reference_set(userb, ch_target, 0);
}

/**
 * It get the current reference channel of a given target channel
 * @param[in] userb TDC board instance token
 * @param[in] ch_target target channel [0, 4]
 * @return the number of the reference channel [0, 4]on success, otherwise -1 and
 *         errno is set appropriately
 */
int fmctdc_reference_get(struct fmctdc_board *userb, unsigned int ch_target) {
  __define_board(b, userb);
  uint32_t ch_ref;
  char path[64];
  int err;

  if (ch_target >= FMCTDC_NUM_CHANNELS) {
    errno = EINVAL;
    return -1;
  }
  snprintf(path, sizeof(path), "ft-ch%d/diff-reference", ch_target + 1);
  err = fmctdc_sysfs_get(b, path, &ch_ref);
  if (err) return -1;
  return ch_ref - 1; /* For the driver channel interval is [1, 5]*/
}

/**
 * It removes all samples from the channel buffer. In order to doing this,
 * the function temporary disable any active acquisition, only when the flush
 * is completed the acquisition will be re-enabled
 * @param[in] userb TDC board instance token
 * @param[in] channel target channel [0, 4]
 * @return 0 on success, otherwise -1 and errno is set appropriately
 */
int fmctdc_flush(struct fmctdc_board *userb, unsigned int channel) {
  __define_board(b, userb);
  char path[64];
  int en, err;

  if (channel >= FMCTDC_NUM_CHANNELS) {
    errno = EINVAL;
    return -1;
  }
  en = fmctdc_channel_status_get(userb, channel);
  if (en < 0) return -1;

  /* Disable acquisition, it will flush the hw buffer */
  err = fmctdc_channel_status_set(userb, channel, FMCTDC_STATUS_DISABLE);
  if (err) return err;

  /* Flush ZIO buffer */
  snprintf(path, sizeof(path), "ft-ch%d/chan0/buffer/flush", channel + 1);
  err = fmctdc_sysfs_set(b, path, &channel);
  if (err) {
    return err;
  }

  /* Re-enable if it was enable */
  return fmctdc_channel_status_set(userb, channel, (fmctdc_channel_status)(en));
}

/**
 * It sets the user offset to be applied on incoming timestamps. All the
 * timestamps read from the driver (this means also from this library) will
 * be already corrected using this offset.
 * @param[in] userb TDC board instance token
 * @param[in] channel target channel [0, 4]
 * @param[in] offset the number of pico-seconds to be added
 * @return 0 on success, otherwise -1 and errno is set appropriately
 */
int fmctdc_set_offset_user(struct fmctdc_board *userb, unsigned int channel, int32_t offset) {
  __define_board(b, userb);
  uint32_t val = (uint32_t)offset;
  char attr[64];

  if (channel >= FMCTDC_NUM_CHANNELS) {
    errno = EINVAL;
    return -1;
  }

  snprintf(attr, sizeof(attr), "ft-ch%d/user-offset", channel + 1);

  return fmctdc_sysfs_set(b, attr, &val);
}

/**
 * It get the current user offset applied to the incoming timestamps
 * @param[in] userb TDC board instance token
 * @param[in] channel target channel [0, 4]
 * @param[out] offset the number of pico-seconds to be added
 * @return 0 on success, otherwise -1 and errno is set appropriately
 */
int fmctdc_get_offset_user(struct fmctdc_board *userb, unsigned int channel, int32_t *offset) {
  __define_board(b, userb);
  uint32_t val;
  char path[64];
  int err;

  if (channel >= FMCTDC_NUM_CHANNELS) {
    errno = EINVAL;
    return -1;
  }
  snprintf(path, sizeof(path), "ft-ch%d/user-offset", channel + 1);
  err = fmctdc_sysfs_get(b, path, &val);
  if (err) return -1;

  *offset = (int32_t)val;
  return 0;
}
