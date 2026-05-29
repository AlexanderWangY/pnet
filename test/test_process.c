#include "process.h"
#include "unity.h"
#include "unity_internals.h"

void setUp(){};
void tearDown(){};

void test_resolve_add() {
  int res = add(10, 20);
  TEST_ASSERT_EQUAL(30, res);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_resolve_add);
  return UNITY_END();
}
