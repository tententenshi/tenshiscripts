#include <stdio.h>
#include <memory>

class CTest {
public:
	class CImpl {
	public:
		void Func1();
		void Func2();
		

		struct SHdl {
			int mem1;
			void (CTest::CImpl::*pFunc)();
		};

		SHdl hdlTbl[3];

		void Set();
		void Call();

	};


	std::auto_ptr<CImpl> pImpl;
	CTest() : pImpl(new CImpl) {}

	void Set() { pImpl->Set(); }
	void Call() { pImpl->Call(); }
};


void CTest::CImpl::Func1()
{
	printf("Func1 Called\n");
}

void CTest::CImpl::Func2()
{
	printf("Func2 Called\n");
}

void CTest::CImpl::Set()
{
	hdlTbl[2].pFunc = &CTest::CImpl::Func1;
}

void CTest::CImpl::Call()
{
	(this->*hdlTbl[2].pFunc)();
}



int main() {
	CTest instance;

	instance.Set();
	instance.Call();
}
