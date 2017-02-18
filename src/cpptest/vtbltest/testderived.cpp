
#include "testbase.h"
#include <stdio.h>

class CTestDerived : public CTestBase {
  void DoSub();
 public:
  CTestDerived() { SetIns(this); }
} gTestDerived;

void CTestDerived::DoSub() {
  printf("derived DoSub called\n");
}
