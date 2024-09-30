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
#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#ifdef __cpp_lib_span
#include <span>
#endif

#ifdef __cpp_lib_concepts
#include <concepts>
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

template <typename T>
concept addable = requires(T a, T b) {
  std::is_convertible_v<int, T>;
  a += b;
  a < b;
};
#endif

template <typename T, typename TRange>
#ifdef __cpp_lib_concepts
  requires(averageable<T> || number<T>)
#endif
static auto calculate_average(const TRange& op) {
  using float_t  = long double;
  using output_t = typename TRange::output_t;

#ifdef __cpp_lib_concepts
  using return_t = std::conditional_t<number<output_t>, float_t, output_t>;
#else
  using return_t =
      std::conditional_t<std::is_integral<output_t>::value || std::is_floating_point_v<output_t>, float_t, output_t>;
#endif

  if (const auto maybe_sum_and_count = op.sum_and_count()) {
    const auto& sum_and_count = *maybe_sum_and_count;
    return std::optional<return_t>{static_cast<return_t>(sum_and_count.first) / sum_and_count.second};
  }

  return std::optional<return_t>{};
}

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
 * @tparam TOutput The full, unmodified type that is returned by the range.
 */
template <typename /*TMy*/, typename TOutput>
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
  [[nodiscard]] auto where(const TPredicate& predicate) const;

  /**
   * @brief Appends a distinct-filter to the range that removes duplicate elements.
   * @return A new range that combines this range with the distinct-range.
   */
  [[nodiscard]] auto distinct() const;

  template <typename TTransform>
  [[nodiscard]] auto select(TTransform&& transform) const;

  [[nodiscard]] auto select_to_string() const;

  template <typename TTransform>
  [[nodiscard]] auto select_many(TTransform&& transform) const;

  [[nodiscard]] auto reverse() const;

  [[nodiscard]] auto take(size_t count) const;

  template <typename TPredicate>
  [[nodiscard]] auto take_while(TPredicate&& predicate) const;

  [[nodiscard]] auto skip(size_t count) const;

  template <typename TPredicate>
  [[nodiscard]] auto skip_while(TPredicate&& predicate) const;

  template <typename TOtherRange>
  [[nodiscard]] auto append(const TOtherRange& other_range) const;

  [[nodiscard]] auto repeat(size_t count) const;

  template <typename TOtherRange, typename TKeySelectorA, typename TKeySelectorB, typename TTransform>
  [[nodiscard]] auto join(const TOtherRange& other_range,
                          TKeySelectorA&&    key_selector_a,
                          TKeySelectorB&&    key_selector_b,
                          TTransform&&       transform) const;

  template <typename TKeySelector>
  [[nodiscard]] auto order_by(TKeySelector&& key_selector, sort_direction sort_dir) const;

  template <typename TKeySelector>
  [[nodiscard]] auto order_by_ascending(TKeySelector&& key_selector) const {
    return order_by<TKeySelector>(std::forward<TKeySelector>(key_selector), sort_direction::ascending);
  }

  template <typename TKeySelector>
  [[nodiscard]] auto order_by_descending(TKeySelector&& key_selector) const {
    return order_by<TKeySelector>(std::forward<TKeySelector>(key_selector), sort_direction::descending);
  }

  template <typename TKeySelector>
  [[nodiscard]] auto then_by(TKeySelector&& key_selector, sort_direction sort_dir) const;

  template <typename TKeySelector>
  [[nodiscard]] auto then_by_ascending(TKeySelector&& key_selector) const {
    return then_by<TKeySelector>(std::forward<TKeySelector>(key_selector), sort_direction::ascending);
  }

  template <typename TKeySelector>
  [[nodiscard]] auto then_by_descending(TKeySelector&& key_selector) const {
    return then_by<TKeySelector>(std::forward<TKeySelector>(key_selector), sort_direction::descending);
  }

  [[nodiscard]] auto sum() const;
  [[nodiscard]] auto min() const;
  [[nodiscard]] auto max() const;

  [[nodiscard]] auto sum_and_count() const;

  [[nodiscard]] auto average() const
#ifdef __cpp_lib_concepts
    requires(averageable<output_t> || number<output_t>)
#endif
  ;

  template <typename TAccumFunc>
  [[nodiscard]] auto aggregate(const TAccumFunc& func) const;

  [[nodiscard]] std::optional<output_t> first() const;

  template <typename TPredicate>
  [[nodiscard]] std::optional<output_t> first(const TPredicate& predicate) const;

  [[nodiscard]] std::optional<output_t> last() const;

  template <typename TPredicate>
  [[nodiscard]] std::optional<output_t> last(const TPredicate& predicate) const;

  template <typename TPredicate>
  [[nodiscard]] bool any(const TPredicate& predicate) const;

  template <typename TPredicate>
  [[nodiscard]] bool all(const TPredicate& predicate) const;

  template <typename TPredicate>
  [[nodiscard]] bool none(const TPredicate& predicate) const;

  [[nodiscard]] size_t count() const;

  template <typename TPredicate>
  [[nodiscard]] size_t count(const TPredicate& predicate) const;

  [[nodiscard]] std::optional<output_t> element_at(size_t index) const;

  [[nodiscard]] std::vector<output_t> to_vector() const;

  [[nodiscard]] auto to_map() const;

  [[nodiscard]] auto to_unordered_map() const;
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

    const output_t& operator*() const {
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
      } while (m_begin != m_end && contains_object(m_begin));

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

    const output_t& operator*() const {
      return *m_begin;
    }

    prev_iter_t       m_begin;
    prev_iter_t       m_end;
    object_container* m_encountered_objects;
  };

  distinct_range() = default;

  explicit distinct_range(const TPrevRange& prev)
      : m_prev(prev) {
  }

  iterator begin() const {
    return iterator{m_prev.begin(), m_prev.end(), std::addressof(m_encountered_objects)};
  }

  iterator end() const {
    const auto prev_end = m_prev.end();
    return iterator{prev_end, prev_end, std::addressof(m_encountered_objects)};
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
      return m_parent->m_transform(*m_begin);
    }

    const select_range* m_parent;
    prev_iter_t         m_begin;
    prev_iter_t         m_end;
  };

  select_range(const TPrevRange& prev, TTransform transform)
      : m_prev(prev)
      , m_transform(std::move(transform)) {
  }

  iterator begin() const {
    return iterator(this, m_prev.begin(), m_prev.end());
  }

  iterator end() const {
    const auto prev_end = m_prev.end();
    return iterator(this, prev_end, prev_end);
  }

private:
  TPrevRange m_prev;
  TTransform m_transform{};
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
        , m_pos(std::move(pos))
        , m_end(std::move(end)) {
      if (m_pos != m_end) {
        const auto& transform = m_parent->m_transform;
        bool        first     = true;

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
          m_ret_range = m_parent->m_transform(*m_pos);
          m_ret_begin = m_ret_range.begin();
          m_ret_end   = m_ret_range.end();
        }
      }

      return *this;
    }

    const output_t& operator*() const {
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

    const output_t& operator*() const {
      return *(*m_prev_iterators)[m_index];
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
    return iterator{nullptr, static_cast<size_t>(-1)};
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

    const output_t& operator*() const {
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

    const output_t& operator*() const {
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

    const output_t& operator*() const {
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
        : m_begin(begin) {
      while (m_begin != end && predicate(*m_begin)) {
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
      ++m_begin;
      return *this;
    }

    const output_t& operator*() const {
      return *m_begin;
    }

    prev_iter_t m_begin;
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

    const output_t& operator*() const {
      return m_my_begin != m_my_end ? *m_my_begin : *m_other_begin;
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

    const output_t& operator*() const {
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
    std::invoke_result_t<TTransform, typename TRangeA::iterator::output_t, typename TRangeB::iterator::output_t>;

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
      return m_parent->m_transform(*m_pos, *m_other_pos);
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
      const auto& key_selector_a = m_parent->m_key_selector_a;
      const auto& key_selector_b = m_parent->m_key_selector_b;

      if (pre_increment_other) {
        ++m_other_pos;
      }

      while (m_pos != m_end) {
        bool       should_continue = true;
        const auto key_a           = key_selector_a(*m_pos);

        while (m_other_pos != m_other_end) {
          const auto key_b = key_selector_b(*m_other_pos);

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

  join_range(const TPrevRange& prev,
             TOtherRange       other_range,
             TKeySelectorA     key_selector_a,
             TKeySelectorB     key_selector_b,
             TTransform        transform)
      : m_prev(prev)
      , m_other_range(other_range)
      , m_key_selector_a(std::move(key_selector_a))
      , m_key_selector_b(std::move(key_selector_b))
      , m_transform(std::move(transform)) {
  }

  iterator begin() const {
    return iterator(m_prev.begin(), m_prev.end(), this);
  }

  iterator end() const {
    const auto prev_end = m_prev.end();
    return iterator(prev_end, prev_end, this);
  }

private:
  TPrevRange    m_prev;
  TOtherRange   m_other_range;
  TKeySelectorA m_key_selector_a;
  TKeySelectorB m_key_selector_b;
  TTransform    m_transform;
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

  order_by_range(const TPrevRange& prev, TKeySelector key_selector, sort_direction sort_dir)
      : m_prev(prev)
      , m_key_selector(std::move(key_selector))
      , m_sort_direction(sort_dir) {
  }

  iterator begin() const {
    m_sorted_values.clear();

    for (const auto& val : m_prev) {
      m_sorted_values.push_back(val);
    }

    std::sort(m_sorted_values.begin(),
              m_sorted_values.end(),
              [this](const container_element_t& a, const container_element_t& b) { return compare_keys(a, b); });

    return iterator(m_sorted_values.begin());
  }

  iterator end() const {
    return iterator(m_sorted_values.end());
  }

  bool compare_keys(const container_element_t& a, const container_element_t& b) const {
    const auto a_val = m_key_selector(a);
    const auto b_val = m_key_selector(b);

    return m_sort_direction == sort_direction::ascending ? /*ascending:*/ a_val < b_val : /*descending:*/ a_val > b_val;
  }

private:
  TPrevRange          m_prev;
  TKeySelector        m_key_selector;
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

  then_by_range(const TPrevRange& prev, TKeySelector key_selector, sort_direction sort_dir)
      : m_prev(prev)
      , m_key_selector(std::move(key_selector))
      , m_sort_direction(sort_dir) {
  }

  iterator begin() const {
    m_sorted_values.clear();
    for (const auto& val : m_prev) {
      m_sorted_values.emplace_back(val);
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
    const auto a_value = m_key_selector(a);
    const auto b_value = m_key_selector(b);

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
  TKeySelector        m_key_selector;
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

    iterator() = default;

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

    container_iter_t m_pos{};
  };

  container_range() = default;

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

    output_t& operator*() const {
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

    output_t& operator*() const {
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

    const output_t& operator*() const {
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

    iterator(output_t value, const output_t* step)
        : m_value(std::move(value))
        , m_step(step) {
    }

    bool operator==(const iterator& o) const {
      return o.m_value < m_value;
    }

    bool operator!=(const iterator& o) const {
      return !(*this == o);
    }

    iterator& operator++() {
      m_value += *m_step;
      return *this;
    }

    const output_t& operator*() const {
      return m_value;
    }

    output_t        m_value;
    const output_t* m_step;
  };

  from_to_range(T start, T end, T step)
      : m_start(std::move(start))
      , m_end(std::move(end))
      , m_step(std::move(step)) {
    assert(start < end);
  }

  iterator begin() const {
    return iterator{m_start, std::addressof(m_step)};
  }

  iterator end() const {
    return iterator{m_end, std::addressof(m_step)};
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

  explicit generator_return_value(T value)
      : m_value(std::move(value))
      , m_is_empty(false) {
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
  using generator_return_type = std::invoke_result_t<TGenerator, size_t>;

  // Ensure that whatever the generator returned is indeed of type generator_return_type.
  static_assert(std::is_base_of_v<generator_return_value_ident, generator_return_type>,
                "The generator function is expected to return a result of linq::generate_return() or "
                "linq::generate_finish().");

  // The type that is wrapped by the returned generator_return_type.
  using value_type = typename generator_return_type::value_type;
};

template <typename TGenerator>
class generator_range
    : public base_range<generator_range<TGenerator>, typename generate_range_traits<TGenerator>::value_type> {
public:
  struct iterator {
    using generator_return_type = typename generate_range_traits<TGenerator>::generator_return_type;
    using output_t              = typename generate_range_traits<TGenerator>::value_type;

    iterator(const generator_range* parent, bool is_end)
        : m_parent(parent) {
      if (is_end) {
        m_last_result = generator_return_type();
      }
      else {
        // First iteration
        m_last_result = m_parent->m_generator(m_iteration);
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
      m_last_result = m_parent->m_generator(m_iteration);
      return *this;
    }

    const output_t& operator*() const {
      return m_last_result.m_value;
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
auto base_range<TMy, TOutput>::select(TTransform&& transform) const {
  return select_range<TMy, TTransform>(static_cast<const TMy&>(*this), std::forward<TTransform>(transform));
}

template <typename TMy, typename TOutput>
[[nodiscard]] auto base_range<TMy, TOutput>::select_to_string() const {
  return select_to_string_range<TMy>(static_cast<const TMy&>(*this));
}

template <typename TMy, typename TOutput>
template <typename TTransform>
auto base_range<TMy, TOutput>::select_many(TTransform&& transform) const {
  return select_many_range<TMy, TTransform>(static_cast<const TMy&>(*this), std::forward<TTransform>(transform));
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
auto base_range<TMy, TOutput>::take_while(TPredicate&& predicate) const {
  return take_while_range<TMy, TPredicate>(static_cast<const TMy&>(*this), std::forward<TPredicate>(predicate));
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::skip(size_t count) const {
  return skip_range<TMy>(static_cast<const TMy&>(*this), count);
}

template <typename TMy, typename TOutput>
template <typename TPredicate>
auto base_range<TMy, TOutput>::skip_while(TPredicate&& predicate) const {
  return skip_while_range<TMy, TPredicate>(static_cast<const TMy&>(*this), std::forward<TPredicate>(predicate));
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
auto base_range<TMy, TOutput>::join(const TOtherRange& other_range,
                                    TKeySelectorA&&    key_selector_a,
                                    TKeySelectorB&&    key_selector_b,
                                    TTransform&&       transform) const {
  return join_range<TMy, TOtherRange, TKeySelectorA, TKeySelectorB, TTransform>(static_cast<const TMy&>(*this),
                                                                                other_range,
                                                                                std::move(key_selector_a),
                                                                                std::move(key_selector_b),
                                                                                std::move(transform));
}

template <typename TMy, typename TOutput>
template <typename TKeySelector>
auto base_range<TMy, TOutput>::order_by(TKeySelector&& key_selector, sort_direction sort_dir) const {
  return order_by_range<TMy, TKeySelector>(static_cast<const TMy&>(*this),
                                           std::forward<TKeySelector>(key_selector),
                                           sort_dir);
}

template <typename TMy, typename TOutput>
template <typename TKeySelector>
auto base_range<TMy, TOutput>::then_by(TKeySelector&& key_selector, sort_direction sort_dir) const {
  return then_by_range<TMy, TKeySelector>(static_cast<const TMy&>(*this),
                                          std::forward<TKeySelector>(key_selector),
                                          sort_dir);
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::sum() const {
  bool     first = true;
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

  return first ? std::optional<output_t>{} : std::optional<output_t>{result};
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::min() const {
  bool     first = true;
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

  return first ? std::optional<output_t>{} : std::optional<output_t>{result};
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::max() const {
  bool     first = true;
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

  return first ? std::optional<output_t>{} : std::optional<output_t>{result};
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::sum_and_count() const {
  bool     first = true;
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

  return first ? std::optional<std::pair<output_t, size_t>>{}
               : std::optional<std::pair<output_t, size_t>>{std::make_pair(std::move(result), count)};
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
  bool     first = true;
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
std::optional<typename base_range<TMy, TOutput>::output_t> base_range<TMy, TOutput>::first() const {
  for (const auto& p : static_cast<const TMy&>(*this)) {
    return std::optional{p};
  }

  return {};
}

template <typename TMy, typename TOutput>
template <typename TPredicate>
std::optional<typename base_range<TMy, TOutput>::output_t>
base_range<TMy, TOutput>::first(const TPredicate& predicate) const {
  for (const auto& p : static_cast<const TMy&>(*this)) {
    if (predicate(p)) {
      return std::optional{p};
    }
  }

  return {};
}

template <typename TMy, typename TOutput>
std::optional<typename base_range<TMy, TOutput>::output_t> base_range<TMy, TOutput>::last() const {
  bool     have_any = false;
  output_t ret{};

  for (const auto& p : static_cast<const TMy&>(*this)) {
    ret      = p;
    have_any = true;
  }

  return have_any ? std::optional{ret} : std::optional<output_t>{};
}

template <typename TMy, typename TOutput>
template <typename TPredicate>
std::optional<typename base_range<TMy, TOutput>::output_t>
base_range<TMy, TOutput>::last(const TPredicate& predicate) const {
  bool     have_any = false;
  output_t ret{};

  for (const auto& p : static_cast<const TMy&>(*this)) {
    if (predicate(p)) {
      ret      = p;
      have_any = true;
    }
  }

  return have_any ? std::optional{ret} : std::optional<output_t>{};
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
  bool any_elements = false;
  bool any_none     = false;

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
std::optional<typename base_range<TMy, TOutput>::output_t> base_range<TMy, TOutput>::element_at(size_t index) const {
  size_t i{0};

  for (const auto& p : static_cast<const TMy&>(*this)) {
    if (i >= index) {
      return std::optional{p};
    }

    ++i;
  }

  return {};
}

template <typename TMy, typename TOutput>
std::vector<typename base_range<TMy, TOutput>::output_t> base_range<TMy, TOutput>::to_vector() const {
  std::vector<output_t> vec;

  for (const auto& p : static_cast<const TMy&>(*this)) {
    vec.emplace_back(p);
  }

  return vec;
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::to_map() const {
  std::map<typename output_t::first_type, typename output_t::second_type> map;

  for (const auto& [first, second] : static_cast<const TMy&>(*this)) {
    map.emplace(first, second);
  }

  return map;
}

template <typename TMy, typename TOutput>
auto base_range<TMy, TOutput>::to_unordered_map() const {
  std::unordered_map<typename output_t::first_type, typename output_t::second_type> map;

  for (const auto& [first, second] : static_cast<const TMy&>(*this)) {
    map.emplace(first, second);
  }

  return map;
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
[[nodiscard]] static auto from(const C<T, A>* container) {
  return details::container_range<C<T, A>>{container};
}

// std::set

template <template <typename, typename, typename> typename C, typename T, typename S, typename U>
[[nodiscard]] static auto from(const C<T, S, U>* container) {
  return details::container_range<C<T, S, U>>{container};
}

// std::array

template <template <class, size_t> class C, typename T, size_t L>
[[nodiscard]] static auto from(const C<T, L>* container) {
  return details::container_range<C<T, L>>{container};
}

// std::map

template <template <class, class, class, class> class C, typename K, typename T, typename S, typename U>
[[nodiscard]] static auto from(const C<K, T, S, U>* container) {
  return details::container_range<C<K, T, S, U>>{container};
}

// misc container

template <template <typename> typename C, class T>
[[nodiscard]] static auto from(const C<T>* container) {
  return details::container_range<C<T>>{container};
}

// from_mutable()

// std::vector, std::list, std::dequeue, ...

template <template <typename, typename> typename C, typename T, typename A>
[[nodiscard]] static auto from_mutable(C<T, A>* container) {
  return details::mutable_container_range<C<T, A>>{container};
}

// std::set

template <template <typename, typename, typename> typename C, typename T, typename S, typename U>
[[nodiscard]] static auto from_mutable(C<T, S, U>* container) {
  return details::mutable_container_range<C<T, S, U>>{container};
}

// std::array

template <template <class, size_t> class C, typename T, size_t L>
[[nodiscard]] static auto from_mutable(C<T, L>* container) {
  return details::mutable_container_range<C<T, L>>{container};
}

// std::map

template <template <class, class, class, class> class C, typename K, typename T, typename S, typename U>
[[nodiscard]] static auto from_mutable(C<K, T, S, U>* container) {
  return details::mutable_container_range<C<K, T, S, U>>{container};
}

// misc container

template <template <typename> typename C, class T>
[[nodiscard]] static auto from_mutable(C<T>* container) {
  return details::mutable_container_range<C<T>>{container};
}

template <typename TContainer>
[[nodiscard]] static auto from_copy(const TContainer& container) {
  return details::container_copy_range<TContainer>{container};
}

template <typename T>
[[nodiscard]] static auto from(std::initializer_list<T> list) {
  return details::initializer_list_range<T>{list};
}

#ifdef __cpp_lib_span
template <typename T>
[[nodiscard]] static auto from(std::span<const T> span) {
  return details::container_copy_range<std::span<const T>>{span};
}

template <typename T>
[[nodiscard]] static auto from(std::span<T> span) {
  return details::container_copy_range<std::span<T>>{span};
}
#endif

template <typename T>
#ifdef __cpp_lib_concepts
  requires(details::addable<T> || details::number<T>)
#endif
[[nodiscard]] static auto from_to(T start, T end, T step = T{1}) {
  return details::from_to_range<T>(std::move(start), std::move(end), std::move(step));
}

template <typename TGenerator>
[[nodiscard]] static details::generator_range<TGenerator> generate(TGenerator&& generator) {
  return details::generator_range<TGenerator>{std::forward<TGenerator>(generator)};
}

template <typename T>
[[nodiscard]] static details::generator_return_value<T> generate_return(T&& value) {
  return details::generator_return_value{std::forward<T>(value)};
}

template <typename T>
[[nodiscard]] static details::generator_return_value<T> generate_finish() {
  return {};
}
} // end namespace linq
