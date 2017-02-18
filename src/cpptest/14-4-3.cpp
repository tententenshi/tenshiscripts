// リストをソートする
#include <iostream>
#include <list>
#include <cstdlib>
using namespace std;

int main()
{
	list<char> lst;
	int i;

	// 無作為の文字を含むリストを作成する
	for (i = 0; i < 10; i++) lst.push_back('A' + (rand() % 26));
	
	cout << "もとの内容: ";
	list<char>::iterator p = lst.begin();

	while (p != lst.end()) {
		cout << *p;
		p++;
	}
	cout << endl << endl;

	// リストをソートする
	lst.sort();

	cout << "ソートした内容: ";
	p = lst.begin();
	while (p != lst.end()) {
		cout << *p;
		p++;
	}
	cout << endl << endl;

	return 0;
}
