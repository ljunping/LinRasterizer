//
// Created by Lin on 2024/11/22.
//

#ifndef UTIL_H
#define UTIL_H
#include <unordered_map>
#include <vector>
#define VectorRemoveHelper_MIN_REMOVE_SIZE 16
template<typename T>
class VectorRemoveHelperElementInterface
{
public:
    T data;
    bool remove;
    VectorRemoveHelperElementInterface(const T& data): data(data)
    {
        remove = false;
    }
    void set_remove()
    {
        remove = true;
    }
    bool is_remove()
    {
        return remove;
    }
};



template <typename T>
class VectorRemoveEasyIterator;
template <typename T>
class SimpleVectorRemoveEasyIterator;

template <typename T>
class VectorRemoveEasy
{
    friend class VectorRemoveEasyIterator<T>;
private:
    int cur_remove_count = 0;
    std::vector<VectorRemoveHelperElementInterface<T>> data_vec;
    std::unordered_map<T, int> index_map;
    bool is_remove(int index);
public:
    void easy_remove(T& elem);
    int valid_size() const;
    int size() const;
    void push_back(T& elem);
    void pop_back();
    void clear();
    VectorRemoveHelperElementInterface<T>& operator[](int index);
    VectorRemoveEasyIterator<T> begin();
    VectorRemoveEasyIterator<T> end();
};



template <typename T>
class VectorRemoveEasyIterator
{
    friend class VectorRemoveEasy<T>;
private:
    int _index = 0;
    std::vector<VectorRemoveHelperElementInterface<T>> & _VectorRemoveEasy;
    void next_index()
    {
        while (_index < _VectorRemoveEasy.size() && _VectorRemoveEasy[_index].is_remove())
        {
            _index++;
        }
    }
    void pre_index()
    {
        while (_index >= 0 && _index < _VectorRemoveEasy.size() && _VectorRemoveEasy[_index].is_remove())
        {
            _index--;
        }
    }
public:
    explicit VectorRemoveEasyIterator(std::vector<VectorRemoveHelperElementInterface<T>>& _VectorRemoveEasy, int index):
    _VectorRemoveEasy(_VectorRemoveEasy),_index(index)
    {

    }
    T& operator*()
    {
        return _VectorRemoveEasy[_index].data;
    }
    VectorRemoveEasyIterator& operator++()
    {
        _index++;
        next_index();
        return *this;
    }
    VectorRemoveEasyIterator& operator--()
    {
        _index--;
        pre_index();
        return *this;
    }
    bool operator<(const VectorRemoveEasyIterator& other)
    {
        return _index < other._index;
    }
    bool operator<=(const VectorRemoveEasyIterator& other)
    {
        return _index <= other._index;
    }
    bool operator>=(const VectorRemoveEasyIterator& other)
    {
        return _index >= other._index;
    }

    bool operator>(const VectorRemoveEasyIterator& other)
    {
        return _index > other._index;
    }

    bool operator!=(const VectorRemoveEasyIterator & other)
    {
        return _index != other._index;
    }

    bool operator==(const VectorRemoveEasyIterator & other)
    {
        return _index == other._index;
    }
};


template <typename T>
void VectorRemoveEasy<T>::easy_remove(T& elem)
{
    if (!index_map.contains(elem))
    {
        throw std::invalid_argument("elem not found");
    }
    int index = index_map[elem];
    if (!this->data_vec[index].is_remove())
    {
        this->data_vec[index].set_remove();
        cur_remove_count++;
    }
    if (cur_remove_count > VectorRemoveHelper_MIN_REMOVE_SIZE && cur_remove_count > this->size() / 2)
    {
        index_map.clear();
        int count = this->size();
        int l = 0;
        for (int i = 0; i < count; ++i)
        {
            auto& var = this->data_vec[i];
            if (!var.is_remove())
            {
                this->data_vec[l] = var;
                index_map[var.data] = l;
                l++;
            }
        }
        this->data_vec.erase(this->data_vec.begin() + l, this->data_vec.end());
        cur_remove_count = 0;
    }
}


template <typename T>
bool VectorRemoveEasy<T>::is_remove(int index)
{
    return this->data_vec[index].is_remove();
}

template <typename T>
int VectorRemoveEasy<T>::valid_size() const
{
    return this->data_vec.size() - cur_remove_count;
}

template <typename T>
int VectorRemoveEasy<T>::size() const
{
    return this->data_vec.size();
}

template <typename T>
void VectorRemoveEasy<T>::push_back(T& elem)
{
    this->data_vec.emplace_back(elem);
    index_map[elem] = this->data_vec.size() - 1;
}

template <typename T>
void VectorRemoveEasy<T>::pop_back()
{
    if (this->data_vec.size() == 0)
    {
        throw std::out_of_range("Vector size is 0");
    }
    auto back = this->data_vec.back();
    if (back.is_remove())
    {
        cur_remove_count--;
    }
    index_map.erase(back.data);
    this->data_vec.pop_back();
}

template <typename T>
void VectorRemoveEasy<T>::clear()
{
    this->data_vec.clear();
    cur_remove_count = 0;
    index_map.clear();
}

template <typename T>
VectorRemoveHelperElementInterface<T>& VectorRemoveEasy<T>::operator[](int index)
{
    return this->data_vec[index];
}


template <typename T>
VectorRemoveEasyIterator<T> VectorRemoveEasy<T>::begin()
{
    auto vector_remove_easy_iterator = VectorRemoveEasyIterator<T>(this->data_vec, 0);
    vector_remove_easy_iterator.next_index();
    return vector_remove_easy_iterator;
}

template <typename T>
VectorRemoveEasyIterator<T> VectorRemoveEasy<T>::end()
{
    auto vector_remove_easy_iterator = VectorRemoveEasyIterator<T>(this->data_vec, this->size());
    return vector_remove_easy_iterator;
}


#endif //UTIL_H
