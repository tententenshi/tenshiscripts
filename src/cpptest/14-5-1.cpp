// ñ��ʥޥåפλ�����
#include <iostream>
#include <map>
using namespace std;

int main()
{
	map<char, int> m;
	int i;

	// �ޥåפ˥��� / �ͤ��Ǽ����
	for (i = 0; i < 10; i++) {
		m.insert(pair<char, int>('A' + i, i));
	}

	char ch;
	cout << "����������: ";
	cin >> ch;

	map<char, int>::iterator p;

	// ����Υ������ͤ�õ��
	p = m.find(ch);
	if (p != m.end()) cout << p->second;
	else cout << "�����ϥޥåפˤʤ�.\n";

	cout << endl;

	return 0;
}
