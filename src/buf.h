#pragma once

/*
  Assume that this structure is pointing at r/w data, that it doesn't own.
 */
typedef struct {
  char *ptr;
  ptrdiff_t len;
  ptrdiff_t cap;
} buf_t;

#define bufFromPtr(ptr, sz)	      (buf_t){ptr, 0, sz}
#define bufFromC(arr)		      (buf_t){arr, 0, sizeof(arr)}

/*
  Basically just sets len to 0.
 */
void bufClear(buf_t *buf);

/*
  Shifts sz bytes off the front of buf.

  If before the buffer looks like...
  
    { [A,B,C,D,_], 4, 5 }

  ...and you call bufDrop(&b, 2), after it will look like...

    { [C,D,_,_,_], 2, 5 }
  
*/
void bufDropBytes(buf_t *buf, size sz);
void bufDropTo(buf_t *buf, char *ptr);

/*
  Returns str w/ any data not appended, so if the returned str has
  len == 0, you know the full input is in buf.

  The second version is utf8 aware and won't copy a partial character
  into buf.
*/
bytes_t bufAppendBytes(buf_t *buf, bytes_t str); // TODO: move to bytes 
str_t bufAppendStr(buf_t *buf, str_t str);

/*
  Basic arena allocation on top of a buffer (simple as in--once you
  run out of memory, you're done!).
 */
#define bufAlloc(buf, t) bufAllocN(buf, t, 1)
#define bufAllocN(buf, t, n) ((t*)bufAllocCount((buf), sizeof(t), _Alignof(t), (n)))

/* aborts if no memory */
void *bufAllocCount(buf_t *buf, size sz, size align, size count);

/* when not enough memory, this returns false and out is untouched */
bool bufAllocTest(buf_t *buf, void **out, size sz, size align, size count);
