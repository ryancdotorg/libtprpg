/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

static int urandom_fd = -1;

void randombytes(uint8_t *ptr, ssize_t len) {
  if (urandom_fd < 0) {
    if ((urandom_fd = open("/dev/urandom", O_RDONLY)) < 0) {
      perror("error opening random");
      exit(1);
    }
  }
  if (read(urandom_fd, ptr, len) != len) {
    perror("error reading random");
    exit(1);
  }
}

/* 
 * vim: ts=2 sw=2 et ai si bg=dark
 */
