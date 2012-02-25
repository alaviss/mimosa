#ifndef MIMOSA_CONTAINER_INTRUSIVE_DLIST_HH
# define MIMOSA_CONTAINER_INTRUSIVE_DLIST_HH

# include <cassert>
# include <cstddef>

# include "../non-copyable.hh"

namespace mimosa
{
  namespace container
  {
    template <typename Ptr>
    class IntrusiveDListHook
    {
    public:
      inline IntrusiveDListHook() : next_(nullptr), prev_(nullptr) {}
      inline IntrusiveDListHook(const IntrusiveDListHook<Ptr> &) : next_(nullptr), prev_(nullptr) {}
      inline IntrusiveDListHook<Ptr> & operator=(const IntrusiveDListHook<Ptr> &) { return *this; }

      Ptr next_;
      Ptr prev_;
    };

    template <typename T, typename Ptr, IntrusiveDListHook<Ptr> T::*Member>
    class IntrusiveDList;

    template <typename T, typename Ptr, IntrusiveDListHook<Ptr> T::*Member>
    class IntrusiveDListIterator
    {
    public:
      inline IntrusiveDListIterator(const IntrusiveDList<T, Ptr, Member> & dlist, Ptr item)
        : dlist_(dlist), item_(item)
      {
      }

      inline T & operator*() const { return *item_; }
      inline Ptr operator->() const { return item_; }
      inline IntrusiveDListIterator<T, Ptr, Member> & operator++()
      {
        assert(item_);
        item_ = (item_->*Member).next_;
        return *this;
      }

      inline IntrusiveDListIterator<T, Ptr, Member> & operator--()
      {
        assert(item_);
        item_ = (item_->*Member).prev_;
        return *this;
      }

      inline bool operator==(const IntrusiveDListIterator<T, Ptr, Member> & other) const
      {
        return &dlist_ == &other.dlist_ && item_ == other.item_;
      }

      inline bool operator!=(const IntrusiveDListIterator<T, Ptr, Member> & other) const
      {
        return !(*this == other);
      }

    private:
      const IntrusiveDList<T, Ptr, Member> & dlist_;
      Ptr                                    item_;
    };

    template <typename T, typename Ptr, IntrusiveDListHook<Ptr> T::*Member>
    class IntrusiveDList : private NonCopyable
    {
    public:
      typedef IntrusiveDListIterator<T, Ptr, Member> iterator;
      friend class IntrusiveDListIterator<T, Ptr, Member>;

      inline IntrusiveDList() : head_(nullptr), tail_(nullptr), size_(0) {}
      inline ~IntrusiveDList()
      {
        while (!empty())
          popBack();
      }

      inline bool empty() const
      {
        return !head_;
      }

      inline size_t size() const
      {
        return size_;
      }

      inline void pushFront(Ptr item)
      {
        ++size_;

        if (head_)
          (head_->*Member).prev_ = item;
        else
          tail_ = item;
        (item->*Member).next_ = head_;
        (item->*Member).prev_ = nullptr;
        head_ = item;
      }

      inline void pushBack(Ptr item)
      {
        ++size_;

        if (tail_)
          (tail_->*Member).next_ = item;
        else
          head_ = item;
        (item->*Member).prev_ = tail_;
        (item->*Member).next_ = nullptr;
        tail_ = item;
      }

      inline Ptr front() const
      {
        assert(head_);
        return head_;
      }

      inline Ptr back() const
      {
        assert(tail_);
        return tail_;
      }

      inline void popFront()
      {
        if (!head_)
          return;

        --size_;

        if (head_ == tail_)
        {
          head_ = nullptr;
          tail_ = nullptr;
          return;
        }

        head_ = (head_->*Member).next_;
        (head_->*Member).prev_ = nullptr;
      }

      inline void popBack()
      {
        if (!tail_)
          return;

        --size_;

        if (head_ == tail_)
        {
          head_ = nullptr;
          tail_ = nullptr;
          return;
        }

        tail_ = (tail_->*Member).prev_;
        (tail_->*Member).next_ = nullptr;
      }

      inline void clear()
      {
        // poping everything to unlink (and potentially release while unref)
        while (!empty())
          popBack();
      }

      inline void erase(Ptr item)
      {
        if ((item->*Member).prev_)
          ((item->*Member).prev_->*Member).next_ = (item->*Member).next_;
        else
          head_ = (item->*Member).prev_;

        if ((item->*Member).next_)
          ((item->*Member).next_->*Member).prev_ = (item->*Member).prev_;
        else
          tail_ = (item->*Member).next_;
      }

      iterator begin() const { return empty() ? end() : iterator(*this, head_); }
      iterator end() const { return iterator(*this, nullptr); }

    private:
      Ptr    head_;
      Ptr    tail_;
      size_t size_;
    };
  }
}

#endif /* !MIMOSA_CONTAINER_INTRUSIVE_DLIST_HH */
