/*
 * This file is part of linq, a header-only LINQ library for C++.
 *
 * Licensed under the Apache 2.0 license.
 * For details, see the README file.
 *
 * Copyright (c) 2015-2024 Cemalettin Dervis
 * https://dervis.de
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <string>
#include <type_traits>
#include <vector>

#if __cplusplus >= 201703L
#include <optional>
#endif

#ifdef __cpp_lib_span
#include <span>
#endif

#ifdef __cpp_lib_concepts
#include <concepts>
#endif

#if __cplusplus >= 201703L
#define LINQ_NODISCARD [[nodiscard]]
#else
#define LINQ_NODISCARD
#endif

namespace linq {
/**
 * Defines a direction for sorting ranges.
 */
enum class sort_direction {
  /**
   * Sort elements in an ascending order.
   */
  ascending,

  /**
   * Sort elements in a descending order.
   */
  descending
};

namespace details {
// ----------------------------------
// Range declarations
// ----------------------------------

template <typename TPrevRange, typename TPredicate>
class where_range;

template <typename TPrevRange>
class distinct_range;

template <typename TPrevRange, typename TTransform>
class select_range;

template <typename TPrevRange>
class select_to_string_range;

template <typename TPrevRange, typename TTransform>
class select_many_range;

template <typename TPrevRange>
class reverse_range;

template <typename TPrevRange>
class take_range;

template <typename TPrevRange, typename TPredicate>
class take_while_range;

template <typename TPrevRange>
class skip_range;

template <typename TPrevRange, typename TPredicate>
class skip_while_range;

template <typename TPrevRange, typename TOtherRange>
class append_range;

template <typename TPrevRange>
class repeat_range;

template <typename TPrevRange,
          typename TOtherRange,
          typename TKeySelectorA,
          typename TKeySelectorB,
          typename TTransform>
class join_range;

template <typename TPrevRange, typename TKeySelector>
class order_by_range;

template <typename TPrevRange, typename TKeySelector>
class then_by_range;

// ----------------------------------
// Average calculators
// ----------------------------------

#ifdef __cpp_lib_concepts
template <typename T>
concept number = std::integral<T> || std::floating_point<T>;

template <typename T>
concept averageable = requires(T a, T b, size_t c) {
  { a / c } -> std::same_as<T>;
  a += b;
};
#endif

template <typename T, typename TRange>
#ifdef __cpp_lib_concepts
  requires(averageable<T> || number<T>)
#endif
#ifdef __cpp_lib_optional
static auto calculate_average(const TRange& op) {
  using float_t  = long double;
  using output_t = typename TRange::output_t;

#ifdef __cpp_lib_concepts
  using return_t = std::conditional_t<number<output_t>, float_t, output_t>;
#else
  using return_t =
      std::conditional_t<std::is_integral<output_t>::value || std::is_floating_point_v<output_t>, float_t, output_t>;
#endif

  const auto maybe_sum_and_count = op.sum_and_count();

  if (maybe_sum_and_count) {
    const auto& sum_and_count = *maybe_sum_and_count;
    return std::optional<return_t>{static_cast<return_t>(sum_and_count.first) / sum_and_count.second};
  }

  return std::optional<return_t>{};
}
#else
#error "not implemented"
#endif

// ----------------------------------
// base_range
// ----------------------------------

// Base class to identify all range types (for compile-time type checking).
class base_range_ident {
  /* Nothing to define here. */
};

// Base class for sorting ranges (for compile-time type checking).
class sorting_range {
  /* Nothing to define here. */
};

/**
 * @brief Represents the base class of all LINQ ranges.
 * @tparam TMy The fully qualified type of the range.
 * @tparam TOutput The full, unmodified type that is returned by the range.
 */
template <typename TMy, typename TOutput>
class base_range : public base_range_ident {
public:
  // Return non-const, non-volatile, non-reference types from methods such as sum, min and max.
  using output_t = std::decay_t<TOutput>;

  /**
   * @brief Appends a filter to the range.
   * @tparam TPredicate The type of the predicate: f(x) -> bool
   * @param predicate The predicate
   * @return A new range that combines this range with the where-range.
   */
  template <typename TPredicate>
  LINQ_NODISCARD auto where(const TPredicate& predicate) const;

  /**
   * @brief Appends a distinct-filter to the range that removes duplicate elements.
   * @return A new range that combines this range with the distinct-range.
   */
  LINQ_NODISCARD auto distinct() const;

  template <typename TTransform>
  LINQ_NODISCARD auto select(const TTransform& transform) const;

  LINQ_NODISCARD auto select_to_string() const;

  template <typename TTransform>
  LINQ_NODISCARD auto select_many(const TTransform& transform) const;

  LINQ_NODISCARD auto reverse() const;

  LINQ_NODISCARD auto take(size_t count) const;

  template <typename TPredicate>
  LINQ_NODISCARD auto take_while(const TPredicate& predicate) const;

  LINQ_NODISCARD auto skip(size_t count) const;

  template <typename TPredicate>
  LINQ_NODISCARD auto skip_while(const TPredicate& predicate) const;

  template <typename TOtherRange>
  LINQ_NODISCARD auto append(const TOtherRange& other_range) const;

  LINQ_NODISCARD auto repeat(size_t count) const;

  template <typename TOtherRange, typename TKeySelectorA, typename TKeySelectorB, typename TTransform>
  LINQ_NODISCARD auto join(const TOtherRange&   other_range,
                           const TKeySelectorA& key_selector_a,
                           const TKeySelectorB& key_selector_b,
                           const TTransform&    transform) const;

  template <typename TKeySelector>
  LINQ_NODISCARD auto order_by(const TKeySelector& key_selector, sort_direction sort_dir) const;

  template <typename TKeySelector>
  LINQ_NODISCARD auto order_by_ascending(const TKeySelector& key_selector) const {
    return order_by<TKeySelector>(key_selector, sort_direction::ascending);
  }

  template <typename TKeySelector>
  LINQ_NODISCARD auto order_by_descending(const TKeySelector& key_selector) const {
    return order_by<TKeySelector>(key_selector, sort_direction::descending);
  }

  template <typename TKeySelector>
  LINQ_NODISCARD auto then_by(const TKeySelector& key_selector, sort_direction sort_dir) const;

  template <typename TKeySelector>
  LINQ_NODISCARD auto then_by_ascending(const TKeySelector& key_selector) const {
    return then_by<TKeySelector>(key_selector, sort_direction::ascending);
  }

  template <typename TKeySelector>
  LINQ_NODISCARD auto then_by_descending(const TKeySelector& key_selector) const {
    return then_by<TKeySelector>(key_selector, sort_direction::descending);
  }

  LINQ_NODISCARD auto sum() const;
  LINQ_NODISCARD auto min() const;
  LINQ_NODISCARD auto max() const;

  LINQ_NODISCARD auto sum_and_count() const;

  LINQ_NODISCARD auto average() const
#ifdef __cpp_lib_concepts
    requires(averageable<output_t> || number<output_t>)
#endif
  ;

  template <typename TAccumFunc>
  LINQ_NODISCARD auto aggregate(const TAccumFunc& func) const;

  LINQ_NODISCARD auto first() const;

  LINQ_NODISCARD auto first_or(output_t&& default_value) const;

  template <typename TPredicate>
  LINQ_NODISCARD auto first(const TPredicate& predicate) const;

  template <typename TPredicate>
  LINQ_NODISCARD auto first_or(const TPredicate& predicate, output_t&& default_value) const;

  LINQ_NODISCARD auto last() const;

  LINQ_NODISCARD auto last_or(output_t&& default_value) const;

  template <typename TPredicate>
  LINQ_NODISCARD auto last(const TPredicate& predicate) const;

  template <typename TPredicate>
  LINQ_NODISCARD auto last_or(const TPredicate& predicate, output_t&& default_value) const;

  template <typename TPredicate>
  LINQ_NODISCARD bool any(const TPredicate& predicate) const;

  template <typename TPredicate>
  LINQ_NODISCARD bool all(const TPredicate& predicate) const;

  template <typename TPredicate>
  LINQ_NODISCARD bool none(const TPredicate& predicate) const;

  LINQ_NODISCARD size_t count() const;

  template <typename TPredicate>
  LINQ_NODISCARD size_t count(const TPredicate& predicate) const;

  LINQ_NODISCARD auto element_at(size_t index, output_t&& default_value) const;

  LINQ_NODISCARD auto to_vector() const;
};

// ----------------------------------
// where
// ----------------------------------

template <typename TPrevRange, typename TPredicate>
class where_range : public base_range<where_range<TPrevRange, TPredicate>, typename TPrevRange::iterator::output_t> {
public:
  struct iterator {
    using prev_iter_t = typename TPrevRange::iterator;
    using output_t    = typename prev_iter_t::output_t;

    iterator(const where_range* parent, prev_iter_t begin, prev_iter_t end)
        : m_parent(parent)
        , m_begin(begin)
        , m_end(end) {
      const auto& pred = *m_parent->m_predicate;

      // Seek the first match.
      while (m_begin != m_end && !pred(*m_begin)) {
        ++m_begin;
      }
    }

    bool operator==(const iterator& o) const {
      return m_begin == o.m_begin;
    }

    bool operator!=(const iterator& o) const {
      return m_begin != o.m_begin;
    }

    iterator& operator++() {
      const auto& pred = *m_parent->m_predicate;

      do {
        ++m_begin;
      } while (m_begin != m_end && !pred(*m_begin));

      return *this;
    }

    output_t operator*() const {
      return *m_begin;
    }

    const where_range* m_parent;
    prev_iter_t        m_begin;
    prev_iter_t        m_end;
  };

  where_range() = default;

  where_range(const TPrevRange& prev, const TPredicate& predicate)
      : m_prev(prev)
      , m_predicate(&predicate) {
  }

  iterator begin() const {
    return iterator(this, m_prev.begin(), m_prev.end());
  }

  iterator end() const {
    const auto prev_end = m_prev.end();
    return iterator(this, prev_end, prev_end);
  }

private:
  TPrevRange        m_prev;
  const TPredicate* m_predicate;
};

// ----------------------------------
// distinct
// ----------------------------------

template <typename TPrevRange>
class distinct_range : public base_range<distinct_range<TPrevRange>, typename TPrevRange::iterator::output_t> {
  using prev_iter_t      = typename TPrevRange::iterator;
  using object_container = std::vector<prev_iter_t>;

public:
  struct iterator {
    using output_t = typename prev_iter_t::output_t;

    iterator(prev_iter_t begin, prev_iter_t end, object_container* encountered_objects)
        : m_begin(begin)
        , m_end(end)
        , m_encountered_objects(encountered_objects) {
      if (m_begin != m_end) {
        encountered_objects->clear();
        encountered_objects->push_back(m_begin);
      }
    }

    bool operator==(const iterator& o) const {
      return m_begin == o.m_begin;
    }

    bool operator!=(const iterator& o) const {
      return m_begin != o.m_begin;
    }

    iterator& operator++() {
      do {
        ++m_begin;
      } while (m_begin != m_end && ontains_object(m_begin));

      if (m_begin != m_end)
        m_encountered_objects->push_back(m_begin);

      return *this;
    }

    bool contains_object(const prev_iter_t& it) {
      const auto& it_val = *it;

      for (size_t i = 0; i < m_encountered_objects->size(); ++i) {
        if (*m_encountered_objects->at(i) == it_val) {
          return true;
        }
      }

      return false;
    }

    output_t operator*() const {
      return *m_begin;
    }

    prev_iter_t       m_begin;
    prev_iter_t       m_end;
    object_container* m_encountered_objects;
  };

public:
  distinct_range() = default;

  explicit distinct_range(const TPrevRange& prev)
      : m_prev(prev) {
  }

  iterator begin() const {
    return iterator(m_prev.begin(), m_prev.end(), &m_encountered_objects);
  }

  iterator end() const {
    const auto prev_end = m_prev.end();
    return iterator(prev_end, &m_encountered_objects);
  }

private:
  TPrevRange               m_prev;
  mutable object_container m_encountered_objects;
};

// ----------------------------------
// select
// ----------------------------------

// Resolves the arguments that are passed to a select functor.
template <typename TPrevRange>
using select_transform_arg_t = std::add_const_t<std::add_lvalue_reference_t<typename TPrevRange::iterator::output_t>>;

// Resolves the return type of a select functor.
template <typename TPrevRange, typename TTransform>
using select_output_t = std::invoke_result_t<TTransform, select_transform_arg_t<TPrevRange>>;

template <typename TPrevRange, typename TTransform>
class select_range : public base_range<select_range<TPrevRange, TTransform>, select_output_t<TPrevRange, TTransform>> {
public:
  struct iterator {
    using prev_iter_t = typename TPrevRange::iterator;
    using output_t    = select_output_t<TPrevRange, TTransform>;

    iterator(const select_range* parent, prev_iter_t begin, prev_iter_t end)
        : m_parent(parent)
        , m_begin(begin)
        , m_end(end) {
    }

    bool operator==(const iterator& o) const {
      return m_begin == o.m_begin;
    }

    bool operator!=(const iterator& o) const {
      return m_begin != o.m_begin;
    }

    iterator& operator++() {
      ++m_begin;
      return *this;
    }

    output_t operator*() const {
      const auto& transform = *m_parent->m_transform;
      return transform(*m_begin);
    }

    const select_range* m_parent;
    prev_iter_t         m_begin;
    prev_iter_t         m_end;
  };

  select_range(const TPrevRange& prev, const TTransform& transform)
      : m_prev(prev)
      , m_transform(&transform) {
  }

  iterator begin() const {
    return iterator(this, m_prev.begin(), m_prev.end());
  }

  iterator end() const {
    const auto prev_end = m_prev.end();
    return iterator(this, prev_end, prev_end);
  }

private:
  TPrevRange        m_prev;
  const TTransform* m_transform{};
};

// ----------------------------------
// select_to_string
// ----------------------------------

template <typename TPrevRange>
class select_to_string_range : public base_range<select_to_string_range<TPrevRange>, std::string> {
public:
  struct iterator {
    using prev_iter_t = typename TPrevRange::iterator;
    using output_t    = std::string;

    iterator(const select_to_string_range* parent, prev_iter_t begin, prev_iter_t end)
        : m_parent(parent)
        , m_begin(begin)
        , m_end(end) {
    }

    bool operator==(const iterator& o) const {
      return m_begin == o.m_begin;
    }

    bool operator!=(const iterator& o) const {
      return m_begin != o.m_begin;
    }

    iterator& operator++() {
      ++m_begin;
      return *this;
    }

    output_t operator*() const {
      return std::to_string(*m_begin);
    }

    const select_to_string_range* m_parent{};
    prev_iter_t                   m_begin;
    prev_iter_t                   m_end;
  };

  explicit select_to_string_range(const TPrevRange& prev)
      : m_prev(prev) {
  }

  iterator begin() const {
    return iterator{this, m_prev.begin(), m_prev.end()};
  }

  iterator end() const {
    const auto prev_end = m_prev.end();
    return iterator{this, prev_end, prev_end};
  }

private:
  TPrevRange m_prev;
};

// ----------------------------------
// select_many
// ----------------------------------

template <typename TPrevRange, typename TTransform>
struct select_many_traits {
  using returned_range_t = std::invoke_result_t<TTransform, typename TPrevRange::iterator::output_t>;

  // See if the returned type is really a range.
  static_assert(std::is_base_of_v<base_range_ident, returned_range_t>,
                "The transform function of select_many is expected to return a LINQ range.");

  using output_t = typename returned_range_t::iterator::output_t;
};

template <typename TPrevRange, typename TTransform>
class select_many_range : public base_range<select_many_range<TPrevRange, TTransform>,
                                            typename select_many_traits<TPrevRange, TTransform>::output_t> {
public:
  struct iterator {
    using prev_iter_t = typename TPrevRange::iterator;

    using returned_range_t      = typename select_many_traits<TPrevRange, TTransform>::returned_range_t;
    using returned_range_iter_t = typename returned_range_t::iterator;
    using output_t              = typename returned_range_iter_t::output_t;

    iterator(const select_many_range* parent, prev_iter_t pos, prev_iter_t end)
        : m_parent(parent)
        , m_pos(pos)
        , m_end(end) {
      if (m_pos != m_end) {
        const auto& transform = m_parent->m_transform;
        bool        first{true};

        do {
          if (!first) {
            ++m_pos;
          }

          if (m_pos == m_end) {
            break;
          }

          m_ret_range = transform(*m_pos);
          m_ret_begin = m_ret_range.begin();
          m_ret_end   = m_ret_range.end();

          first = false;
        } while (m_ret_begin == m_ret_end);
      }
    }

    bool operator==(const iterator& o) const {
      return m_pos == o.m_pos;
    }

    bool operator!=(const iterator& o) const {
      return m_pos != o.m_pos;
    }

    iterator& operator++() {
      if (m_ret_begin != m_ret_end) {
        // There are values left to be obtained from the returned range.
        ++m_ret_begin;
      }

      if (m_ret_begin == m_ret_end) {
        // Move our parent iterator forward to get the next container.
        ++m_pos;
        if (m_pos != m_end) {
          const auto& transform = m_parent->Transform;
          m_ret_range           = transform(*m_pos);
          m_ret_begin           = m_ret_range.begin();
          m_ret_end             = m_ret_range.end();
        }
      }

      return *this;
    }

    output_t operator*() const {
      return *m_ret_begin;
    }

    const select_many_range* m_parent;
    prev_iter_t              m_pos;
    prev_iter_t              m_end;

    returned_range_t      m_ret_range;
    returned_range_iter_t m_ret_begin;
    returned_range_iter_t m_ret_end;
  };

  select_many_range() = default;

  select_many_range(const TPrevRange& prev, TTransform transform)
      : m_prev(prev)
      , m_transform(std::move(transform)) {
  }

  iterator begin() const {
    return iterator{this, m_prev.begin(), m_prev.end()};
  }

  iterator end() const {
    const auto prev_end = m_prev.end();
    return iterator{this, prev_end, prev_end};
  }

private:
  TPrevRange m_prev;
  TTransform m_transform;
};

// ----------------------------------
// reverse
// ----------------------------------

template <typename TPrevRange>
class reverse_range : public base_range<reverse_range<TPrevRange>, typename TPrevRange::iterator::output_t> {
public:
  using prev_iter_t      = typename TPrevRange::iterator;
  using object_container = std::vector<prev_iter_t>;

  struct iterator {
    using output_t = typename prev_iter_t::output_t;

    iterator(const object_container* prev_iterators, size_t index)
        : m_prev_iterators(prev_iterators)
        , m_index(index) {
    }

    bool operator==(const iterator& o) const {
      return m_index == o.m_index;
    }

    bool operator!=(const iterator& o) const {
      return m_index != o.m_index;
    }

    iterator& operator++() {
      --m_index;
      return *this;
    }

    output_t operator*() const {
      return *m_prev_iterators->at(m_index);
    }

    const object_container* m_prev_iterators;
    size_t                  m_index{};
  };

  reverse_range() = default;

  explicit reverse_range(const TPrevRange& prev)
      : m_prev(prev) {
  }

  iterator begin() const {
    m_prev_iterators.clear();

    for (auto beg = m_prev.begin(), end = m_prev.end(); beg != end; ++beg) {
      m_prev_iterators.push_back(beg);
    }

    return iterator{std::addressof(m_prev_iterators), m_prev_iterators.size() - 1};
  }

  iterator end() const {
    return iterator{nullptr, -1};
  }

private:
  TPrevRange               m_prev;
  mutable object_container m_prev_iterators;
};

// ----------------------------------
// take
// ----------------------------------

template <typename TPrevRange>
class take_range : public base_range<take_range<TPrevRange>, typename TPrevRange::iterator::output_t> {
public:
  struct iterator {
    using prev_iter_t = typename TPrevRange::iterator;
    using output_t    = typename prev_iter_t::output_t;

    iterator(prev_iter_t begin, size_t count)
        : m_begin(begin)
        , m_count(count) {
    }

    bool operator==(const iterator& o) const {
      return m_count == o.m_count || m_begin == o.m_begin;
    }

    bool operator!=(const iterator& o) const {
      return m_count != o.m_count && m_begin != o.m_begin;
    }

    iterator& operator++() {
      ++m_begin;
      --m_count;
      return *this;
    }

    output_t operator*() const {
      return *m_begin;
    }

    prev_iter_t m_begin;
    size_t      m_count{};
  };

  take_range() = default;

  take_range(const TPrevRange& prev, size_t count)
      : m_prev(prev)
      , m_count(count) {
  }

  iterator begin() const {
    return iterator(m_prev.begin(), m_count);
  }

  iterator end() const {
    return iterator(m_prev.end(), 0);
  }

private:
  TPrevRange m_prev;
  size_t     m_count{};
};

// ----------------------------------
// take_while
// ----------------------------------

template <typename TPrevRange, typename TPredicate>
class take_while_range
    : public base_range<take_while_range<TPrevRange, TPredicate>, typename TPrevRange::iterator::output_t> {
public:
  struct iterator {
    using prev_iter_t = typename TPrevRange::iterator;
    using output_t    = typename prev_iter_t::output_t;

    iterator(const take_while_range* parent, prev_iter_t begin, prev_iter_t end)
        : m_parent(parent)
        , m_begin(begin)
        , m_end(end) {
      const auto& pred = m_parent->m_predicate;

      if (m_begin != m_end && !pred(*m_begin)) {
        m_begin = m_end;
      }
    }

    bool operator==(const iterator& o) const {
      return m_begin == o.m_begin;
    }

    bool operator!=(const iterator& o) const {
      return m_begin != o.m_begin;
    }

    iterator& operator++() {
      ++m_begin;

      const auto& pred = m_parent->m_predicate;

      if (m_begin != m_end && !pred(*m_begin)) {
        m_begin = m_end;
      }

      return *this;
    }

    output_t operator*() const {
      return *m_begin;
    }

    const take_while_range* m_parent;
    prev_iter_t             m_begin;
    prev_iter_t             m_end;
  };

  take_while_range() = default;

  take_while_range(const TPrevRange& prev, TPredicate predicate)
      : m_prev(prev)
      , m_predicate(std::move(predicate)) {
  }

  iterator begin() const {
    return iterator(this, m_prev.begin(), m_prev.end());
  }

  iterator end() const {
    return iterator(this, m_prev.end(), m_prev.end());
  }

private:
  TPrevRange m_prev;
  TPredicate m_predicate;
};

// ----------------------------------
// skip
// ----------------------------------

template <typename TPrevRange>
class skip_range : public base_range<skip_range<TPrevRange>, typename TPrevRange::iterator::output_t> {
public:
  struct iterator {
    using prev_iter_t = typename TPrevRange::iterator;
    using output_t    = typename prev_iter_t::output_t;

    iterator(prev_iter_t begin, prev_iter_t end, size_t count)
        : m_begin(begin) {
      while (m_begin != end && count > 0) {
        ++m_begin;
        --count;
      }
    }

    bool operator==(const iterator& o) const {
      return m_begin == o.m_begin;
    }

    bool operator!=(const iterator& o) const {
      return m_begin != o.m_begin;
    }

    iterator& operator++() {
      ++m_begin;
      return *this;
    }

    output_t operator*() const {
      return *m_begin;
    }

    prev_iter_t m_begin;
  };

  skip_range(const TPrevRange& prev, size_t count)
      : m_prev(prev)
      , m_count(count) {
  }

  iterator begin() const {
    return iterator(m_prev.begin(), m_prev.end(), m_count);
  }

  iterator end() const {
    const auto prev_end = m_prev.end();
    return iterator(prev_end, prev_end, 0);
  }

private:
  TPrevRange m_prev;
  size_t     m_count{};
};

// ----------------------------------
// skip_while
// ----------------------------------

template <typename TPrevRange, typename TPredicate>
class skip_while_range
    : public base_range<skip_while_range<TPrevRange, TPredicate>, typename TPrevRange::iterator::output_t> {
public:
  struct iterator {
    using prev_iter_t = typename TPrevRange::iterator;
    using output_t    = typename prev_iter_t::output_t;

    iterator(prev_iter_t begin, prev_iter_t end, const TPredicate& predicate)
        : Begin(begin) {
      while (Begin != end && predicate(*Begin))
        ++Begin;
    }

    bool operator==(const iterator& o) const {
      return Begin == o.Begin;
    }
    bool operator!=(const iterator& o) const {
      return Begin != o.Begin;
    }

    iterator& operator++() {
      ++Begin;
      return *this;
    }

    output_t operator*() const {
      return *Begin;
    }

    prev_iter_t Begin;
  };

  skip_while_range(const TPrevRange& prev, const TPredicate& predicate)
      : m_prev(prev)
      , m_predicate(&predicate) {
  }

  iterator begin() const {
    return iterator(m_prev.begin(), m_prev.end(), *m_predicate);
  }

  iterator end() const {
    const auto prev_end = m_prev.end();
    return iterator(prev_end, prev_end, *m_predicate);
  }

private:
  TPrevRange        m_prev;
  const TPredicate* m_predicate{};
};

// ----------------------------------
// append
// ----------------------------------

template <typename TPrevRange, typename TOtherRange>
class append_range : public base_range<append_range<TPrevRange, TOtherRange>, typename TPrevRange::iterator::output_t> {
public:
  using other_range_iter_t = typename TOtherRange::iterator;

  struct iterator {
    using prev_iter_t = typename TPrevRange::iterator;
    using output_t    = typename prev_iter_t::output_t;

    iterator(prev_iter_t begin, prev_iter_t end, other_range_iter_t other_begin, other_range_iter_t other_end)
        : m_my_begin(begin)
        , m_my_end(end)
        , m_other_begin(other_begin)
        , m_other_end(other_end) {
    }

    bool operator==(const iterator& o) const {
      return m_other_begin == o.m_other_begin;
    }

    bool operator!=(const iterator& o) const {
      return m_other_begin != o.m_other_begin;
    }

    iterator& operator++() {
      if (m_my_begin != m_my_end) {
        ++m_my_begin;
      }
      else {
        ++m_other_begin;
      }

      return *this;
    }

    output_t operator*() const {
      return (m_my_begin != m_my_end) ? *m_my_begin : *m_other_begin;
    }

    prev_iter_t        m_my_begin;
    prev_iter_t        m_my_end;
    other_range_iter_t m_other_begin;
    other_range_iter_t m_other_end;
  };

  append_range(const TPrevRange& prev, const TOtherRange& other_range)
      : m_prev(prev)
      , m_other_range(other_range) {
  }

  iterator begin() const {
    return iterator(m_prev.begin(), m_prev.end(), m_other_range.begin(), m_other_range.end());
  }

  iterator end() const {
    const auto prev_end  = m_prev.end();
    const auto other_end = m_other_range.end();
    return iterator(prev_end, prev_end, other_end, other_end);
  }

private:
  TPrevRange  m_prev;
  TOtherRange m_other_range;
};

// ----------------------------------
// repeat
// ----------------------------------

template <typename TPrevRange>
class repeat_range : public base_range<repeat_range<TPrevRange>, typename TPrevRange::iterator::output_t> {
public:
  struct iterator {
    using prev_iter_t = typename TPrevRange::iterator;
    using output_t    = typename prev_iter_t::output_t;

    iterator(TPrevRange* prev_range_ptr, prev_iter_t begin, prev_iter_t end, size_t count)
        : m_prev_range_ptr(prev_range_ptr)
        , m_pos(begin)
        , m_end(end)
        , m_count(count) {
    }

    bool operator==(const iterator& o) const {
      return m_pos == o.m_pos;
    }

    bool operator!=(const iterator& o) const {
      return m_pos != o.m_pos;
    }

    iterator& operator++() {
      ++m_pos;

      if (m_pos == m_end && m_count > 0) {
        m_pos = m_prev_range_ptr->begin();
        --m_count;
      }

      return *this;
    }

    output_t operator*() const {
      return *m_pos;
    }

    TPrevRange* m_prev_range_ptr{};
    prev_iter_t m_pos{};
    prev_iter_t m_end{};
    size_t      m_count{};
  };

  repeat_range(const TPrevRange& prev, size_t count)
      : m_prev(prev)
      , m_count(count) {
  }

  iterator begin() const {
    return iterator(&m_prev, m_prev.begin(), m_prev.end(), m_count);
  }

  iterator end() const {
    const auto prev_end = m_prev.end();
    return iterator(&m_prev, prev_end, prev_end, 0);
  }

private:
  mutable TPrevRange m_prev;
  size_t             m_count;
};

// ----------------------------------
// join
// ----------------------------------

// Determines the output of a selection in the join range.
template <typename TRangeA, typename TRangeB, typename TTransform>
using join_output_t =
    std::invoke_result_t<TTransform(typename TRangeA::iterator::output_t, typename TRangeB::iterator::output_t)>;

// Nested loop inner join operator.
template <typename TPrevRange,
          typename TOtherRange,
          typename TKeySelectorA,
          typename TKeySelectorB,
          typename TTransform>
class join_range : public base_range<join_range<TPrevRange, TOtherRange, TKeySelectorA, TKeySelectorB, TTransform>,
                                     join_output_t<TPrevRange, TOtherRange, TTransform>> {
  using other_range_iter_t = typename TOtherRange::iterator;

public:
  struct iterator {
    using prev_iter_t = typename TPrevRange::iterator;
    using output_t    = join_output_t<TPrevRange, TOtherRange, TTransform>;

    iterator(prev_iter_t begin, prev_iter_t end, const join_range* parent)
        : m_begin(begin)
        , m_end(end)
        , m_pos(begin)
        , m_other_begin(parent->m_other_range.begin())
        , m_other_end(parent->m_other_range.end())
        , m_other_pos(parent->m_other_range.begin())
        , m_parent(parent) {
      // Find the first match without pre-incrementing the
      // other position, so that we start at the beginning.
      find_next(false);
    }

    bool operator==(const iterator& o) const {
      return m_pos == o.m_pos;
    }
    bool operator!=(const iterator& o) const {
      return m_pos != o.m_pos;
    }

    iterator& operator++() {
      // Find the next match, but pre-increment the other
      // position, so that we can move forward.
      find_next(true);
      return *this;
    }

    output_t operator*() const {
      const auto& transform = *m_parent->Transform;
      return transform(*m_pos, *m_other_pos);
    }

    prev_iter_t m_begin;
    prev_iter_t m_end;
    prev_iter_t m_pos;

    other_range_iter_t m_other_begin;
    other_range_iter_t m_other_end;
    other_range_iter_t m_other_pos;

    const join_range* m_parent;

  private:
    // Finds the next match in both ranges using the key selectors and == comparison.
    void find_next(bool pre_increment_other) {
      const auto& key_selector_a = *m_parent->KeySelectorA;
      const auto& key_selector_b = *m_parent->KeySelectorB;

      if (pre_increment_other) {
        ++m_other_pos;
      }

      while (m_pos != m_end) {
        bool        should_continue = true;
        const auto& key_a           = key_selector_a(*m_pos);

        while (m_other_pos != m_other_end) {
          const auto& key_b = key_selector_b(*m_other_pos);

          if (key_a == key_b) {
            should_continue = false;
            break;
          }

          ++m_other_pos;
        }

        // Start over in the other range if it's finished.
        if (m_other_pos == m_other_end)
          m_other_pos = m_other_begin;

        if (!should_continue) {
          break;
        }

        ++m_pos;
      }
    }
  };

  join_range(const TPrevRange&    prev,
             TOtherRange          other_range,
             const TKeySelectorA& key_selector_a,
             const TKeySelectorB& key_selector_b,
             const TTransform&    transform)
      : m_prev(prev)
      , m_other_range(other_range)
      , m_key_selector_a(&key_selector_a)
      , m_key_selector_b(&key_selector_b)
      , m_transform(&transform) {
  }

  iterator begin() const {
    return iterator(m_prev.begin(), m_prev.end(), this);
  }

  iterator end() const {
    const auto prev_end = m_prev.end();
    return iterator(prev_end, prev_end, this);
  }

private:
  TPrevRange           m_prev;
  TOtherRange          m_other_range;
  const TKeySelectorA* m_key_selector_a;
  const TKeySelectorB* m_key_selector_b;
  const TTransform*    m_transform;
};

// ----------------------------------
// order_by
// ----------------------------------

template <typename TPrevRange, typename TKeySelector>
class order_by_range
    : public base_range<order_by_range<TPrevRange, TKeySelector>, typename TPrevRange::iterator::output_t>,
      public sorting_range {
public:
  using container_element_t = std::decay_t<typename TPrevRange::iterator::output_t>;
  using container_t         = std::vector<container_element_t>;
  using container_iter_t    = typename container_t::const_iterator;

  struct iterator {
    using output_t = typename container_iter_t::reference;

    explicit iterator(container_iter_t pos)
        : m_pos(pos) {
    }

    bool operator==(const iterator& o) const {
      return m_pos == o.m_pos;
    }

    bool operator!=(const iterator& o) const {
      return m_pos != o.m_pos;
    }

    iterator& operator++() {
      ++m_pos;
      return *this;
    }

    output_t operator*() const {
      return *m_pos;
    }

    container_iter_t m_pos;
  };

  order_by_range(const TPrevRange& prev, const TKeySelector& key_selector, sort_direction sort_dir)
      : m_prev(prev)
      , m_key_selector(&key_selector)
      , m_sort_direction(sort_dir) {
  }

  iterator begin() const {
    m_sorted_values.clear();

    for (const auto& val : m_prev) {
      m_sorted_values.push_back(val);
    }

    std::sort(m_sorted_values.begin(),
              m_sorted_values.end(),
              [this](const container_element_t& a, const container_element_t& b) { return this->compare_keys(a, b); });

    return iterator(m_sorted_values.begin());
  }

  iterator end() const {
    return iterator(m_sorted_values.end());
  }

  bool compare_keys(const container_element_t& a, const container_element_t& b) const {
    const auto& key_selector = *m_key_selector;
    const auto& a_val        = key_selector(a);
    const auto& b_val        = key_selector(b);

    return m_sort_direction == sort_direction::ascending ? /*ascending:*/ a_val < b_val : /*descending:*/ b_val < a_val;
  }

private:
  TPrevRange          m_prev;
  const TKeySelector* m_key_selector;
  sort_direction      m_sort_direction;
  mutable container_t m_sorted_values;
};

// ----------------------------------
// then_by
// ----------------------------------

template <typename TPrevRange, typename TKeySelector>
class then_by_range
    : public base_range<then_by_range<TPrevRange, TKeySelector>, typename TPrevRange::iterator::output_t>,
      public sorting_range {
  static_assert(std::is_assignable_v<sorting_range, TPrevRange>,
                "A then_by operation can only be appended to another then_by or order_by operation.");

public:
  using container_element_t = std::decay_t<typename TPrevRange::iterator::output_t>;
  using container_t         = std::vector<container_element_t>;
  using container_iter_t    = typename container_t::const_iterator;

  struct iterator {
    using output_t = typename container_iter_t::reference;

    explicit iterator(container_iter_t pos)
        : m_pos(pos) {
    }

    bool operator==(const iterator& o) const {
      return m_pos == o.m_pos;
    }

    bool operator!=(const iterator& o) const {
      return m_pos != o.m_pos;
    }

    iterator& operator++() {
      ++m_pos;
      return *this;
    }

    output_t operator*() const {
      return *m_pos;
    }

    container_iter_t m_pos;
  };

  then_by_range(const TPrevRange& prev, const TKeySelector& key_selector, sort_direction sort_dir)
      : m_prev(prev)
      , m_key_selector(&key_selector)
      , m_sort_direction(sort_dir) {
  }

  iterator begin() const {
    m_sorted_values.clear();
    for (const auto& val : m_prev)
      m_sorted_values.push_back(val);

    std::sort(m_sorted_values.begin(),
              m_sorted_values.end(),
              [this](const container_element_t& a, const container_element_t& b) { return this->compare_keys(a, b); });

    return iterator(m_sorted_values.begin());
  }

  iterator end() const {
    return iterator(m_sorted_values.end());
  }

  bool compare_keys(const container_element_t& a, const container_element_t& b) const {
    const auto& key_selector = *m_key_selector;
    const auto& a_value      = key_selector(a);
    const auto& b_value      = key_selector(b);

    if (m_prev.compare_keys(a, b)) {
      return true;
    }

    if (m_prev.compare_keys(b, a)) {
      return false;
    }

    return m_sort_direction == sort_direction::ascending ? /*ascending:*/ a_value < b_value
                                                         : /*descending:*/ b_value < a_value;
  }

private:
  TPrevRange          m_prev;
  const TKeySelector* m_key_selector;
  sort_direction      m_sort_direction;
  mutable container_t m_sorted_values;
};

// ----------------------------------
// container_range
// ----------------------------------

template <typename TContainer>
class container_range : public base_range<container_range<TContainer>, typename TContainer::value_type> {
public:
  struct iterator {
    using container_iter_t = typename TContainer::const_iterator;
    using output_t         = typename TContainer::const_reference;

    explicit iterator(container_iter_t pos)
        : m_pos(pos) {
    }

    bool operator==(const iterator& o) const {
      return m_pos == o.m_pos;
    }

    bool operator!=(const iterator& o) const {
      return m_pos != o.m_pos;
    }

    iterator& operator++() {
      ++m_pos;
      return *this;
    }

    output_t operator*() const {
      return *m_pos;
    }

    container_iter_t m_pos;
  };

  explicit container_range(const TContainer* container)
      : m_container(container) {
    assert(container != nullptr && "null container given to range");
  }

  iterator begin() const {
    return iterator(m_container->cbegin());
  }

  iterator end() const {
    return iterator(m_container->cend());
  }

private:
  const TContainer* m_container{};
};

// ----------------------------------
// mutable_container_range
// ----------------------------------

template <typename TContainer>
class mutable_container_range
    : public base_range<mutable_container_range<TContainer>, typename TContainer::value_type> {
public:
  struct iterator {
    using container_iter_t = typename TContainer::iterator;
    using output_t         = typename TContainer::reference;

    explicit iterator(container_iter_t pos)
        : m_pos(pos) {
    }

    bool operator==(const iterator& o) const {
      return m_pos == o.m_pos;
    }

    bool operator!=(const iterator& o) const {
      return m_pos != o.m_pos;
    }

    iterator& operator++() {
      ++m_pos;
      return *this;
    }

    output_t operator*() const {
      return *m_pos;
    }

    container_iter_t m_pos;
  };

  explicit mutable_container_range(TContainer* container)
      : m_container(container) {
    assert(container != nullptr && "null container given to range");
  }

  iterator begin() const {
    return iterator(m_container->begin());
  }

  iterator end() const {
    return iterator(m_container->end());
  }

private:
  TContainer* m_container{};
};

// ----------------------------------
// container_copy_range
// ----------------------------------

template <typename TContainer>
class container_copy_range : public base_range<container_copy_range<TContainer>, typename TContainer::value_type> {
public:
  struct iterator {
    using container_iter_t = typename TContainer::iterator;
    using output_t         = typename TContainer::reference;

    explicit iterator(container_iter_t pos)
        : m_pos(pos) {
    }

    bool operator==(const iterator& o) const {
      return m_pos == o.m_pos;
    }

    bool operator!=(const iterator& o) const {
      return m_pos != o.m_pos;
    }

    iterator& operator++() {
      ++m_pos;
      return *this;
    }

    output_t operator*() const {
      return *m_pos;
    }

    container_iter_t m_pos;
  };

  explicit container_copy_range(const TContainer& container)
      : m_container(container) {
  }

  iterator begin() const {
    return iterator{m_container.begin()};
  }

  iterator end() const {
    return iterator{m_container.end()};
  }

private:
  TContainer m_container{};
};

// ----------------------------------
// from_initializer_list
// ----------------------------------

template <typename T, typename TContainer = std::initializer_list<T>>
class initializer_list_range : public base_range<initializer_list_range<T>, typename TContainer::const_iterator> {
public:
  struct iterator {
    using container_iter_t = typename TContainer::const_iterator;
    using output_t         = typename TContainer::const_reference;

    explicit iterator(container_iter_t pos)
        : m_pos(pos) {
    }

    bool operator==(const iterator& o) const {
      return m_pos == o.m_pos;
    }

    bool operator!=(const iterator& o) const {
      return m_pos != o.m_pos;
    }

    iterator& operator++() {
      ++m_pos;
      return *this;
    }

    output_t operator*() const {
      return *m_pos;
    }

    container_iter_t m_pos;
  };

  explicit initializer_list_range(std::initializer_list<T> list)
      : m_list(std::move(list)) {
  }

  iterator begin() const {
    return iterator(m_list.begin());
  }

  iterator end() const {
    return iterator(m_list.end());
  }

private:
  std::initializer_list<T> m_list{};
};

// ----------------------------------
// from_to_range
// ----------------------------------

template <typename T>
class from_to_range : public base_range<from_to_range<T>, T> {
public:
  struct iterator {
    using output_t = T;

    iterator(const T& value, const T& bound, const T& step)
        : m_start(value)
        , m_value(value)
        , m_bound(bound)
        , m_step(step) {
    }

    bool operator==(const iterator& o) const {
      return m_value == o.m_value;
    }

    bool operator!=(const iterator& o) const {
      return m_value != o.m_value;
    }

    iterator& operator++() {
      ++m_index;
      return *this;
    }

    output_t operator*() const {
      m_value = (m_step * m_index) + m_start;

      if (m_step < T()) {
        if (m_value < m_bound) {
          m_value = m_bound;
        }
      }
      else if (m_value > m_bound) {
        m_value = m_bound;
      }

      return m_value;
    }

    int32_t   m_index{};
    T         m_start;
    mutable T m_value;
    T         m_bound;
    T         m_step;
  };

  from_to_range(const T& start, const T& end, const T& step)
      : m_start(start)
      , m_end(end)
      , m_step(step) {
    // Unsign the step value.
    if (m_step < T()) {
      m_step = -m_step;
    }

    // Invert the step value if we're going backwards.
    if (m_start > m_end) {
      m_step = -m_step;
    }
  }

  iterator begin() const {
    return iterator(m_start, m_end, m_step);
  }

  iterator end() const {
    return iterator(m_end, T(), T());
  }

private:
  T m_start;
  T m_end;
  T m_step;
};

// ----------------------------------
// generate_range
// ----------------------------------

struct generator_return_value_ident {
  /* Nothing to define here. */
};

template <typename T>
struct generator_return_value : generator_return_value_ident {
  using value_type = T;

  generator_return_value()
      : m_is_empty(true) {
  }

  explicit generator_return_value(const T& value)
      : m_value(value)
      , m_is_empty(false) {
  }

  explicit operator T&() {
    return m_value;
  }

  explicit operator const T&() const {
    return m_value;
  }

  bool operator==(const generator_return_value& o) const {
    // We only care about whether two generator_return_values are empty or not.
    // If the iterator has an empty one, it will match the one from the last
    // iterator, which indicates the end of iteration.
    return m_is_empty == o.m_is_empty;
  }

  bool operator!=(const generator_return_value& o) const {
    // Same as in operator==.
    return m_is_empty != o.m_is_empty;
  }

  T    m_value;
  bool m_is_empty;
};

template <typename TGenerator>
struct generate_range_traits {
  using generator_return_type = std::invoke_result_t<TGenerator(size_t)>;

  // Ensure that whatever the generator returned is indeed of type generator_return_type.
  static_assert(std::is_base_of_v<generator_return_value_ident, generator_return_type>,
                "The generator function is expected to return a result of linq::generate_return() or "
                "linq::generate_finish().");

  // The type that is wrapped by the returned generator_return_type.
  using core_type = typename generator_return_type::value_type;
};

template <typename TGenerator>
class generator_range
    : public base_range<generator_range<TGenerator>, typename generate_range_traits<TGenerator>::core_type> {
public:
  struct iterator {
    using generator_return_type = typename generate_range_traits<TGenerator>::generator_return_type;
    using output_t              = typename generate_range_traits<TGenerator>::core_type;

    iterator(const generator_range* parent, bool is_end)
        : m_parent(parent) {
      if (is_end) {
        m_last_result = generator_return_type();
      }
      else {
        // First iteration
        const auto& generator = *m_parent->Generator;
        m_last_result         = generator(m_iteration);
      }
    }

    bool operator==(const iterator& o) const {
      return m_last_result == o.m_last_result;
    }

    bool operator!=(const iterator& o) const {
      return m_last_result != o.m_last_result;
    }

    iterator& operator++() {
      ++m_iteration;

      const auto& generator = *m_parent->Generator;
      m_last_result         = generator(m_iteration);

      return *this;
    }

    output_t operator*() const {
      return m_last_result;
    }

    const generator_range* m_parent;
    size_t                 m_iteration{};
    generator_return_type  m_last_result;
  };

  explicit generator_range(TGenerator generator)
      : m_generator(std::move(generator)) {
  }

  iterator begin() const {
    return iterator(this, false);
  }

  iterator end() const {
    return iterator(this, true);
  }

private:
  TGenerator m_generator;
};

// ----------------------------------
// base_range method definitions
// ----------------------------------

template <typename TMy, typename TOutput>
template <typename TPredicate>
auto base_range<TMy, TOutput>::where(const TPredicate& predicate) const {
  return where_range<TMy, TPredicate>(static_cast<const TMy&>(*this), predicate);
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::distinct() const {
  return distinct_range<TMy>(static_cast<const TMy&>(*this));
}

template <typename TMy, typename TOutput>
template <typename TTransform>
auto base_range<TMy, TOutput>::select(const TTransform& transform) const {
  return select_range<TMy, TTransform>(static_cast<const TMy&>(*this), transform);
}

template <typename TMy, typename TOutput>
LINQ_NODISCARD auto base_range<TMy, TOutput>::select_to_string() const {
  return select_to_string_range<TMy>(static_cast<const TMy&>(*this));
}

template <typename TMy, typename TOutput>
template <typename TTransform>
auto base_range<TMy, TOutput>::select_many(const TTransform& transform) const {
  return select_many_range<TMy, TTransform>(static_cast<const TMy&>(*this), transform);
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::reverse() const {
  return reverse_range<TMy>(static_cast<const TMy&>(*this));
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::take(size_t count) const {
  return take_range<TMy>(static_cast<const TMy&>(*this), count);
}

template <typename TMy, typename TOutput>
template <typename TPredicate>
auto base_range<TMy, TOutput>::take_while(const TPredicate& predicate) const {
  return take_while_range<TMy, TPredicate>(static_cast<const TMy&>(*this), predicate);
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::skip(size_t count) const {
  return skip_range<TMy>(static_cast<const TMy&>(*this), count);
}

template <typename TMy, typename TOutput>
template <typename TPredicate>
auto base_range<TMy, TOutput>::skip_while(const TPredicate& predicate) const {
  return skip_while_range<TMy, TPredicate>(static_cast<const TMy&>(*this), predicate);
}

template <typename TMy, typename TOutput>
template <typename TOtherRange>
auto base_range<TMy, TOutput>::append(const TOtherRange& other_range) const {
  return append_range<TMy, TOtherRange>(static_cast<const TMy&>(*this), other_range);
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::repeat(size_t count) const {
  return repeat_range<TMy>(static_cast<const TMy&>(*this), count);
}

template <typename TMy, typename TOutput>
template <typename TOtherRange, typename TKeySelectorA, typename TKeySelectorB, typename TTransform>
auto base_range<TMy, TOutput>::join(const TOtherRange&   other_range,
                                    const TKeySelectorA& key_selector_a,
                                    const TKeySelectorB& key_selector_b,
                                    const TTransform&    transform) const {
  return join_range<TMy, TOtherRange, TKeySelectorA, TKeySelectorB, TTransform>(static_cast<const TMy&>(*this),
                                                                                other_range,
                                                                                key_selector_a,
                                                                                key_selector_b,
                                                                                transform);
}

template <typename TMy, typename TOutput>
template <typename TKeySelector>
auto base_range<TMy, TOutput>::order_by(const TKeySelector& key_selector, sort_direction sort_dir) const {
  return order_by_range<TMy, TKeySelector>(static_cast<const TMy&>(*this), key_selector, sort_dir);
}

template <typename TMy, typename TOutput>
template <typename TKeySelector>
auto base_range<TMy, TOutput>::then_by(const TKeySelector& key_selector, sort_direction sort_dir) const {
  return then_by_range<TMy, TKeySelector>(static_cast<const TMy&>(*this), key_selector, sort_dir);
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::sum() const {
  bool     first{true};
  output_t result{};

  for (const auto& p : static_cast<const TMy&>(*this)) {
    if (first) {
      result = p;
      first  = false;
    }
    else {
      result += p;
    }
  }

#ifdef __cpp_lib_optional
  return first ? std::optional<output_t>{} : std::optional<output_t>{result};
#else
  return std::make_pair(!first, sum);
#endif
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::min() const {
  bool     first{true};
  output_t result{};

  for (const auto& p : static_cast<const TMy&>(*this)) {
    if (first) {
      result = p;
      first  = false;
    }
    else if (p < result) {
      result = p;
    }
  }

#ifdef __cpp_lib_optional
  return first ? std::optional<output_t>{} : std::optional<output_t>{result};
#else
  return std::make_pair(!first, sum);
#endif
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::max() const {
  bool     first{true};
  output_t result{};

  for (const auto& p : static_cast<const TMy&>(*this)) {
    if (first) {
      result = p;
      first  = false;
    }
    else if (result < p) {
      result = p;
    }
  }

#ifdef __cpp_lib_optional
  return first ? std::optional<output_t>{} : std::optional<output_t>{result};
#else
  return std::make_pair(!first, sum);
#endif
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::sum_and_count() const {
  bool     first{true};
  output_t result{};
  size_t   count{0};

  for (const auto& p : static_cast<const TMy&>(*this)) {
    if (first) {
      result = p;
      first  = false;
    }
    else {
      result += p;
    }
    ++count;
  }

#ifdef __cpp_lib_optional
  return first ? std::optional<std::pair<output_t, size_t>>{}
               : std::optional<std::pair<output_t, size_t>>{std::make_pair(std::move(result), count)};
#else
  return std::make_pair(!first, std::make_pair(std::move(result), count));
#endif
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::average() const
#ifdef __cpp_lib_concepts
  requires(averageable<output_t> || number<output_t>)
#endif
{
  return calculate_average<output_t>(static_cast<const TMy&>(*this));
}

template <typename TMy, typename TOutput>
template <typename TAccumFunc>
auto base_range<TMy, TOutput>::aggregate(const TAccumFunc& func) const {
  bool     first{true};
  output_t sum{};

  for (const auto& p : static_cast<const TMy&>(*this)) {
    if (first) {
      sum   = p;
      first = false;
    }
    else {
      sum = func(sum, p);
    }
  }

  return sum;
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::first() const {
  for (const auto& p : static_cast<const TMy&>(*this)) {
#ifdef __cpp_lib_optional
    return std::optional<output_t>{p};
#else
    return std::make_pair(true, p);
#endif
  }

  return std::optional<output_t>{};
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::first_or(output_t&& default_value) const {
#ifdef __cpp_lib_optional
  return this->first().value_or(std::forward<output_t>(default_value));
#else
  auto result = this->first();
  return result.first ? std::forward<output_t>(default_value) : output_t{};
#endif
}

template <typename TMy, typename TOutput>
template <typename TPredicate>
auto base_range<TMy, TOutput>::first(const TPredicate& predicate) const {
  for (const auto& p : static_cast<const TMy&>(*this)) {
    if (predicate(p)) {
#ifdef __cpp_lib_optional
      return std::optional<output_t>{p};
#else
      return std::make_pair(true, p);
#endif
    }
  }

#ifdef __cpp_lib_optional
  return std::optional<output_t>{};
#else
  return std::make_pair(false, output_t{});
#endif
}

template <typename TMy, typename TOutput>
template <typename TPredicate>
auto base_range<TMy, TOutput>::first_or(const TPredicate& predicate, output_t&& default_value) const {
#ifdef __cpp_lib_optional
  return this->first(predicate).value_or(std::forward<output_t>(default_value));
#else
  auto result = this->first(predicate);
  return result.first ? std::forward<output_t>(default_value) : output_t{};
#endif
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::last() const {
  bool     first{true};
  output_t ret{};

  for (const auto& p : static_cast<const TMy&>(*this)) {
    ret   = p;
    first = false;
  }

#ifdef __cpp_lib_optional
  return first ? std::optional<output_t>{} : std::optional<output_t>{ret};
#else
  return std::make_pair(!first, ret);
#endif
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::last_or(output_t&& default_value) const {
#ifdef __cpp_lib_optional
  return this->last().value_or(std::forward<output_t>(default_value));
#else
  auto result = this->last();
  return result.first ? std::forward<output_t>(default_value) : output_t{};
#endif
}

template <typename TMy, typename TOutput>
template <typename TPredicate>
auto base_range<TMy, TOutput>::last(const TPredicate& predicate) const {
  bool     first{true};
  output_t ret{};

  for (const auto& p : static_cast<const TMy&>(*this)) {
    if (predicate(p)) {
      ret   = p;
      first = false;
    }
  }

#ifdef __cpp_lib_optional
  return first ? std::optional<output_t>{} : std::optional<output_t>{ret};
#else
  return std::make_pair(!first, ret);
#endif
}

template <typename TMy, typename TOutput>
template <typename TPredicate>
auto base_range<TMy, TOutput>::last_or(const TPredicate& predicate, output_t&& default_value) const {
#ifdef __cpp_lib_optional
  return this->last(predicate).value_or(std::forward<output_t>(default_value));
#else
  auto result = this->last(predicate);
  return result.first ? std::forward<output_t>(default_value) : output_t{};
#endif
}

template <typename TMy, typename TOutput>
template <typename TPredicate>
bool base_range<TMy, TOutput>::any(const TPredicate& predicate) const {
  for (const auto& p : static_cast<const TMy&>(*this)) {
    if (predicate(p)) {
      return true;
    }
  }

  return false;
}

template <typename TMy, typename TOutput>
template <typename TPredicate>
bool base_range<TMy, TOutput>::all(const TPredicate& predicate) const {
  for (const auto& p : static_cast<const TMy&>(*this)) {
    if (!predicate(p)) {
      return false;
    }
  }

  return true;
}

template <typename TMy, typename TOutput>
template <typename TPredicate>
bool base_range<TMy, TOutput>::none(const TPredicate& predicate) const {
  bool any_elements{false};
  bool any_none{false};

  for (const auto& p : static_cast<const TMy&>(*this)) {
    any_elements = true;

    if (!predicate(p)) {
      any_none = true;
      break;
    }
  }

  return any_elements ? any_none : true;
}

template <typename TMy, typename TOutput>
size_t base_range<TMy, TOutput>::count() const {
  size_t ret{0};

  for (const auto& p : static_cast<const TMy&>(*this)) {
    ++ret;
  }

  return ret;
}

template <typename TMy, typename TOutput>
template <typename TPredicate>
size_t base_range<TMy, TOutput>::count(const TPredicate& predicate) const {
  size_t ret{0};

  for (const auto& p : static_cast<const TMy&>(*this)) {
    if (predicate(p)) {
      ++ret;
    }
  }

  return ret;
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::element_at(size_t index, output_t&& default_value) const {
  size_t i{0};

  for (const auto& p : static_cast<const TMy&>(*this)) {
    if (i >= index) {
      return p;
    }

    ++i;
  }

  return default_value;
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::to_vector() const {
  std::vector<output_t> vec;

  for (const auto& p : static_cast<const TMy&>(*this)) {
    vec.push_back(p);
  }

  return vec;
}

} // end namespace details

// from()

/**
 * @brief Creates a non-owning range that references an immutable container.
 * Subsequent operations of this range will produce immutable items.
 *
 * @tparam C The type of container to reference
 * @tparam T The type of a value inside the container
 * @tparam A The allocator type of the container
 * @param container The container to reference
 * @return A range to be used for subsequent operations
 *
 * Example:
 * @code{.cpp}
 * const std::vector<int> numbers{1,2,3};
 * auto query = linq::from(&numbers);
 * for (const auto& value : query) {
 *   // ...
 * }
 * @endcode
 */
template <template <typename, typename> typename C, typename T, typename A>
LINQ_NODISCARD static auto from(const C<T, A>* container) {
  return details::container_range<C<T, A>>{container};
}

// std::set

template <template <typename, typename, typename> typename C, typename T, typename S, typename U>
LINQ_NODISCARD static auto from(const C<T, S, U>* container) {
  return details::container_range<C<T, S, U>>{container};
}

// std::array

template <template <class, size_t> class C, typename T, size_t L>
LINQ_NODISCARD static auto from(const C<T, L>* container) {
  return details::container_range<C<T, L>>{container};
}

// std::map

template <template <class, class, class, class> class C, typename K, typename T, typename S, typename U>
LINQ_NODISCARD static auto from(const C<K, T, S, U>* container) {
  return details::container_range<C<K, T, S, U>>{container};
}

// misc container

template <template <typename> typename C, class T>
LINQ_NODISCARD static auto from(const C<T>* container) {
  return details::container_range<C<T>>{container};
}

// from_mutable()

// std::vector, std::list, std::dequeue, ...

template <template <typename, typename> typename C, typename T, typename A>
LINQ_NODISCARD static auto from_mutable(C<T, A>* container) {
  return details::mutable_container_range<C<T, A>>{container};
}

// std::set

template <template <typename, typename, typename> typename C, typename T, typename S, typename U>
LINQ_NODISCARD static auto from_mutable(C<T, S, U>* container) {
  return details::mutable_container_range<C<T, S, U>>{container};
}

// std::array

template <template <class, size_t> class C, typename T, size_t L>
LINQ_NODISCARD static auto from_mutable(C<T, L>* container) {
  return details::mutable_container_range<C<T, L>>{container};
}

// std::map

template <template <class, class, class, class> class C, typename K, typename T, typename S, typename U>
LINQ_NODISCARD static auto from_mutable(C<K, T, S, U>* container) {
  return details::mutable_container_range<C<K, T, S, U>>{container};
}

// misc container

template <template <typename> typename C, class T>
LINQ_NODISCARD static auto from_mutable(C<T>* container) {
  return details::mutable_container_range<C<T>>{container};
}

// C arrays

/*
template <typename T, size_t N>
LINQ_NODISCARD static auto from_mutable(T (&array)[N]) {
  return details::c_array_range<T, N>(array, array + N);
}
*/

template <typename TContainer>
LINQ_NODISCARD static auto from_copy(const TContainer& container) {
  return details::container_copy_range<TContainer>{container};
}

// TODO: from_move?
// TODO: span compat?

template <typename T>
LINQ_NODISCARD static auto from(std::initializer_list<T> list) {
  return details::initializer_list_range<T>{list};
}

#ifdef __cpp_lib_span
template <typename T>
LINQ_NODISCARD static auto from(std::span<const T> span) {
  return details::container_copy_range<std::span<const T>>{span};
}
#endif

template <typename T>
LINQ_NODISCARD static auto from_to(const T& start, const T& end, const T& step = static_cast<T>(1)) {
  static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>,
                "Only integral and floating point types are supported by from_to().");

  return details::from_to_range<T>(start, end, step);
}

template <typename TGenerator>
LINQ_NODISCARD static auto generate(const TGenerator& generator) {
  return details::generator_range<TGenerator>(generator);
}

template <typename T>
LINQ_NODISCARD static auto generate_return(const T& value) {
  return details::generator_return_value<T>(value);
}

template <typename T>
LINQ_NODISCARD static auto generate_finish() {
  return details::generator_return_value<T>();
}
} // end namespace linq
