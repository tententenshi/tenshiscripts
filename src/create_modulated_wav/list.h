/*==============================================================
 *
 * list.h	DSP List Manager
 *
 *==============================================================
 */
#ifndef _list_h_
#define _list_h_

template <class TDATA>
class CList {
  protected:
	struct Node {
		Node*   prev;
		Node*   next;
		TDATA   data;
	} mEndNode;

  public:
	struct Iterator {
		Node* mNode;
		Iterator(Node* theNode) : mNode(theNode) {}
		bool operator==(const Iterator& x) const { return mNode == x.mNode; }
		bool operator!=(const Iterator& x) const { return mNode != x.mNode; }
		TDATA& operator*() const { return mNode->data; }
		TDATA* operator->() const { return &(mNode->data); }
		Iterator& operator++() { mNode = mNode->next; return *this; }
		Iterator operator++(int) { Iterator aIter = *this; ++*this; return aIter; }
		Iterator& operator--() { mNode = mNode->prev; return *this; }
		Iterator operator--(int) { Iterator aIter = *this; --*this; return aIter; }
	};

	CList() {
		mEndNode.prev = &mEndNode;
		mEndNode.next = &mEndNode;
	}
	Iterator push_back(const TDATA& thePrm) {
		Node* tmpNode = new Node;
		tmpNode->next = &mEndNode;
		tmpNode->prev = mEndNode.prev;
		tmpNode->data = thePrm;
		(mEndNode.prev)->next = tmpNode;
		mEndNode.prev = tmpNode;
		return Iterator(tmpNode);
	}
	Iterator erase(Iterator theNode) {
		if (theNode.mNode == &mEndNode) return &mEndNode;	// improper argument

		(theNode.mNode->prev)->next = theNode.mNode->next;
		(theNode.mNode->next)->prev = theNode.mNode->prev;
		Node* aNextNode = theNode.mNode->next;
		delete theNode.mNode;
		return Iterator(aNextNode);
	}
	bool Empty() const { return mEndNode.next == &mEndNode; }
	Iterator begin() { return Iterator(mEndNode.next); }
	Iterator end() { return Iterator(&mEndNode); }
};

#endif	/* !_list_h_ */

