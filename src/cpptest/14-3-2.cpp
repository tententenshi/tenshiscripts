// ȿ���Ҥ���Ѥ��ƥ٥��ȥ�˥�����������
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v;	// Ĺ������Υ٥��ȥ���������
	int i;
	
	// �٥��ȥ���ͤ��ɲä���
	for (i = 0; i < 10; i++) v.push_back(i);

	// ź��������Ѥ��ƥ٥��ȥ�����Ƥ˥��������Ǥ���
	for (i = 0; i < v.size(); i++) cout << v[i] << " ";
	cout << endl;

	// ȿ���Ҥ���Ѥ��ƥ�����������
	vector<int>::iterator p = v.begin();
	while(p != v.end()) {
		cout << *p << " ";
		p++;
	}

	return 0;
}
