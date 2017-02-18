// 反復子を使用してベクトルにアクセスする
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v;	// 長さゼロのベクトルを作成する
	int i;
	
	// ベクトルに値を追加する
	for (i = 0; i < 10; i++) v.push_back(i);

	// 添え字を使用してベクトルの内容にアクセスできる
	for (i = 0; i < v.size(); i++) cout << v[i] << " ";
	cout << endl;

	// 反復子を使用してアクセスする
	vector<int>::iterator p = v.begin();
	while(p != v.end()) {
		cout << *p << " ";
		p++;
	}

	return 0;
}
