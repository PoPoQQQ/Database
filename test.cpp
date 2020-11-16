#include <iostream>
#include <cstring>
#include <vector>
#include <assert.h>
using namespace std;

template <typename K, typename T>
struct BplusLeaf;

template <typename K, typename T>
struct BplusTree;

template <typename K, typename T>
struct BplusNode {
    BplusNode(int maxSize, BplusNode<K, T>* parent, BplusTree<K, T>* tree) : maxSize(maxSize), parent(parent), tree(tree) {
        this->no = ++ BplusNode::totalNum;
        BplusNode<K, T>::debug.push_back(this);
    }

    ~BplusNode(){
        this->key.clear();
        if(this->released)
            this->children.clear();
        else {
            for(int i = 0; i < this->children.size(); ++i) {
                delete this->children[i];
            }
            this->children.clear();
        }
    }
    /**
     * 递归查询键值
     * 键值必须要实现 < 与 == 比较符
     * */
    T* find(const K& key) const {
        const int size = this->key.size();
        if(this->key.size() == 0){ return NULL; }
        
        for(int i = 0;i < this->key.size(); ++i) {
            if(key < this->key[i]) {
                return this->children[i]->find(key);
            } else if(i == size - 1) {
                return this->children[i]->find(key);
            }
        }
    }

    /**
     * 不直接处理插入操作
     * */
    void insert(const K& key, const T& value) {
        return;
    }

    /** 
     * 删除处理
     * */
    void remove(const K& key) {
        return;
    }

    void print() {
        if(this->isLeaf){
            static_cast<BplusLeaf<K, T>*>(this)->print();
            return;
        }
        printf("----------------BplusNode(%d)----------\n", this->no); 
        cout << "[";
        for(int i = 0;i < this->size(); ++i){
            cout << key[i] << ",\t";
        }
        cout << "]" << endl;
        cout << endl;
        for(int i = 0;i < this->children.size(); ++i) {
            cout << "BplusNode(" << children[i]->no << ")\t";
        }
        cout << endl;
    }

    int size() {
        return this->key.size();
    }

    void addChild(const K& key, BplusNode* child) {
        // 直接插入
        int i = 0;
        for(i = 0; i < this->size(); ++i) {
            if(key < this->key[i]) {
                break;
            }
        }
        this->key.insert(this->key.begin() + i, key);
        this->children.insert(this->children.begin() + i + 1, child);
        if(this->size() >= this->maxSize) {
            int pos = this->maxSize / 2;
            // 分裂出新的内部节点
            if(this->parent == NULL) {
                BplusNode<K, T>* father = new BplusNode<K, T>(this->maxSize, NULL, this->tree);
                this->parent = father;
                father->children.push_back(this);
                this->tree->root = father;
            }
            BplusNode<K, T>* sibling = new BplusNode<K, T>(this->maxSize, this->parent, this->tree);
            // 分配内容
            const K splitKey = this->key[pos];
            sibling->key.insert(sibling->key.begin(), this->key.begin() + pos + 1, this->key.end());
            sibling->children.insert(sibling->children.begin(), this->children.begin() + pos + 1, this->children.end());
            for(auto it = sibling->children.begin(); it != sibling->children.end(); ++it) {
                (*it)->parent = sibling;
            }
            
            this->key.erase(this->key.begin() + pos, this->key.end());
            this->children.erase(this->children.begin() + pos + 1, this->children.end());
            // 处理父子关系
            this->parent->addChild(splitKey, sibling);
        }
    }
    void updateKey(const K& key, BplusNode<K, T>* child) {
        if(child == this->children[0]) {
            // 不更新第一个 child，认为每一个 child 都只对应右侧的节点
            return;
        }
        for(int i = 0;i < this->children.size(); ++i) {
            if(this->children[i] == child) {
                this->key[i - 1] = key;
                return;
            }
        }
        // 没有对应的子节点，不做处理
    }
    BplusNode* findLeftSibling(BplusNode* current) {
        for(int i = 0;i < this->children.size(); i++) {
            if(this->children[i] == current) {
                return i - 1 < 0 ? NULL : this->children[i - 1];
            }
        }
        return NULL;
    }
    BplusNode* findRightSibling(BplusNode* current) {
        for(int i = 0;i < this->children.size(); i++) {
            if(this->children[i] == current) {
                return i + 1 < this->children.size() ? this->children[i + 1] : NULL;
            }
        }
        return NULL;
    }

    /**
     * 删除两个子节点之间的父节点的 key 值
     * */
    void removeBetween(BplusNode* left, BplusNode* right) {
        for(int i = 0;i < this->children.size();i++) {
            if(this->children[i] == left && this->children[i + 1] == right) {
                // 认为每一个 key[i] 对应 children[i + 1]
                // 每次删除删除 key[i] 与 children[i]，合并后的节点应该位于 children[i + 1] 处
                this->key.erase(this->key.begin() + i);
                this->children.erase(this->children.begin() + i);
                break;
            }
        }
        if(this->size() < this->maxSize / 2) {
            this->handleRemove();
        }
    }

    void handleRemove() {
        const int minSize = this->maxSize / 2;
        if(this->parent == NULL) {
            if(this->children.size() == 1){
                // 说明在一切更新正确的前提下，出现这种情况说明根节点需要变成叶节点
                // 然后释放该节点
                this->tree->root = this->children[0];
                this->children[0]->parent = NULL;
                this->released = true;
                BplusNode<K, T>::debug[this->no - 1] = NULL;
                delete this;
                return;
            } else if(this->children.size() == 0) {
                cerr << "ERROR: node children number is 0 in handleRemove" << endl;
            }
        } else {
            //* 一个内部如果有父节点则说明它至少有一个兄弟节点
            BplusNode<K, T>* sibling = this->parent->findLeftSibling(this);
            if(sibling != nullptr) {
                const int index = this->parent->getChildIndex(this);
                // 如果内部节点有左兄弟，则做左合并
                if(sibling->size() > minSize) {
                    // 如果有富裕则让父节点下降，兄弟节点上升
                    // checked
                    this->key.insert(this->key.begin(), this->parent->key[index - 1]);
                    this->children.insert(this->children.begin(), sibling->children.back());
                    this->children.front()->parent = this;

                    this->parent->key[index - 1] = sibling->key.back();
                    sibling->key.pop_back();
                    sibling->children.pop_back();
                } else {
                    // 如果没有富裕则让父节点下降，兄弟节点合并
                    this->key.insert(this->key.begin(), this->parent->key[index - 1]);
                    this->key.insert(this->key.begin(), sibling->key.begin(), sibling->key.end());
                    this->children.insert(this->children.begin(), sibling->children.begin(), sibling->children.end());
                    for(auto it = sibling->children.begin(); it != sibling->children.end(); ++it){
                        (*it)->parent = this;
                    }
                    sibling->released = true;
                    BplusNode<K, T>::debug[sibling->no - 1] = NULL;
                    this->parent->removeBetween(sibling, this);
                    delete sibling;
                }
            } else {
                // 如果没有左兄弟，则做右合并
                // 没有左兄弟说明 this 的 children 的下标为 0
                sibling = this->parent->children[1];
                if(sibling->size() > minSize) {
                    // 如果有富裕则让父节点下降，兄弟节点上升
                    // checked
                    this->key.push_back(this->parent->key[0]); //父节点下降
                    this->children.push_back(sibling->children[0]); // 父节点对应的指针
                    this->children.back()->parent = this;

                    this->parent->key[0] = sibling->key[0]; // 兄弟节点上升
                    sibling->key.erase(sibling->key.begin());
                    sibling->children.erase(sibling->children.begin());
                } else {
                    // 否则让父节点下降，兄弟节点合并
                    // 将 this 合并到右侧的 sibling 中
                    sibling->key.insert(sibling->key.begin(), this->parent->key[0]);
                    sibling->key.insert(sibling->key.begin(), this->key.begin(), this->key.end());
                    sibling->children.insert(sibling->children.begin(), this->children.begin(), this->children.end());
                    for(auto it = this->children.begin(); it != this->children.end(); ++it) {
                        (*it)->parent = sibling;
                    }
                    this->released = true;
                    BplusNode<K, T>::debug[this->no - 1] = NULL;
                    this->parent->removeBetween(this, sibling);
                    delete this;
                }
            }
        }
    }
    int getChildIndex(const BplusNode<K, T>* node) {
        for(int i = 0;i < this->children.size();i++) {
            if(node == this->children[i]) {
                return i;
            }
        }
        return -1;
    }
    static void debug_print() {
        for(int i = 0;i < debug.size();i++) {
            if(debug[i] != NULL)
                debug[i]->print();
        }
    }
    vector<K> key;
    vector<BplusNode<K, T>*> children;
    BplusNode<K, T>* parent = NULL;
    bool isLeaf = false; // 用于判断是否为叶子节点
    int maxSize = 3;
public:
    static vector<BplusNode<K, T>*> debug;
    int no;
    static int totalNum;
    bool released = false; // 用于动态管理内存。代表节点中的 children 数据是否被移交、保存或使用某种方法使其可以被管理
    BplusTree<K, T>* tree;
};
template<typename K, typename T>
int BplusNode<K, T>::totalNum = 0;
template<typename K, typename T>
vector<BplusNode<K, T>*> BplusNode<K, T>::debug;

template <typename K, typename T>
struct BplusLeaf: public BplusNode<K, T> {
    BplusLeaf(int maxSize, BplusNode<K, T>* parent, BplusTree<K, T>* tree): BplusNode<K, T>(maxSize, parent, tree){
        this->isLeaf = true;
    }

    ~BplusLeaf() {
        this->data.clear();
    }

    T* find(const K& key) const {
        if(this->key.size() == 0) return NULL;

        for(int i = 0;i < this->key.size(); ++i) {
            if(key == this->key[i]) return &(this->data[i]);
        }
        return NULL;
    }

    void insert(const K& key, const T& value) {
        // 直接插入
        int i = 0;
        for(i = 0; i < this->size(); ++i) {
            if(key < this->key[i]) {
                break;
        cout << this->no << ":" << i << endl;
            }
        }
        this->key.insert(this->key.begin() + i, key);
        this->data.insert(this->data.begin() + i, value);
        // 如果达到最大的大小则进行分裂
        if(this->size() >= this->maxSize) {
            int pos = this->maxSize / 2;
            if(this->parent == NULL) {
                BplusNode<K, T>* father = new BplusNode<K, T>(this->maxSize, NULL, this->tree);
                father->children.push_back(this);
                this->parent = father;
                this->tree->root = father;
            }
            // 分裂出新的叶节点
            BplusLeaf<K, T>* splitLeaf = new BplusLeaf<K, T>(this->maxSize, this->parent, this->tree);
            const K splitKey = this->key[pos];
            // 更新链表
            splitLeaf->next = this->next;
            this->next = splitLeaf;
            // 分配内容
            splitLeaf->key.insert(splitLeaf->key.begin(), this->key.begin() + pos, this->key.end());
            splitLeaf->data.insert(splitLeaf->data.begin(), this->data.begin() + pos, this->data.end());
            this->key.erase(this->key.begin() + pos, this->key.end());
            this->data.erase(this->data.begin() + pos, this->data.end());
            // 处理父子关系
            this->parent->addChild(splitKey, splitLeaf);
        }
    }

    void remove(const K& key) {
        // 首先直接删除，不管长度
        int i = 0;
        for(i = 0;i < this->key.size(); ++i){
            if(this->key[i] == key) {
                break;
            }
        }
        if(i == this->key.size()) {
            // 没有查找到可以删除的键，删除失败
            return;
        } else {
            this->key.erase(this->key.begin() + i);
            this->data.erase(this->data.begin() + i);
        }

        // 处理节点的合并，永远合并左侧的子节点到右侧的节点中
        const int minSize = this->maxSize / 2;
        if(this->size() < minSize) {
            // 如果自己的关键字不足，则试图从左兄弟中获取一个关键字
            if(this->parent == NULL) { // 如果已经是根节点，说明没办法填充数据
                return; // 删除成功，但是长度不一定不小于 maxSize / 2
            }
            //* 一个叶节点如果有父节点则说明它至少有一个兄弟节点
            BplusLeaf<K, T>* sibling = static_cast<BplusLeaf<K, T>*>(this->parent->findLeftSibling(this));
            if(sibling != nullptr) {
                // 如果叶节点有左兄弟，则做左合并
                if(sibling->size() > minSize) {
                    // 如果有富裕则借用
                    const K borrowKey = *(sibling->key.erase(sibling->key.end() - 1));
                    const T borrowData = *(sibling->data.erase(sibling->data.end() - 1));
                    this->key.insert(this->key.begin(), borrowKey);
                    this->data.insert(this->data.begin(), borrowData);
                    // 并且更新父节点的相关信息
                    this->parent->updateKey(this->key[0], this);
                } else {
                    // 如果没有富裕则合并
                    this->key.insert(this->key.begin(), sibling->key.begin(), sibling->key.end());
                    this->data.insert(this->data.begin(), sibling->data.begin(), sibling->data.end());
                    this->parent->removeBetween(sibling, this);

                    BplusNode<K, T>::debug[sibling->no - 1] = NULL;
                    delete sibling;
                }
            } else {
                // 如果叶节点没有左兄弟，则做右合并
                sibling = static_cast<BplusLeaf<K, T>*>(this->parent->findRightSibling(this));
                if(sibling->size() > minSize) {
                    // 如果有富裕则借用
                    const K borrowKey = sibling->key.front();
                    const T borrowData = sibling->data.front();
                    sibling->key.erase(sibling->key.begin());
                    sibling->data.erase(sibling->data.begin());
                    this->key.insert(this->key.end(), borrowKey);
                    this->data.insert(this->data.end(), borrowData);
                    // 并且更新父节点的相关信息
                    this->parent->updateKey(sibling->key[0], sibling);
                } else {
                    // 否则合并
                    sibling->key.insert(sibling->key.begin(), this->key.begin(), this->key.end());
                    sibling->data.insert(sibling->data.begin(), this->data.begin(), this->data.end());
                    this->parent->removeBetween(this, sibling);
                    
                    BplusNode<K, T>::debug[this->no - 1] = NULL;
                    delete this;
                }
            }
        }
    }

    void update(const K& key, const T& value) {
        for(int i = 0; i < this->key.size(); ++i) {
            if(this->key[i] == key) {
                this->data[i] = value;
                return;
            }
        }
    }

    void print() {
        printf("----------Leaf Node(%d)--------\n", this->no);
        cout << "[";
        for(int i = 0;i < data.size(); ++i) {
            cout << "{"  << this->key[i] << ": " <<  data[i] << "}, ";
        }
        cout << "]" << endl;
    }

    void addChild(){
        // 叶节点不能增加子节点
        cerr << "Try to add children into leaf node" << endl;
        assert(false);
    }

    vector<T> data;
    BplusLeaf* next = NULL;
};

template <typename K, typename T>
struct BplusTree {
public:
    BplusTree(int maxSize) : maxSize(maxSize){
        this->root = new BplusLeaf<K, T>(maxSize, NULL, this);
    }
    ~BplusTree() {
        if(this->root != NULL)
            delete this->root;
    }

    T* find (const K& key) {
        currentNode = this->root;
        while(!currentNode->isLeaf) {
            if(currentNode->size() == 0) return NULL;
            for(int i = 0;i < currentNode->size(); ++i) {
                if(key < currentNode->key[i]){
                    currentNode = currentNode->children[i];
                    break;
                } else if(i == currentNode->size() - 1) {
                    currentNode = currentNode->children[i + 1];
                    break;
                }
            }
        }
        
        BplusLeaf<K, T>* leafNode = static_cast<BplusLeaf<K, T>*>(currentNode);
        if(leafNode->size() == 0) return NULL;
        for(int i = 0;i < leafNode->size(); ++i) {
            if(leafNode->key[i] == key) {
                return &(leafNode->data[i]);
            }
        }
        return NULL;
    }

    bool insert(const K& key, const T& value) {
        if(this->find(key) != NULL) return false;
        // 通过 currentNode 指针可以访问到插入的叶子节点
        assert(currentNode != NULL);
        assert(currentNode->isLeaf);
        BplusLeaf<K, T>* leafNode = static_cast<BplusLeaf<K, T>*>(currentNode);
        // 从叶节点插入并且向上更新
        leafNode->insert(key, value);
        // 更新根节点
        while(this->root->parent != nullptr) {
            this->root = this->root->parent;
        }
        return true;
    }

    bool remove(const K& key) {
        if(this->find(key) == NULL) return false;

        assert(currentNode != NULL);
        assert(currentNode->isLeaf);
        BplusLeaf<K, T>* leafNode = static_cast<BplusLeaf<K, T>*>(currentNode);

        leafNode->remove(key);
        return true;
    }

    bool update(const K& key, const T& value) {
        if(this->find(key) == NULL) return false;

        assert(currentNode != NULL);
        assert(currentNode->isLeaf);
        BplusLeaf<K, T>* leafNode = static_cast<BplusLeaf<K, T>*>(currentNode);

        leafNode->update(key, value);
    }
public: 
    BplusNode<K, T>* root;
    BplusNode<K, T>* currentNode = NULL;
    int maxSize;
};

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
    for(int i = 0;i <= 20; ++i){
        tree.remove(i);
    }
    BplusNode<int, int>::debug_print();
    return 0;
}