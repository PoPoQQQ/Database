#include <iostream>
#include "./Utils/BplusTree.h"
using namespace std;

int main(){
    BplusTree<int, int> tree(5);
    for(int i = 0; i <= 20; ++i){
        tree.insert(i, i * 1031);
    }
    BplusNode<int, int>::debug_print();
    tree.remove(1);
    tree.remove(2);
    tree.remove(4);
    tree.remove(9);
    tree.remove(10);
    tree.update(7, 2333);
    tree.update(2, 66666);
    tree.remove(11);
    tree.insert(100, 100);
    tree.insert(1231432,123412341);
    tree.remove(100);
    tree.remove(5);
    cout << "-------------------------------------------" << endl;
    BplusLeaf<int, int> *tmp = tree.leafHead;
    while(tmp != NULL){
        tmp->print();
        tmp = tmp->next;
    }
    for(int i = 0;i <= 20; ++i){
        tree.remove(i);
    }
    cout << "-------------------------------------------" << endl;
    BplusNode<int, int>::debug_print();
    return 0;
}