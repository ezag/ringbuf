#include <glib.h>
#include "ringbuf.c"

#define assert_cmpstrn(s1, cmp, s2, n) do { \
  char *__s1 = malloc((n) + 1), *__s2 = malloc((n) + 1); \
  strncpy(__s1, (s1), (n)); __s1[(n)] = '\0'; \
  strncpy(__s2, (s2), (n)); __s2[(n)] = '\0'; \
  if (strncmp (__s1, __s2, (n)) cmp 0) ; else  { \
    g_assertion_message_cmpstr ( \
      G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, \
      #s1 " " #cmp " " #s2, __s1, #cmp, __s2 \
    );} free(__s1); free(__s2); \
  } while (0)

#define DST_SIZE 64

char DST[DST_SIZE];
char PAD[] = "..............................................................";
char SRC[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcderghijklmnopqrstuvwxyz";

const size_t BUF_SIZE = 16;
const size_t PART_SIZE = 6;
const size_t BOUNDS[2] = {6, 12};

void untouched(RingBuf **rbuf, const void *data) {
  *rbuf = rbuf_new(BUF_SIZE);
  g_assert(*rbuf != NULL);
  g_assert_cmpuint(rbuf_size(*rbuf), ==, BUF_SIZE);
  g_assert_cmpuint(rbuf_can_get(*rbuf), ==, 0);
}

void from_start(RingBuf **rbuf, const void *data) {
  const size_t shift = *((size_t*)data);
  g_assert_cmpuint(shift, <=, BUF_SIZE);
  untouched(rbuf, NULL);
  g_assert_cmpuint(rbuf_put(*rbuf, PAD, shift), ==, shift);
  g_assert_cmpuint(rbuf_can_get(*rbuf), ==, shift);
}

void empty(RingBuf **rbuf, const void *data) {
  const size_t shift = *((size_t*)data);
  from_start(rbuf, &shift);
  g_assert_cmpuint(rbuf_get(*rbuf, DST, shift), ==, shift);
  g_assert_cmpuint(rbuf_can_get(*rbuf), ==, 0);
  g_assert_cmpuint(rbuf_can_put(*rbuf), ==, BUF_SIZE);
}

void to_end(RingBuf **rbuf, const void *data) {
  const size_t shift = *((size_t*)data);
  g_assert_cmpuint(shift, <=, BUF_SIZE);
  untouched(rbuf, NULL);
  g_assert_cmpuint(rbuf_put(*rbuf, PAD, BUF_SIZE), ==, BUF_SIZE);
  g_assert_cmpuint(rbuf_can_get(*rbuf), ==, BUF_SIZE);
  g_assert_cmpuint(rbuf_get(*rbuf, DST, shift), ==, shift);
}

void inside(RingBuf **rbuf, const void *data) {
  const size_t left = ((size_t*)data)[0], right = ((size_t*)data)[1];
  g_assert_cmpuint(left, <, right);
  g_assert_cmpuint(right, <=, BUF_SIZE);
  from_start(rbuf, &right);
  g_assert_cmpuint(rbuf_get(*rbuf, DST, left), ==, left);
  g_assert_cmpuint(rbuf_can_get(*rbuf), ==, right - left);
}

void outside(RingBuf **rbuf, const void *data) {
  const size_t left = ((size_t*)data)[0], right = ((size_t*)data)[1];
  const size_t DATA_SIZE = BUF_SIZE - (right - left);
  g_assert_cmpuint(left, <, right);
  g_assert_cmpuint(right, <=, BUF_SIZE);
  empty(rbuf, &right);
  g_assert_cmpuint(rbuf_put(*rbuf, DST, DATA_SIZE), ==, DATA_SIZE);
  g_assert_cmpuint(rbuf_can_get(*rbuf), ==, DATA_SIZE);
}

void teardown(RingBuf **rbuf, const void *data) {
  rbuf_free(*rbuf);
}

static void put_get(RingBuf **rbuf, const void *data) {
  const size_t BUF_LEN = rbuf_can_get(*rbuf);
  const size_t DATA_SIZE = BUF_SIZE - BUF_LEN;
  g_assert_cmpuint(DATA_SIZE, >=, 0);
  g_assert_cmpuint(rbuf_put(*rbuf, SRC, DATA_SIZE), ==, DATA_SIZE);
  g_assert_cmpuint(rbuf_can_get(*rbuf), ==, BUF_SIZE);
  g_assert_cmpuint(rbuf_get(*rbuf, DST, BUF_SIZE), ==, BUF_SIZE);
  assert_cmpstrn(SRC, ==, DST + BUF_LEN, DATA_SIZE);
}

static void overrun(RingBuf **rbuf, const void *data) {
  const size_t BUF_LEN = rbuf_can_get(*rbuf);
  const size_t DATA_SIZE = BUF_SIZE - BUF_LEN;
  g_assert_cmpuint(DATA_SIZE, >=, 0);
  g_assert_cmpuint(rbuf_put(*rbuf, SRC, DATA_SIZE * 2), ==, DATA_SIZE);
  g_assert_cmpuint(rbuf_can_get(*rbuf), ==, BUF_SIZE);
  g_assert_cmpuint(rbuf_get(*rbuf, DST, BUF_SIZE), ==, BUF_SIZE);
  assert_cmpstrn(SRC, ==, DST + BUF_LEN, DATA_SIZE);
}

static void underrun(RingBuf **rbuf, const void *data) {
  const size_t BUF_LEN = rbuf_can_get(*rbuf);
  const size_t DATA_SIZE = (BUF_SIZE - BUF_LEN) / 2;
  g_assert_cmpuint(DATA_SIZE, >=, 0);
  g_assert_cmpuint(rbuf_put(*rbuf, SRC, DATA_SIZE), ==, DATA_SIZE);
  g_assert_cmpuint(rbuf_can_get(*rbuf), ==, BUF_LEN + DATA_SIZE);
  g_assert_cmpuint(rbuf_get(*rbuf, DST, BUF_SIZE), ==, BUF_LEN + DATA_SIZE);
  assert_cmpstrn(SRC, ==, DST + BUF_LEN, DATA_SIZE);
}

int main(int argc, char *argv[]) {
  g_test_init(&argc, &argv, NULL);
  g_test_add("/untouched/put_get", RingBuf*, NULL,
             untouched, put_get, teardown);
  g_test_add("/full/put_get", RingBuf*, &BUF_SIZE,
             from_start, put_get, teardown);
  g_test_add("/empty/put_get", RingBuf*, &PART_SIZE,
             empty, put_get, teardown);
  g_test_add("/from_start/put_get", RingBuf*, &PART_SIZE,
             from_start, put_get, teardown);
  g_test_add("/to_end/put_get", RingBuf*, &PART_SIZE,
             to_end, put_get, teardown);
  g_test_add("/inside/put_get", RingBuf*, &BOUNDS,
             inside, put_get, teardown);
  g_test_add("/outside/put_get", RingBuf*, &BOUNDS,
             outside, put_get, teardown);

  g_test_add("/untouched/overrun", RingBuf*, NULL,
             untouched, overrun, teardown);
  g_test_add("/full/overrun", RingBuf*, &BUF_SIZE,
             from_start, overrun, teardown);
  g_test_add("/empty/overrun", RingBuf*, &PART_SIZE,
             empty, overrun, teardown);
  g_test_add("/from_start/overrun", RingBuf*, &PART_SIZE,
             from_start, overrun, teardown);
  g_test_add("/to_end/overrun", RingBuf*, &PART_SIZE,
             to_end, overrun, teardown);
  g_test_add("/inside/overrun", RingBuf*, &BOUNDS,
             inside, overrun, teardown);
  g_test_add("/outside/overrun", RingBuf*, &BOUNDS,
             outside, overrun, teardown);

  g_test_add("/untouched/underrun", RingBuf*, NULL,
             untouched, underrun, teardown);
  g_test_add("/full/underrun", RingBuf*, &BUF_SIZE,
             from_start, underrun, teardown);
  g_test_add("/empty/underrun", RingBuf*, &PART_SIZE,
             empty, underrun, teardown);
  g_test_add("/from_start/underrun", RingBuf*, &PART_SIZE,
             from_start, underrun, teardown);
  g_test_add("/to_end/underrun", RingBuf*, &PART_SIZE,
             to_end, underrun, teardown);
  g_test_add("/inside/underrun", RingBuf*, &BOUNDS,
             inside, underrun, teardown);
  g_test_add("/outside/underrun", RingBuf*, &BOUNDS,
             outside, underrun, teardown);

  return g_test_run();
}
