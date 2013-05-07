#include <iostream>
#include <cstdlib>
#include <typeinfo>
using namespace std;

class Shape {
public:
	virtual void example() = 0;
};

class Rectangle : public Shape {
public:
	void example() {
		cout << "*****\n*   *\n*   *\n*****\n";
	}
};

class Triangle : public Shape {
public:
	void example() {
		cout << "*\n* *\n*  *\n*****\n";
	}
};

class Line : public Shape {
public:
	void example() {
		cout << "*****\n";
	}
};

class NullShape : public Shape {
public:
	void example() {
		cout << "null shape is selected!";
	}
};

Shape *generator()
{
	switch(rand() % 4) {
	case 0:
		return new Line;
	case 1:
		return new Rectangle;
	case 2:
		return new Triangle;
	case 3:
		return new NullShape;
	}
	return NULL;
}

int main()
{
	for (int i = 0; i < 10; i++) {
		Shape* p = generator();

		cout << typeid(*p).name() << endl;

		if (typeid(*p) != typeid(NullShape)) {
			p->example();
		}
	}

	return 0;
}
