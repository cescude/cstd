#pragma once

typedef struct {
  size passed, failed;
} test_t;

typedef struct {
  void (*fn)(test_t *);
  char *name;
} test_defn_t;

#define assertTrue(t, bln, desc) if (bln) testPass(t); else testFail(t, desc)
#define assertFalse(t, bln, desc) if (bln) testFail(t, desc); else testPass(t)

void testFail(test_t *t, str_t desc);
void testPass(test_t *t);

size testRunner(test_defn_t *tests, size num_tests, bool verbose);
