// �ꥹ�Ȥδ������
#include <iostream>
#include <list>
using namespace std;

int main()
{
	list<char> lst;	// ���Υꥹ�Ȥ��������
	int i;
	
	for (i = 0; i < 10; i++) lst.push_back('A' + i);
	
	cout << "�礭�� = " << lst.size() << endl;
	
	list<char>::iterator p;
	
	cout << "����: ";
	while(!lst.empty()) {
		p = lst.begin();
		cout << *p;
		lst.pop_front();
	}
	
	return 0;
}
