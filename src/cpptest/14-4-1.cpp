// リストの基本操作
#include <iostream>
#include <list>
using namespace std;

int main()
{
	list<char> lst;	// 空のリストを作成する
	int i;
	
	for (i = 0; i < 10; i++) lst.push_back('A' + i);
	
	cout << "大きさ = " << lst.size() << endl;
	
	list<char>::iterator p;
	
	cout << "内容: ";
	while(!lst.empty()) {
		p = lst.begin();
		cout << *p;
		lst.pop_front();
	}
	
	return 0;
}
