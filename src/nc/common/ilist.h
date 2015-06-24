/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <cassert>
#include <iterator>
#include <memory> /* std::unique_ptr */
#include <type_traits>
#include <utility> /* std::swap */

namespace nc {

/**
 * Base class for the elements of intrusive lists.
 */
class ilist_item {
    /** Pointer to the next element. */
    ilist_item *next_;

    /** Pointer to the previous element. */
    ilist_item *prev_;

protected:
    /**
     * Protected constructor.
     */
    ilist_item() noexcept: next_(nullptr), prev_(nullptr) {}

private:
    /**
     * Copying is forbidden.
     */
    ilist_item(const ilist_item &);

    /**
     * Copying is forbidden.
     */
    ilist_item &operator=(const ilist_item &);

    template<class X, class Y> friend class ilist;
    template<class X> friend class ilist_iterator;
};

/**
 * Class containing pointers to the first and last element of a list.
 *
 * \tparam T Type of elements.
 */
class ilist_data {
public:
    /** Pointer to the first element. */
    ilist_item *front_;

    /** Pointer to the last element. */
    ilist_item *back_;

protected:
    /**
     * Default constructor. Does no initialization.
     */
    ilist_data() {}

private:
    /**
     * Copying is forbidden.
     */
    ilist_data(const ilist_data &);

    /**
     * Copying is forbidden.
     */
    ilist_data &operator=(const ilist_data &);
};

/**
 * Intrusive list iterator.
 *
 * \tparam T Element type as exposed to the user.
 */
template<class T>
class ilist_iterator {
public:
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef T *value_type;
    typedef std::ptrdiff_t difference_type;
    typedef T *pointer;
    typedef T *reference;

private:
    /** Pointer to the element the iterator points to. */
    ilist_item *element_;

    /** Valid pointer to the data of the list being iterated. */
    const ilist_data *list_;

public:
    /**
     * Constructor.
     *
     * \param list      Reference to the data of the list being iterated.
     * \param element   Pointer to the element this iterator points to. Can be nullptr.
     */
    explicit ilist_iterator(const ilist_data *list, ilist_item *element = nullptr) noexcept:
        element_(element), list_(list)
    {
        assert(list != nullptr);
    }

private:
    class Tag {};

public:
    /**
     * Constructor from a compatible iterator.
     *
     * \param that Iterator to construct from.
     */
    template<class U>
    ilist_iterator(const ilist_iterator<U> &that, typename std::enable_if<std::is_convertible<U *, T *>::value, Tag>::type = Tag()) noexcept:
        element_(that.element_), list_(that.list_)
    {}

    /**
     * Assignment from a compatible iterator.
     *
     * \param that Iterator to assign from.
     */
    template<class U>
    typename std::enable_if<std::is_convertible<U *, T *>::value, ilist_iterator &>::type
    operator=(const ilist_iterator<U> &that) noexcept {
        element_ = that.element_;
        list_ = that.list_;
        return *this;
    }

    /**
     * Prefix increment.
     */
    ilist_iterator &operator++() noexcept {
        if (element_ != nullptr) {
            element_ = element_->next_;
        } else {
            element_ = list_->front_;
        }
        return *this;
    }

    /**
     * Postfix increment.
     */
    ilist_iterator operator++(int) noexcept {
        auto copy = *this;
        ++*this;
        return copy;
    }

    /**
     * Prefix decrement.
     */
    ilist_iterator &operator--() noexcept {
        if (element_ != nullptr) {
            element_ = element_->prev_;
        } else {
            element_ = list_->back_;
        }
        return *this;
    }

    /**
     * Postfix decrement.
     */
    ilist_iterator operator--(int) noexcept {
        auto copy = *this;
        --*this;
        return copy;
    }

    /**
     * \returns True if *this and that point to the same element, false otherwise.
     */
    bool operator==(const ilist_iterator &that) const noexcept {
        assert(this->list_ == that.list_);
        return this->element_ == that.element_;
    }

    /**
     * \returns True if *this and that point to different elements, false otherwise.
     */
    bool operator!=(const ilist_iterator &that) const noexcept {
        return !(*this == that);
    }

    /**
     * \return Pointer to the element being pointed to.
     */
    pointer operator->() const noexcept {
        return static_cast<pointer>(element_);
    }

    /**
     * \return Pointer to the element being pointed to.
     */
    pointer operator*() const noexcept {
        return static_cast<pointer>(element_);
    }

    template<class X> friend class ilist_iterator;
    template<class X, class Y> friend class ilist;
};

/**
* Intrusive doubly linked list owning its elements.
* Features stable iterators, insertion and deletion in O(1) time.
*
* \param T       Type of elements.
* \param Deleter Type of deleter for the list elements.
*/
template<class T, class Deleter = std::default_delete<T>>
class ilist: private ilist_data {
public:
    typedef T value_type;
    typedef Deleter deleter_type;
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef std::unique_ptr<T, deleter_type> unique_ptr;
    typedef ilist_iterator<T> iterator;
    typedef ilist_iterator<const T> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef std::size_t size_type;

    /**
     * Constructs an empty list.
     *
     * \param deleter Deleter for the list elements.
     */
    ilist(const deleter_type &deleter = deleter_type()):
        deleter_(deleter)
    {
        front_ = nullptr;
        back_ = nullptr;
    }

    /**
     * Move constructor.
     *
     * \param that The list to move from.
     */
    template<class U>
    ilist(ilist<U, Deleter> &&that):
        deleter_(std::move(that.deleter_))
    {
        front_ = that.front_;
        back_ = that.back_;

        that.front_ = nullptr;
        that.back_ = nullptr;
    }

    /**
     * Destructor.
     *
     * Destroys all the elements in the list.
     */
    ~ilist() { clear(); }

    /**
     * Move-assignment operator.
     *
     * \param that List to move from.
     */
    template<class U>
    ilist &operator=(ilist<U, Deleter> &&that) {
        clear();

        deleter_ = std::move(that.deleter_);
        front_ = that.front_;
        back_ = that.back_;

        that.front_ = nullptr;
        that.back_ = nullptr;

        return *this;
    }

    /**
     * Swaps *this and that lists.
     *
     * \param that Another list.
     */
    void swap(ilist &that) {
        std::swap(deleter_, that.deleter_);
        std::swap(front_, that.front_);
        std::swap(back_, that.back_);
    }

    /**
     * \return Deleter used for the destruction of managed elements.
     */
    deleter_type &get_deleter() noexcept { return deleter_; }

    /**
     * \return Deleter used for the destruction of managed elements.
     */
    const deleter_type &get_deleter() const noexcept { return deleter_; }

    /**
     * \return Pointer to the first element. Will be nullptr if the list is empty.
     */
    pointer front() noexcept { return static_cast<pointer>(front_); }

    /**
     * \return Pointer to the first element. Will be nullptr if the list is empty.
     */
    const_pointer front() const noexcept { return static_cast<pointer>(front_); }

    /**
     * \return Pointer to the last element. Will be nullptr if the list is empty.
     */
    pointer back() noexcept { return static_cast<pointer>(back_); }

    /**
     * \return Pointer to the last element. Will be nullptr if the list is empty.
     */
    const_pointer back() const noexcept { return static_cast<pointer>(back_); }

    /**
     * \return True if the list is empty, false otherwise.
     */
    bool empty() const noexcept { return front() == nullptr; }

    /**
     * Removes and destroys all the elements in the list.
     */
    void clear() {
        while (!empty()) {
            pop_front();
        }
    }

    /**
     * Removes given element from the list.
     *
     * \param element Valid pointer to an element in the list.
     *
     * \return Valid pointer to the removed element.
     */
    unique_ptr erase(const_pointer element) {
        assert(element != nullptr);

        ilist_item *item = const_cast<pointer>(element);

        if (item == front_) {
            front_ = front_->next_;
        }
        if (item == back_) {
            back_ = back_->prev_;
        }

        if (item->prev_) {
            item->prev_->next_ = item->next_;
        }
        if (item->next_) {
            item->next_->prev_ = item->prev_;
        }

        item->next_ = nullptr;
        item->prev_ = nullptr;

        return unique_ptr(const_cast<pointer>(element), deleter_);
    }

    /**
     * Removes the element at a given position from the list.
     *
     * \param iterator Iterator pointing to the given element.
     *
     * \return Valid pointer to the removed element.
     */
    unique_ptr erase(const_iterator iterator) {
        return erase(*iterator);
    }

    /**
     * Removes the front element from the list.
     *
     * \return Valid pointer to the erased element.
     */
    unique_ptr pop_front() {
        return erase(front());
    }

    /**
     * Removes the back element from the list.
     *
     * \return Valid pointer to the erased element.
     */
    unique_ptr pop_back() {
        return erase(back());
    }

    /**
     * Inserts the element in the list at the given position.
     *
     * \param position Iterator identifying the position.
     * \param element Valid pointer to the element to insert.
     *
     * \return Valid pointer to the inserted element.
     */
    pointer insert(const_iterator position, unique_ptr element) noexcept {
        assert(position.list_ == this);
        assert(element != nullptr);

        auto item = static_cast<ilist_item *>(element.get());

        assert(item->next_ == nullptr);
        assert(item->prev_ == nullptr);

        item->next_ = const_cast<pointer>(*position);
        item->prev_ = const_cast<pointer>(*--position);

        if (item->next_ != nullptr) {
            item->next_->prev_ = item;
        } else {
            back_ = item;
        }

        if (item->prev_ != nullptr) {
            item->prev_->next_ = element.get();
        } else {
            front_ = item;
        }

        return element.release();
    }

    /**
     * Inserts given element to the front of the list.
     *
     * \param element Valid pointer to the element.
     *
     * \return Valid pointer to the inserted element.
     */
    pointer push_front(unique_ptr element) noexcept {
        return insert(begin(), std::move(element));
    }

    /**
     * Inserts given element to the back of the list.
     *
     * \param element Valid pointer to the element.
     *
     * \return Valid pointer to the inserted element.
     */
    pointer push_back(unique_ptr element) noexcept {
        return insert(end(), std::move(element));
    }

    /**
     * \param first First element of the range to be cut out.
     * \param last Next after the last element of the range to be cut out.
     *
     * \return List containing the range being cut out.
     */
    ilist cut_out(const_iterator first, const_iterator last) {
        assert(first.list_ == this);
        assert(last.list_ == this);

        ilist result(deleter_);

        if (first == last) {
            return result;
        }

        result.front_ = const_cast<pointer>(*first);
        result.back_ = const_cast<pointer>(*--last);

        assert(result.front_ != nullptr);
        assert(result.back_ != nullptr);

        if (result.front_ == front_) {
            front_ = result.back_->next_;
            if (front_) {
                front_->prev_ = nullptr;
            }
        }

        if (result.back_ == back_) {
            back_ = result.front_->prev_;
            if (back_) {
                back_->next_ = nullptr;
            }
        }

        if (result.front_->prev_) {
            result.front_->prev_->next_ = result.back_->next_;
        }
        if (result.back_->next_) {
            result.back_->next_->prev_ = result.front_->prev_;
        }

        result.front_->prev_ = nullptr;
        result.back_->next_ = nullptr;

        return result;
    }

    /**
     * \return Iterator pointing to the first element.
     */
    iterator begin() noexcept { return iterator(this, front_); }

    /**
     * \return Iterator pointing to the next after the last element.
     */
    iterator end() noexcept { return iterator(this); }

    /**
     * \return Constant iterator to the first element.
     */
    const_iterator begin() const noexcept { return cbegin(); }

    /**
     * \return Constant iterator to the next after the last element.
     */
    const_iterator end() const noexcept { return cend(); }

    /**
     * Reverse iterator pointing to the last element.
     */
    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

    /**
     * Reverse iterator pointing to the element before the first one.
     */
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

    /**
     * Reverse iterator pointing to the last element.
     */
    const_reverse_iterator rbegin() const noexcept { return crbegin(); }

    /**
     * Reverse iterator pointing to the element before the first one.
     */
    const_reverse_iterator rend() const noexcept { return crend(); }

    /**
     * \return Constant iterator to the first element.
     */
    const_iterator cbegin() const noexcept { return const_iterator(this, front_); }

    /**
     * \return Constant iterator to the next after the last element.
     */
    const_iterator cend() const noexcept { return const_iterator(this); }

    /**
     * Reverse iterator pointing to the last element.
     */
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }

    /**
     * Reverse iterator pointing to the element before the first one.
     */
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

    /**
     * \param element Valid pointer to an element of the list.
     *
     * \return Iterator pointing to the given element.
     */
    iterator get_iterator(const_pointer element) noexcept {
        assert(element != nullptr);
        return iterator(this, const_cast<pointer>(element));
    }

    /**
     * \param element Valid pointer to an element of the list.
     *
     * \return Constant iterator pointing to the given element.
     */
    const_iterator get_iterator(const_pointer element) const noexcept {
        assert(element != nullptr);
        return const_iterator(this, element);
    }

    /**
     * \return Number of elements in the list.
     *
     * \warning This function takes O(N) time to compute.
     */
    size_type size() const noexcept {
        return std::distance(begin(), end());
    }

private:
    /**
     * Deleter for the list elements.
     */
    deleter_type deleter_;
};

} // namespace nc

/* vim:set et sts=4 sw=4: */
