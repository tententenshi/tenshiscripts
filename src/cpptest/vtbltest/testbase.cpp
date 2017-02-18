#include "testbase.h"
#include <stdio.h>

CTestBase* CTestBase::gpInstance = 0;

void CTestBase::DoSub() {
  printf("testbase DoSub called\n");
}

