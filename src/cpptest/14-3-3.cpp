// insert()�ؿ��� erase()�ؿ��λ�����
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v(5, 1);	// Ĺ������Υ٥��ȥ���������
	int i;
	
	// �٥��ȥ�κǽ�����Ƥ�ɽ������
	cout << "������ = " << v.size() << endl;
	cout << "���ߤ�����:\n";
	for (i = 0; i < v.size(); i++) cout << v[i] << " ";
	cout << endl;

	// ȿ���Ҥ���Ѥ��ƥ�����������
	vector<int>::iterator p = v.begin();
	p += 2;	// 3���ܤ����Ǥ�ؤ�

	// ��9�����10�Ĥ����Ǥ���������
	v.insert(p, 10, 9);

	// ������Υ٥��ȥ�����Ƥ�ɽ������
	cout << "������Υ����� = " << v.size() << endl;
	cout << "�����������:\n";
	for (i = 0; i < v.size(); i++) cout << v[i] << " ";
	cout << endl << endl;

	// ���������Ǥ�������
	p = v.begin();
	p += 4;	// 3���ܤ����Ǥ�ؤ�
	v.erase(p, p+10);	// ����10�Ĥ����Ǥ�������

	// �����Υ٥��ȥ�����Ƥ�ɽ������
	cout << "�����Υ����� = " << v.size() << endl;
	cout << "����������:\n";
	for (i = 0; i < v.size(); i++) cout << v[i] << " ";
	cout << endl;

	return 0;
}
