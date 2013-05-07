#include <iostream>
#include <typeinfo>
#include <cstdlib>
using namespace std;

class B {
public:
//	virtual ~B() {}
};
class D : public B { };

int main(void)
{
    D* pobj1 = new D();
    B* pobj2 = pobj1;

    if (typeid(*pobj1) == typeid(*pobj2)) // will be false
        cout << "same" << endl;
    else
        cout << "different" << endl;

    cout << typeid(*pobj1).name() << endl;
    cout << typeid(*pobj2).name() << endl;
    cout << typeid(pobj1).name() << endl;

    delete pobj1;

    return 0;
}

