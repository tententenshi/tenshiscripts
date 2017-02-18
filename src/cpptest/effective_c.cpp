#include <iostream>
#include <string>
using namespace std;

class B;

class A {
public:
	A (const B&) : name("A") {}
	const string& get() const { return name; }
private:
	string name;
};

class B {
public:
	B() : name("B") { a = new A(*this); }
	~B() { delete a; }
	operator A() const { return *a; }
	const string& get() const { return name; }
private:
	A* a;
	string name;
};

void f(const A& a) {
	cout << a.get() << endl;
}

int main() {
	B b;
	f(b);
}
