// 単純なマップの使用例
#include <iostream>
#include <map>
using namespace std;

int main()
{
	map<char, int> m;
	int i;

	// マップにキー / 値を格納する
	for (i = 0; i < 10; i++) {
		m.insert(pair<char, int>('A' + i, i));
	}

	char ch;
	cout << "キーを入力: ";
	cin >> ch;

	map<char, int>::iterator p;

	// 指定のキーの値を探す
	p = m.find(ch);
	if (p != m.end()) cout << p->second;
	else cout << "キーはマップにない.\n";

	cout << endl;

	return 0;
}
