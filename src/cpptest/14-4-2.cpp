// ���Ǥ�ꥹ�Ȥ���������Ǥ��������Ǥ��ɲä��뤳�Ȥ��Ǥ���
#include <iostream>
#include <list>
using namespace std;

int main()
{
	list<char> lst;
	list<char> revlst;
	int i;
	
	for (i = 0; i < 10; i++) lst.push_back('A' + i);
	
	cout << "lst ���礭�� = " << lst.size() << endl;
	cout << "��Ȥ�����: ";

	list<char>::iterator p;

	// lst �������Ǥ��������������Ǥ�
	// revlst �˵ս���ɲä���
	while (!lst.empty()) {
		p = lst.begin();
		cout << *p;
		lst.pop_front();
		revlst.push_front(*p);
	}
	cout << endl << endl;

	cout << "revlst ���礭�� = ";
	cout << revlst.size() << endl;
	cout << "�ս������: ";
	p = revlst.begin();
	while (p != revlst.end()) {
		cout << *p;
		p++;
	}
	
	return 0;
}
