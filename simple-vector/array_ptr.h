#pragma once
#include <cassert>
#include <cstdlib>
#include <utility>

template <typename Type>
class ArrayPtr {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    // Инициализирует ArrayPtr нулевым указателем
    ArrayPtr() = default;

    // Создаёт в куче массив из size элементов типа Type.
    // Если size == 0, поле raw_ptr_ должно быть равно nullptr
    explicit ArrayPtr(size_t size) {
        raw_ptr_ = size != 0 ? new Type[size]{0} : nullptr;
    }

    // Конструктор из сырого указателя, хранящего адрес массива в куче либо nullptr
    explicit ArrayPtr(Type* raw_ptr) noexcept {
        raw_ptr_ = raw_ptr;
    }

    // Запрещаем копирование
    ArrayPtr(const ArrayPtr&) = delete;

    ArrayPtr(ArrayPtr&& other) :
        raw_ptr_(std::exchange(other.raw_ptr_, nullptr))
    {}

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    // Запрещаем присваивание
    ArrayPtr& operator=(const ArrayPtr&) = delete;

    ArrayPtr& operator=(ArrayPtr&& other) {
        raw_ptr_ = std::exchange(other.raw_ptr_, nullptr);
        return *this;
    }

    // Прекращает владением массивом в памяти, возвращает значение адреса массива
    // После вызова метода указатель на массив должен обнулиться
    [[nodiscard]] Type* Release() noexcept {
        Type* array = raw_ptr_;
        raw_ptr_ = nullptr;
        return array;
    }

    // Возвращает ссылку на элемент массива с индексом index
    Type& operator[](size_t index) noexcept {
        return raw_ptr_[index];
    }

    // Возвращает константную ссылку на элемент массива с индексом index
    const Type& operator[](size_t index) const noexcept {
        return raw_ptr_[index];
    }

    auto operator* () {
        return *raw_ptr_;
    }

    // Возвращает true, если указатель ненулевой, и false в противном случае
    explicit operator bool() const {
        return raw_ptr_ != nullptr;
    }

    // Возвращает значение сырого указателя, хранящего адрес начала массива
    Type* Get() const noexcept {
        return raw_ptr_;
    }

    // Обменивается значениям указателя на массив с объектом other
    void swap(ArrayPtr& other) noexcept {
        Type* tmp_ptr = other.raw_ptr_;
        other.raw_ptr_ = raw_ptr_;
        raw_ptr_ = tmp_ptr;
    }

    Iterator begin() noexcept {
        return raw_ptr_;
    }

    ConstIterator begin() const noexcept {
        return raw_ptr_;
    }

    ConstIterator cbegin() const noexcept {
        return raw_ptr_;
    }

private:
    Type* raw_ptr_ = nullptr;
};

template <typename Type>
inline bool operator==(const ArrayPtr<Type>& lhs, const ArrayPtr<Type>& rhs) {
    return lhs.Get() == rhs.Get();
}

template <typename Type>
inline bool operator!=(const ArrayPtr<Type>& lhs, const ArrayPtr<Type>& rhs) {
    return lhs.Get() != rhs.Get();
}