#pragma once
#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <iterator>
#include <utility>

#include "array_ptr.h"

struct ReserveProxyObj {
    size_t to_reserve = 0;

    ReserveProxyObj(size_t capacity) : to_reserve(capacity) {

    }
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    SimpleVector(const ReserveProxyObj& obj) {
        Reserve(obj.to_reserve);
    }

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) {
        Initialize(size);
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) {
        Initialize(size);
        std::fill_n(items_, size_, value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {
        Initialize(init.size());
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(const SimpleVector& dimple) :
        size_(dimple.size_),
        capacity_(dimple.capacity_)
    {
        items_ = new Type[dimple.capacity_]{ 0 };
        std::copy(dimple.begin(), dimple.end(), begin());
    }

    SimpleVector(SimpleVector&& dimple) :
        items_(std::move(dimple.items_)),
        size_(std::exchange(dimple.size_, 0)),
        capacity_(std::exchange(dimple.capacity_, 0))
    {}

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert((index < size_) && (index >= 0));
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert((index < size_) && (index >= 0));
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_array(new_capacity);
            if (items_.Get()) {
                std::move(begin(), end(), new_array.begin());
            }
            capacity_ = new_capacity;
            items_.~ArrayPtr();
            items_ = std::move(new_array);
        }
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        Reserve(new_size);
        size_ = new_size;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.begin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.begin() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.begin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_.begin() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_.begin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_.begin() + size_;
    }

    ~SimpleVector() {}

    SimpleVector& operator=(SimpleVector&& dimple) {
        if (items_ != dimple.items_) {
            ArrayPtr<Type> tmp_items = std::move(dimple.items_);
            items_.~ArrayPtr();
            items_ = std::move(tmp_items);
            size_ = dimple.size_;
            capacity_ = dimple.capacity_;
        }
        return *this;
    }

    SimpleVector& operator=(const SimpleVector& dimple) {
        if (items_ != dimple.items_) {
            Type* tmp_items = new Type[dimple.capacity_]{ 0 };
            std::copy(dimple.begin(), dimple.end(), tmp_items);
            delete[] items_;
            items_ = tmp_items;
            size_ = dimple.size_;
            capacity_ = dimple.capacity_;
        }
        return *this;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert((pos >= begin()) && (pos <= end()));
        size_t insert_index = GetIndex(pos);
        if (size_ == capacity_) {
            Reserve(std::max(capacity_ * 2, size_t(1)));
        }
        std::move_backward(begin() + insert_index, end(), end() + 1);
        items_[insert_index] = std::move(value);
        ++size_;
        return begin() + insert_index;
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        assert((pos >= begin()) && (pos <= end()));
        size_t insert_index = GetIndex(pos);
        if (size_ == capacity_) {
            Reserve(std::max(capacity_ * 2, size_t(1)));
        }
        std::move_backward(begin() + insert_index, end(), end() + 1);
        items_[insert_index] = value;
        ++size_;
        return begin() + insert_index;
    }

    void PushBack(const Type& item) {
        Insert(cend(), item);
    }

    void PushBack(Type&& item) {
        Insert(cend(), std::move(item));
    }

    void PopBack() noexcept {
        if (size_) {
            --size_;
        }
    }

    Iterator Erase(ConstIterator pos) {
        assert((pos >= begin()) && (pos <= end()));
        std::move(begin() + GetIndex(pos) + 1, end()+1, begin() + GetIndex(pos));
        --size_;
        return const_cast<Type*>(pos);
    }

    void swap(SimpleVector& other) noexcept {
        std::swap(items_, other.items_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
    }

private:
    ArrayPtr<Type> items_;

    size_t size_ = 0;
    size_t capacity_ = 0;

    void ChangeCapacity(size_t new_capacity) {
        size_t old_size = size_;
        Resize(new_capacity);
        Resize(old_size);
    }

	void Initialize(size_t size) {
		items_ = ArrayPtr<Type>(size);
		size_ = size;
		capacity_ = size;
	}

    size_t GetIndex(ConstIterator pos) {
        return (pos - begin());
    }
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::less());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs == rhs) || (lhs < rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::greater());
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs == rhs) || (lhs > rhs);
}
