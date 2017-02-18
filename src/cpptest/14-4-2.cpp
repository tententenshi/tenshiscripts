// 要素をリストの前方からでも後方からでも追加することができる
#include <iostream>
#include <list>
using namespace std;

int main()
{
	list<char> lst;
	list<char> revlst;
	int i;
	
	for (i = 0; i < 10; i++) lst.push_back('A' + i);
	
	cout << "lst の大きさ = " << lst.size() << endl;
	cout << "もとの内容: ";

	list<char>::iterator p;

	// lst から要素を削除し、その要素を
	// revlst に逆順に追加する
	while (!lst.empty()) {
		p = lst.begin();
		cout << *p;
		lst.pop_front();
		revlst.push_front(*p);
	}
	cout << endl << endl;

	cout << "revlst の大きさ = ";
	cout << revlst.size() << endl;
	cout << "逆順の内容: ";
	p = revlst.begin();
	while (p != revlst.end()) {
		cout << *p;
		p++;
	}
	
	return 0;
}
