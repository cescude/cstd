#include "std.h"

static print_t tout = printerUnbuffered(2);

void testFail(test_t *t, str_t desc) {
  t->failed++;
  printStr(tout, strC("FAILED: Case #"));
  printU64(tout, t->failed + t->passed);
  if (strNonEmpty(desc)) {
    printStr(tout, strC(", "));
    printStr(tout, desc);
  }
  printStr(tout, strC("\n"));
}

void testPass(test_t *t) {
  t->passed++;
}

size testRunner(test_defn_t *tests, size num_tests, bool verbose) {
  size total_passed = 0;
  size total_failed = 0;
  
  for (size i=0; i<num_tests; i++) {
    test_t t = {0};

    if (verbose) {
      printStr(tout, strC("["));
      printU64F(tout, i+1, (format_t){.right = 1, .width=4});
      printStr(tout, strC("] "));
      printStr(tout, strFromC(tests[i].name));
      printStr(tout, strC("\n"));
    }
    
    tests[i].fn(&t);

    if (verbose) {
      printStr(tout, strC("    passed="));
      printU64(tout, t.passed);
      printStr(tout, strC(" failed="));
      printU64(tout, t.failed);
      printStr(tout, strC("\n"));
    }

    total_passed += t.passed;
    total_failed += t.failed;
  }

  if (verbose) {
    printStr(tout, strC("\n"));
  }

  printStr(tout, strC("Results: passed="));
  printU64(tout, total_passed);
  printStr(tout, strC(" failed="));
  printU64(tout, total_failed);
  printStr(tout, strC("\n"));

  return total_failed;
}
