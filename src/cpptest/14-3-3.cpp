// insert()関数と erase()関数の使用例
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v(5, 1);	// 長さゼロのベクトルを作成する
	int i;
	
	// ベクトルの最初の内容を表示する
	cout << "サイズ = " << v.size() << endl;
	cout << "現在の内容:\n";
	for (i = 0; i < v.size(); i++) cout << v[i] << " ";
	cout << endl;

	// 反復子を使用してアクセスする
	vector<int>::iterator p = v.begin();
	p += 2;	// 3つ目の要素を指す

	// 値9を持つ10個の要素を挿入する
	v.insert(p, 10, 9);

	// 挿入後のベクトルの内容を表示する
	cout << "挿入後のサイズ = " << v.size() << endl;
	cout << "挿入後の内容:\n";
	for (i = 0; i < v.size(); i++) cout << v[i] << " ";
	cout << endl << endl;

	// これらの要素を削除する
	p = v.begin();
	p += 4;	// 3つ目の要素を指す
	v.erase(p, p+10);	// 次の10個の要素を削除する

	// 削除後のベクトルの内容を表示する
	cout << "削除後のサイズ = " << v.size() << endl;
	cout << "削除後の内容:\n";
	for (i = 0; i < v.size(); i++) cout << v[i] << " ";
	cout << endl;

	return 0;
}
