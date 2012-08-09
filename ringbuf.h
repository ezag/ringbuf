/* Project page: https://github.com/ezag/ringbuf
   License: MIT (see LICENSE file or http://opensource.org/licenses/mit)
   Copyright (c) 2012 Eugen Zagorodniy <https://github.com/ezag>
*/
#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdlib.h>

typedef struct {
	size_t rbuf_size;
	void *rbuf_start;
	void *rbuf_head;
	void *rbuf_tail;
} RingBuf;

RingBuf *rbuf_new(size_t size);
void rbuf_free(RingBuf *rbuf);

size_t rbuf_size(const RingBuf *rbuf);
size_t rbuf_can_get(const RingBuf *rbuf);
size_t rbuf_can_put(const RingBuf *rbuf);

size_t rbuf_put(RingBuf *rbuf, const void *data, size_t count);
size_t rbuf_get(RingBuf *rbuf, void *data, size_t count);

#endif
