// 2つのリストを統合する
#include <iostream>
#include <list>
using namespace std;

int main()
{
	list<char> lst1, lst2;
	int i;

	for (i = 0; i < 10; i += 2) lst1.push_back('A' + i);
	for (i = 1; i < 11; i += 2) lst2.push_back('A' + i);

	cout << "lst1の内容: ";
	list<char>::iterator p = lst1.begin();
	while (p != lst1.end()) {
		cout << *p;
		p++;
	}
	cout << endl << endl;
	cout << "lst2の内容: ";
	p = lst2.begin();
	while (p != lst2.end()) {
		cout << *p;
		p++;
	}
	cout << endl << endl;

	// 2つのリストを統合する
	lst1.merge(lst2);
	if (lst2.empty()) cout << "lst2 は現在、空\n";
	cout << "統合後の lst1 の内容:\n";
	p = lst1.begin();
	while (p != lst1.end()) {
		cout << *p;
		p++;
	}
	cout << endl << endl;

	return 0;
}
