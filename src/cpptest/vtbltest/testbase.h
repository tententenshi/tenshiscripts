
#include <stdio.h>

class CTestBase {
  static CTestBase* gpInstance;
  virtual void DoSub();

 public:
  void SetIns(CTestBase* theIns) { gpInstance = theIns; }
  static void Do() {
    if (gpInstance) {
      gpInstance->DoSub();
    } else {
      printf("derived not set\n");
    }
  }
};

