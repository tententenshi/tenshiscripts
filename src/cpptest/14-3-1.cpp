// �٥��ȥ�δ������
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v;	// Ĺ������Υ٥��ȥ���������
	int i;
	
	// v�κǽ�Υ�������ɽ������
	cout << "������ = " << v.size() << endl;
	
	// �٥��ȥ���������ͤ��ɲä���
	for (i = 0; i < 10; i++) v.push_back(i);
	
	// v�θ��ߤΥ�������ɽ������
	cout << "���ߤΥ����� = " << v.size() << endl;
	
	// �٥��ȥ�����Ƥ�ɽ������
	cout << "���ߤ�����:\n";
	for (i = 0; i < v.size(); i++) cout << v[i] << " ";
	cout << endl;
	
	// �٥��ȥ�������ˤ�����ͤ��ɲä���
	for (i = 0; i < 10; i++) v.push_back(i + 10);
	
	// v�θ��ߤΥ�������ɽ������
	cout << "���ߤΥ����� = " << v.size() << endl;
	
	// �٥��ȥ�����Ƥ�ɽ������
	cout << "���ߤ�����:\n";
	for (i = 0; i < v.size(); i++) cout << v[i] << " ";
	cout << endl;
	
	// �٥��ȥ�����Ƥ��ѹ�����
	for (i = 0; i < v.size(); i++) v[i] = v[i] + v[i];
	
	// �٥��ȥ�����Ƥ�ɽ������
	cout << "���ߤ�����:\n";
	for (i = 0; i < v.size(); i++) cout << v[i] << " ";
	cout << endl;

	return 0;
}
