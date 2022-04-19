#ifndef SKIPLIST_H
#define SKIPLIST_H
#include<cstring>
#include<cstdlib>
#include<iostream>
#include <mutex>
namespace MyDatebase{

template<typename K, typename V>
class Node
{
private:
    K key_;
    V value_;
public:
    int next_node_level_height_;
    Node<K,V>** next_node_;
    Node(int height);
    Node(K k, V v,int height);
    Node<K,V>** getNextNode(){
        return next_node_;
    }
    K getKey(){
        return key_;
    }
    V getValue(){
        return value_;
    }
    int getNodeHeight(){
        return next_node_level_height_;
    }
    ~Node(){
        delete [] next_node_;
    }
};

template<typename K, typename V>
Node<K,V>::Node(int height){
    next_node_level_height_ = height;
    this->next_node_ = new Node<K,V>*[height + 1];
    memset(this->next_node_,0,sizeof(Node<K, V>*)*(height+1));
}

template<typename K, typename V>
Node<K,V>::Node(K k, V v,int height): key_(k),value_(v),next_node_level_height_(height)
{
    this->next_node_ = new Node<K,V>*[height + 1];
    memset(this->next_node_,0,sizeof(Node<K, V>*)*(height+1));
}

template<typename K, typename V>
class skiplist
{
private:
    int skiplist_max_level_;
    Node<K,V>* skiplistHead_;
    int getRandomNumber();
    int elements_;
    std::mutex mtx_;
    void mSearchElement_( Node<K,V>* update[] ,K key);
    Node<K,V>* create_node(K k ,V v, int height);

public:
int current_max_level_;
void destory();
    skiplist(int maxlevel);
    bool insertElement(K key,V value);
    bool deleteElement(K key);

    int size(){
        return this->elements_;
    }
    void display_list();

    bool searchElement(K key);
    ~skiplist();
};
template<typename K, typename V>
Node<K,V>* skiplist<K, V>::create_node(K key ,V value, int random_level){
    Node<K,V>* node = new Node<K,V>(key,value,random_level);
    return node;
}

template<typename K, typename V>
skiplist<K, V>::skiplist(int maxlevel): skiplist_max_level_(maxlevel)
{
      this->current_max_level_ = 0;
      this->elements_ = 0;
      K k;
      V v;
      skiplistHead_ = create_node( k,v,maxlevel + 1);
}

template<typename K, typename V>
void skiplist<K, V>::destory(){
    if( this->size() > 0 ){
        Node<K,V>* ptr = skiplistHead_->next_node_[0];
        for (int i = 0; i <= current_max_level_; i++) {
            skiplistHead_->next_node_[i] = NULL;
        }
        while(ptr != NULL ){
            Node<K,V>* del = ptr;
            ptr = ptr->next_node_[0];
            delete del;
            elements_--;
        }
    }
}

template<typename K, typename V>
skiplist<K, V>::~skiplist()
{
    destory();
    delete skiplistHead_;
}

template<typename K, typename V>
void skiplist<K, V>::mSearchElement_( Node<K,V>* update[] ,K key){
    Node<K,V>* current = this->skiplistHead_;
    for(int i = current_max_level_ ; i >= 0 ;i-- ){
        int k = 0;
        if(current->next_node_[i] != NULL ){
            k = current->next_node_[i]->getKey();
        }
        while(current->next_node_[i] != NULL && current->next_node_[i]->getKey() < key ){
            current = current->next_node_[i];
        }
        update[i] = current;
    }
}

template<typename K, typename V>
bool skiplist<K, V>::searchElement(K key){
    bool ret = false;
    // 从head开始找，找到合适的位置。
    Node<K,V>* update [skiplist_max_level_ + 1];
    memset(update, 0, sizeof(Node<K, V>*)*(skiplist_max_level_+1));
    mSearchElement_(update,key);

    Node<K,V>* search_node = update[0]->next_node_[0];
    if( search_node != NULL && search_node->getKey() == key ){
        std::cout<<"find the key: "<<key<<std::endl;
        ret = true;
    }

    return ret;
}

template<typename K, typename V>
bool skiplist<K, V>::deleteElement(K key){
    bool ret = false;
    Node<K,V>* update [skiplist_max_level_ + 1];
    memset(update, 0, sizeof(Node<K, V>*)*(skiplist_max_level_+1));

    mtx_.lock();
    // 从head开始找，找到合适的位置。
    mSearchElement_(update,key);

    Node<K,V>* delete_node = update[0]->next_node_[0];
    //如果已经存在了
    if( delete_node != NULL && delete_node->getKey() == key ){
        for (int i = 0; i <= delete_node->next_node_level_height_; i++) {
            update[i]->next_node_[i] = delete_node->next_node_[i];
        }
        while( this->current_max_level_ > 0 && skiplistHead_->next_node_[current_max_level_] == 0){
            this->current_max_level_--;
        }
        ret = true;
        std::cout<<"delete the key: "<<key<<std::endl;
        this->elements_--;
        delete delete_node;
    }else{
        std::cout<<" the key: "<<key<<" don't exist"<<std::endl;
    mtx_.unlock();
        return false;
    }
    mtx_.unlock();
    return ret;
}

template<typename K, typename V>
bool skiplist<K, V>::insertElement(K key,V value){
    bool ret = false;
    Node<K,V>* update [skiplist_max_level_ + 1];
    memset(update, 0, sizeof(Node<K, V>*)*(skiplist_max_level_+1));

    mtx_.lock();
    // 从head开始找，找到合适的位置。
    mSearchElement_(update,key);

    // 插入位置
    Node<K,V>* next = update[0]->next_node_[0];
    //如果已经存在了
    if( next != NULL && next->getKey() == key ){
        std::cout<<" the key: "<<key<<"has exists"<<std::endl;
    mtx_.unlock();
        return false;
    }else{
       //如果不存在的话， 获得随机的层高
      int random_level = getRandomNumber();
      Node<K,V>* node =  create_node(key,value,random_level);
      // 需要进行层高扩容
      if( random_level > this->current_max_level_ ){
          for (int i = current_max_level_+1 ; i <= random_level; i++) {
              update[i] = skiplistHead_;
          }
          this->current_max_level_ = random_level;
      }
      // 插入
      for (int i = 0; i <= random_level; i++) {
          node->next_node_[i] = update[i]->next_node_[i];
          update[i]->next_node_[i] = node;
      }
      std::cout<<" insert the key: "<<key<<", value : "<<value<<std::endl;
      this->elements_++;
      ret = true;
    }
    mtx_.unlock();
    return ret;
}


template<typename K, typename V>
int skiplist<K, V>::getRandomNumber(){
    return rand()% skiplist_max_level_;
}

template<typename K, typename V>
void skiplist<K, V>::display_list() {
    std::cout << "\n*****Skip List*****"<<"\n";
    for (int i = 0; i <= skiplist_max_level_; i++) {
        Node<K, V> *node = this->skiplistHead_->next_node_[i];
        std::cout << "Level " << i << ": ";
        while (node != NULL) {
            std::cout << node->getKey() << ":" << node->getValue() << ", ";
            node = node->next_node_[i];
        }
        std::cout << std::endl;
    }
}

}

#endif // SKIPLIST_H
