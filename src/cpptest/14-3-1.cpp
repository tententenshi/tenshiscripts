// ベクトルの基本操作
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v;	// 長さゼロのベクトルを作成する
	int i;
	
	// vの最初のサイズを表示する
	cout << "サイズ = " << v.size() << endl;
	
	// ベクトルの末尾に値を追加する
	for (i = 0; i < 10; i++) v.push_back(i);
	
	// vの現在のサイズを表示する
	cout << "現在のサイズ = " << v.size() << endl;
	
	// ベクトルの内容を表示する
	cout << "現在の内容:\n";
	for (i = 0; i < v.size(); i++) cout << v[i] << " ";
	cout << endl;
	
	// ベクトルの末尾にさらに値を追加する
	for (i = 0; i < 10; i++) v.push_back(i + 10);
	
	// vの現在のサイズを表示する
	cout << "現在のサイズ = " << v.size() << endl;
	
	// ベクトルの内容を表示する
	cout << "現在の内容:\n";
	for (i = 0; i < v.size(); i++) cout << v[i] << " ";
	cout << endl;
	
	// ベクトルの内容を変更する
	for (i = 0; i < v.size(); i++) v[i] = v[i] + v[i];
	
	// ベクトルの内容を表示する
	cout << "現在の内容:\n";
	for (i = 0; i < v.size(); i++) cout << v[i] << " ";
	cout << endl;

	return 0;
}
