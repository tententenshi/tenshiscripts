// �ꥹ�Ȥ򥽡��Ȥ���
#include <iostream>
#include <list>
#include <cstdlib>
using namespace std;

int main()
{
	list<char> lst;
	int i;

	// ̵��٤�ʸ����ޤ�ꥹ�Ȥ��������
	for (i = 0; i < 10; i++) lst.push_back('A' + (rand() % 26));
	
	cout << "��Ȥ�����: ";
	list<char>::iterator p = lst.begin();

	while (p != lst.end()) {
		cout << *p;
		p++;
	}
	cout << endl << endl;

	// �ꥹ�Ȥ򥽡��Ȥ���
	lst.sort();

	cout << "�����Ȥ�������: ";
	p = lst.begin();
	while (p != lst.end()) {
		cout << *p;
		p++;
	}
	cout << endl << endl;

	return 0;
}
