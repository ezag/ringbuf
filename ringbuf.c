#include <stdlib.h>
#include <string.h>

#include "ringbuf.h"

RingBuf *rbuf_new(size_t size) {
  RingBuf *rbuf = malloc(sizeof(RingBuf));
  if (rbuf == NULL) return NULL;
  rbuf->rbuf_start = malloc(size);
  if (rbuf->rbuf_start == NULL) return NULL;
  rbuf->rbuf_size = size;
  rbuf->rbuf_head = NULL; 
  rbuf->rbuf_tail = rbuf->rbuf_start;
  return rbuf;
}

void rbuf_free(RingBuf *rbuf) {
  free(rbuf->rbuf_start);
  free(rbuf);
}

size_t rbuf_size(const RingBuf *rbuf) {
  return rbuf->rbuf_size;
}

size_t rbuf_can_get(const RingBuf *rbuf) {
  if (rbuf->rbuf_head == NULL) {
    return 0;
  }
  if (rbuf->rbuf_tail == NULL) {
    return rbuf->rbuf_size;
  }
  if (rbuf->rbuf_head < rbuf->rbuf_tail) {
    return (char*)rbuf->rbuf_tail - (char*)rbuf->rbuf_head;
  }
  else {
    return rbuf->rbuf_size - (size_t)(
        (char*)rbuf->rbuf_head - (char*)rbuf->rbuf_tail);
  }
}

size_t rbuf_can_put(const RingBuf *rbuf) {
  return rbuf->rbuf_size - rbuf_can_get(rbuf);
}

typedef struct {
  size_t before_split_size;
  size_t after_split_size;
  size_t total_size;
  const void *split;
  void *fixed;
  void *shifting;
} MemMap;

MemMap map_mem(const void *linear, size_t linear_size,
               const void *circular, size_t circular_size,
               void *fixed, void *shifting) {
  MemMap mm;
  mm.fixed = fixed;
  mm.after_split_size = 0;
  if (shifting == NULL) {
    mm.before_split_size = 0;
    mm.shifting = shifting;
  }
  else {
    if (fixed == NULL || fixed <= shifting) {
      mm.before_split_size = circular_size - (size_t)(
          (char*)shifting - (char*)circular);
      if (fixed == NULL && linear_size > 0) {
        mm.fixed = shifting;
      }
      if (mm.before_split_size < linear_size) {
        const size_t remaining_bytes = linear_size - mm.before_split_size;
        mm.after_split_size = (char*)mm.fixed - (char*)circular;
        if (mm.after_split_size > remaining_bytes) {
          mm.after_split_size = remaining_bytes;
        }
        mm.shifting = (char*)circular + mm.after_split_size;
      }
      else {
        mm.before_split_size = linear_size;
        mm.shifting = (char*)shifting + mm.before_split_size;
      }
    }
    else {
      mm.before_split_size = (char*)mm.fixed - (char*)shifting;
      if (mm.before_split_size > linear_size) {
        mm.before_split_size = linear_size;
      }
      mm.shifting = (char*)shifting + mm.before_split_size;
    }
  }
  mm.split = (char*)linear + mm.before_split_size;
  mm.total_size = mm.before_split_size + mm.after_split_size;
  if (mm.shifting == mm.fixed) {
    mm.shifting = NULL;
  }
  return mm;
}

size_t rbuf_put(RingBuf *rbuf, const void *data, size_t count) {
  MemMap mm = map_mem(data, count, rbuf->rbuf_start, rbuf->rbuf_size,
                      rbuf->rbuf_head, rbuf->rbuf_tail);
  if (mm.before_split_size) {
    memcpy(rbuf->rbuf_tail, data, mm.before_split_size);
  }
  if (mm.after_split_size) {
    memcpy(rbuf->rbuf_start, mm.split, mm.after_split_size);
  }
  rbuf->rbuf_tail = mm.shifting;
  rbuf->rbuf_head = mm.fixed;
  return mm.total_size;
}

size_t rbuf_get(RingBuf *rbuf, void *data, size_t count) {
  MemMap mm = map_mem(data, count, rbuf->rbuf_start, rbuf->rbuf_size,
                      rbuf->rbuf_tail, rbuf->rbuf_head);
  if (mm.before_split_size) {
    memcpy(data, rbuf->rbuf_head, mm.before_split_size);
  }
  if (mm.after_split_size) {
    memcpy(mm.split, rbuf->rbuf_start, mm.after_split_size);
  }
  rbuf->rbuf_head = mm.shifting;
  rbuf->rbuf_tail = mm.fixed;
  return mm.total_size;
}
