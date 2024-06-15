#include <memory>
#include <vector>
#include <type_traits>


/*  !!!!!!!  WARNING  !!!!!!!  //

Летом 2024 году автор этого кода все еще является студентом 1 курса.
Если ты, мой однокурсник, скопируешь код отсюда, то будет БАН (нам обоим).
Надеюсь на твою сознательность. (Не копируй, пожалуйста)

//  !!!!!!!  WARNING  !!!!!!!  */


template<size_t N>
class StackStorage;


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// StackAllocator declaration //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


template<typename T, size_t N>
class StackAllocator {
  StackStorage<N>* storage;

  explicit StackAllocator(StackStorage<N>*);

public:
  using pointer = T*;
  using const_pointer = const T*;
  using void_pointer = void*;
  using const_void_pointer = const void*;
  using value_type = T;
  using size_type = size_t;
  using difference_type = long long;

  template<typename U, size_t M>
  friend class StackAllocator;

  T* allocate(size_t);

  void deallocate(T*, size_t);

  StackAllocator() : storage(&StackStorage<N>()) {}

  explicit StackAllocator(StackStorage<N>& st) : storage(&st) {}

  template<typename U>
  StackAllocator(const StackAllocator<U, N>& other) : storage(const_cast<StackStorage<N>*>(other.storage)) {}

  StackAllocator<T, N>& operator=(const StackAllocator&) = default;

  ~StackAllocator() = default;

  bool operator==(const StackAllocator&) const;

  bool operator!=(const StackAllocator&) const;

  template<typename U>
  struct rebind {
    using other = StackAllocator<U, N>;
  };
};


////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// StackStorage declaration //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


template<size_t N>
class StackStorage {
  char array[N];
  char* cur;

  char* reserve(size_t);

  void align(int);

public:

  StackStorage() : array(), cur(array) {}

  StackStorage(const StackStorage&) = delete;

  template<typename T, size_t M>
  friend class StackAllocator;
};


////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// List declaration //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


template<typename T, typename A = std::allocator<T>>
class List {

  ///************* Node *************///

  struct SimpleNode {
    SimpleNode* previous = nullptr;
    SimpleNode* next = nullptr;
  };

  struct Node : SimpleNode {
    T value;

    explicit Node(const T&);

    Node() : value() {};
  };

  ///********************************///

  using NodeAlloc = typename std::allocator_traits<A>::template rebind_alloc<Node>;
  using NodeTraits = std::allocator_traits<NodeAlloc>;

  SimpleNode head;
  size_t sz;
  [[no_unique_address]] NodeAlloc allocator;

  void link(SimpleNode*, SimpleNode*);

  void spawn(unsigned int);

  void spawn_with_example(unsigned int, const T&);

  void spawn_from_other(const List<T, A>&, NodeAlloc&);

  void clear();

public:

  ///*********** Iterator ***********///

  template<bool is_const>
  class ListIterator {
    SimpleNode* pin;

    explicit ListIterator(SimpleNode*);

    explicit ListIterator(const SimpleNode*);

    friend class List;

  public:
    using value_type = typename std::conditional<is_const, const T, T>::type;
    using difference_type = int;
    using iterator_category = std::bidirectional_iterator_tag;
    using reference = value_type&;
    using pointer = value_type*;

    ListIterator& operator++();

    ListIterator& operator--();

    ListIterator operator++(int);

    ListIterator operator--(int);

    bool operator==(const ListIterator&) const;

    bool operator!=(const ListIterator&) const;

    reference operator*();

    const T& operator*() const;

    pointer operator->();

    const T* operator->() const;

    operator ListIterator<true>() const {
      return ListIterator<true>(pin);
    }
  };

  ///********************************///

  using iterator = ListIterator<false>;
  using const_iterator = ListIterator<true>;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  List();

  explicit List(unsigned int);

  List(unsigned int, const T&);

  explicit List(const A&);

  List(unsigned int, const A&);

  List(unsigned int, const T&, const A&);

  List(const List<T, A>&);

  A get_allocator() const;

  List& operator=(const List<T, A>&);

  size_t size() const;

  void push_back(const T&);

  void push_front(const T&);

  void pop_back();

  void pop_front();

  ~List();

  iterator begin();

  iterator end();

  const_iterator begin() const;

  const_iterator end() const;

  const_iterator cbegin() const;

  const_iterator cend() const;

  reverse_iterator rbegin();

  reverse_iterator rend();

  const_reverse_iterator rbegin() const;

  const_reverse_iterator rend() const;

  const_reverse_iterator crbegin() const;

  const_reverse_iterator crend() const;

  template<bool is_const>
  void insert(const ListIterator<is_const>&, const T&);

  template<bool is_const>
  void erase(const ListIterator<is_const>&);
};


///////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// StackAllocator definition /////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


template<typename T, size_t N>
StackAllocator<T, N>::StackAllocator(StackStorage<N>* st) : storage(st) {}

template<typename T, size_t N>
T* StackAllocator<T, N>::allocate(size_t amount) {
  storage->align(alignof(T));
  size_t range;
  range = sizeof(T) - sizeof(T) % alignof(T) + alignof(T);
  return reinterpret_cast<T*>(storage->reserve(amount * range));
}

template<typename T, size_t N>
void StackAllocator<T, N>::deallocate(T*, size_t) {}

template<typename T, size_t N>
bool StackAllocator<T, N>::operator==(const StackAllocator& other) const {
  return storage == other.storage;
}

template<typename T, size_t N>
bool StackAllocator<T, N>::operator!=(const StackAllocator& other) const {
  return !(*this == other);
}

///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// StackStorage definition //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


template<size_t N>
char* StackStorage<N>::reserve(size_t amount) {
  if (cur - array + amount >= N) {
    return nullptr;
  }
  cur += amount;
  return cur - amount;
}

template<size_t N>
void StackStorage<N>::align(int size) {
  size_t space = N - (cur - array);
  void* cur_void = reinterpret_cast<void*>(cur);
  cur = static_cast<char *>(std::align(size, size, cur_void, space));
}


///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// List definition //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

///************* Node *************///

template<typename T, typename A>
List<T, A>::Node::Node(const T& va) : value(va) {}

///*********** Iterator ***********///

template<typename T, typename A>
template<bool is_const>
List<T, A>::ListIterator<is_const>::ListIterator(SimpleNode* po) : pin(po) {}

template<typename T, typename A>
template<bool is_const>
List<T, A>::ListIterator<is_const>::ListIterator(const SimpleNode* po) : pin(const_cast<SimpleNode*>(po)) {}

template<typename T, typename A>
template<bool is_const>
typename List<T, A>::template ListIterator<is_const>& List<T, A>::ListIterator<is_const>::operator++() {
  pin = pin->next;
  return *this;
}

template<typename T, typename A>
template<bool is_const>
typename List<T, A>::template ListIterator<is_const>& List<T, A>::ListIterator<is_const>::operator--() {
  pin = pin->previous;
  return *this;
}

template<typename T, typename A>
template<bool is_const>
typename List<T, A>::template ListIterator<is_const> List<T, A>::ListIterator<is_const>::operator++(int) {
  ListIterator<is_const> it(*this);
  ++*this;
  return it;
}

template<typename T, typename A>
template<bool is_const>
typename List<T, A>::template ListIterator<is_const> List<T, A>::ListIterator<is_const>::operator--(int) {
  ListIterator<is_const> it(*this);
  --*this;
  return it;
}

template<typename T, typename A>
template<bool is_const>
bool List<T, A>::ListIterator<is_const>::operator==(const ListIterator<is_const>& other) const {
  return pin == other.pin;
}

template<typename T, typename A>
template<bool is_const>
bool List<T, A>::ListIterator<is_const>::operator!=(const ListIterator<is_const>& other) const {
  return !(*this == other);
}

template<typename T, typename A>
template<bool is_const>
typename List<T, A>::template ListIterator<is_const>::reference List<T, A>::ListIterator<is_const>::operator*() {
  return static_cast<Node*>(pin)->value;
}

template<typename T, typename A>
template<bool is_const>
const T& List<T, A>::ListIterator<is_const>::operator*() const {
  return static_cast<Node*>(pin)->value;
}

template<typename T, typename A>
template<bool is_const>
typename List<T, A>::template ListIterator<is_const>::pointer List<T, A>::ListIterator<is_const>::operator->() {
  return &(static_cast<Node*>(pin)->value);
}

template<typename T, typename A>
template<bool is_const>
const T* List<T, A>::ListIterator<is_const>::operator->() const {
  return &(static_cast<Node*>(pin)->value);
}

///********************************///

template<typename T, typename A>
void List<T, A>::link(SimpleNode* dad, SimpleNode* son) {
  dad->next = son;
  son->previous = dad;
}

template<typename T, typename A>
void List<T, A>::spawn(unsigned int amount) {
  if (amount == 0) {
    return;
  }
  SimpleNode* last = &head;
  unsigned int pos = 0;
  try {
    for (; pos < amount; ++pos) {
      SimpleNode* cur = NodeTraits::allocate(allocator, 1);
      NodeTraits::construct(allocator, static_cast<Node*>(cur));
      link(last, cur);
      last = cur;
    }
    link(last, &head);
    sz = amount;
  } catch (...) {
    sz = pos;
    link(last, &head);
    if (sz == 0) {
      return;
    }
    clear();
    sz = 0;
    throw;
  }
}

template<typename T, typename A>
void List<T, A>::spawn_with_example(unsigned int amount, const T& example) {
  if (amount == 0) {
    return;
  }
  SimpleNode* last = &head;
  int pos = 0;
  try {
    for (; pos < amount; ++pos) {
      SimpleNode* cur = NodeTraits::allocate(allocator, 1);
      NodeTraits::construct(allocator, static_cast<Node*>(cur), example);
      link(last, cur);
      last = cur;
    }
    link(last, &head);
    sz = amount;
  } catch (...) {
    sz = pos;
    link(last, &head);
    if (sz == 0) {
      return;
    }
    clear();
    sz = 0;
    throw;
  }
}


template<typename T, typename A>
void List<T, A>::spawn_from_other(const List<T, A>& other, NodeAlloc& alloc) {
  if (other.sz == 0) {
    return;
  }
  SimpleNode* last = &head;
  auto example = other.cbegin();
  size_t pos = 0;
  try {
    for (; pos < other.sz; ++pos) {
      SimpleNode* cur = NodeTraits::allocate(alloc, 1);
      NodeTraits::construct(alloc, static_cast<Node*>(cur), *example);
      link(last, cur);
      last = cur;
      ++example;
    }
    link(last, &head);
    sz = other.sz;
  } catch (...) {
    sz = pos;
    link(last, &head);
    if (sz == 0) {
      return;
    }
    clear();
    sz = 0;
    throw;
  }
}

template<typename T, typename A>
List<T, A>::List() : head(SimpleNode()), sz(0), allocator() {
  link(&head, &head);
}

template<typename T, typename A>
List<T, A>::List(unsigned int amount) : head(SimpleNode()), sz(0), allocator() {
  link(&head, &head);
  spawn(amount);
}

template<typename T, typename A>
List<T, A>::List(unsigned int amount, const T& example) : head(SimpleNode()), sz(0), allocator() {
  link(&head, &head);
  spawn_with_example(amount, example);
}

template<typename T, typename A>
List<T, A>::List(const A& alloc) : head(SimpleNode()), sz(0), allocator(alloc) {
  link(&head, &head);
}

template<typename T, typename A>
List<T, A>::List(unsigned int amount, const A& alloc) : head(SimpleNode()),  sz(0), allocator(alloc) {
  link(&head, &head);
  spawn(amount);
}

template<typename T, typename A>
List<T, A>::List(unsigned int amount, const T& example, const A& alloc) : head(SimpleNode()), sz(0), allocator(alloc) {
  link(&head, &head);
  spawn_with_example(amount, example);
}

template<typename T, typename A>
List<T, A>::List(const List<T, A>& other)
    : head(SimpleNode()), sz(0), allocator(NodeTraits::select_on_container_copy_construction(other.allocator)) {
  link(&head, &head);
  spawn_from_other(other, allocator);
}

template<typename T, typename A>
A List<T, A>::get_allocator() const {
  return allocator;
}

template<typename T, typename A>
List<T, A>& List<T, A>::operator=(const List<T, A>& other) {
  if (&other == this) {
    return *this;
  }
  SimpleNode* ne = head.next;
  SimpleNode* prev = head.previous;
  size_t saved = sz;
  link(&head, &head);
  try {
    if (NodeTraits::propagate_on_container_copy_assignment::value) {
      NodeAlloc buf = other.allocator;
      spawn_from_other(other, buf);
    } else {
      spawn_from_other(other, allocator);
    }
    std::swap(ne, head.next);
    std::swap(prev, head.previous);

    clear();

    head.next = ne;
    head.previous = prev;
    sz = other.sz;
    if (NodeTraits::propagate_on_container_copy_assignment::value) {
      allocator = other.allocator;
    }
  } catch (...) {
    head.next = ne;
    head.previous = prev;
    sz = saved;
    throw;
  }
  return *this;
}

template<typename T, typename A>
size_t List<T, A>::size() const {
  return sz;
}

template<typename T, typename A>
void List<T, A>::push_back(const T& element) {
  try {
    Node* temp = NodeTraits::allocate(allocator, 1);
    NodeTraits::construct(allocator, temp, element);
    link(head.previous, temp);
    link(temp, &head);
    ++sz;
  } catch  (...) {
    throw;
  }
}

template<typename T, typename A>
void List<T, A>::push_front(const T& element) {
  try {
    Node* temp = NodeTraits::allocate(allocator, 1);
    NodeTraits::construct(allocator, temp, element);
    link(temp, head.next);
    link(&head, temp);
    ++sz;
  } catch  (...) {
    throw;
  }
}

template<typename T, typename A>
void List<T, A>::pop_back() {
  SimpleNode* prev = head.previous->previous;
  NodeTraits::destroy(allocator, static_cast<Node*>(head.previous));
  NodeTraits::deallocate(allocator, static_cast<Node*>(head.previous), 1);
  link(prev, &head);
  --sz;
}

template<typename T, typename A>
void List<T, A>::pop_front() {
  SimpleNode* ne = head.next->next;
  NodeTraits::destroy(allocator, static_cast<Node*>(head.next));
  NodeTraits::deallocate(allocator, static_cast<Node*>(head.next), 1);
  link(&head, ne);
  --sz;
}

template<typename T, typename A>
List<T, A>::~List() {
  if (sz == 0) {
    return;
  }
  clear();
  sz = 0;
}

template<typename T, typename A>
typename List<T, A>::iterator List<T, A>::begin() {
  return List<T, A>::ListIterator<false>(head.next);
}

template<typename T, typename A>
typename List<T, A>::iterator List<T, A>::end() {
  return List<T, A>::ListIterator<false>(&head);
}

template<typename T, typename A>
typename List<T, A>::const_iterator List<T, A>::begin() const {
  return cbegin();
}

template<typename T, typename A>
typename List<T, A>::const_iterator List<T, A>::end() const {
  return cend();
}

template<typename T, typename A>
typename List<T, A>::const_iterator List<T, A>::cbegin() const {
  return const_iterator(head.next);
}

template<typename T, typename A>
typename List<T, A>::const_iterator List<T, A>::cend() const {
  return const_iterator(&head);
}

template<typename T, typename A>
typename List<T, A>::reverse_iterator List<T, A>::rbegin() {
  return reverse_iterator(end());
}

template<typename T, typename A>
typename List<T, A>::reverse_iterator List<T, A>::rend() {
  return reverse_iterator(begin());
}

template<typename T, typename A>
typename List<T, A>::const_reverse_iterator List<T, A>::rbegin() const {
  return crbegin();
}

template<typename T, typename A>
typename List<T, A>::const_reverse_iterator List<T, A>::rend() const {
  return crend();
}

template<typename T, typename A>
typename List<T, A>::const_reverse_iterator List<T, A>::crbegin() const {
  return const_reverse_iterator(cend());
}

template<typename T, typename A>
typename List<T, A>::const_reverse_iterator List<T, A>::crend() const {
  return const_reverse_iterator(cbegin());
}

template<typename T, typename A>
template<bool is_const>
void List<T, A>::insert(const ListIterator<is_const>& it, const T& element) {
  SimpleNode* pin = it.pin;
  try {
    Node* cur = NodeTraits::allocate(allocator, 1);
    NodeTraits::construct(allocator, cur, element);
    link(pin->previous, cur);
    link(cur, pin);
  } catch (...) {
    throw;
  }
  ++sz;
}

template<typename T, typename A>
template<bool is_const>
void List<T, A>::erase(const ListIterator<is_const>& it) {
  SimpleNode* pin = it.pin;
  link(pin->previous, pin->next);
  NodeTraits::destroy(allocator, static_cast<Node*>(pin));
  NodeTraits::deallocate(allocator, static_cast<Node*>(pin), 1);
  --sz;
}

template<typename T, typename A>
void List<T, A>::clear() {
  SimpleNode* cur = head.next;
  while (cur != &head) {
    cur = cur->next;
    NodeTraits::destroy(allocator, static_cast<Node*>(cur->previous));
    NodeTraits::deallocate(allocator, static_cast<Node*>(cur->previous), 1);
  }
}

