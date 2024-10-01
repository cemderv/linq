# linq - LINQ for C++

linq is a header-only LINQ library for C++ 17 and newer.
It has no dependencies and neatly integrates into the STL by taking advantage of modern C++ features.

linq ...

- resolves all type-related functionality at compile-time; no virtual dispatch is used
- uses lazy evaluation, so your queries still work even after you modify the container it's based on
- focuses on immutability, so your queries stay predictable
- is efficient in the way it works with your data; it avoids copies and instead moves data wherever it can
- generates an operation chain at compile-time
- works with all generic container types, not just the STL
- does not rely on exceptions and instead reports errors at compile-time
- uses the beloved dot operator!

---

[![CMake Build](https://github.com/cemderv/linq/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/cemderv/linq/actions/workflows/cmake-multi-platform.yml)

## Examples

#### Intro

```cpp
#include <linq.hpp>

struct Person {
    string name;
    int age;
};

const vector<Person> people {
    { .name = "P1", .age = 20 },
    { .name = "P2", .age = 21 },
    { .name = "P3", .age = 22 },
};
  
auto query = linq::from(&people)
                  .where( [](const Person& p) { return p.age > 20; } );
                    
for (const Person& p : query) {
    println("{}, {}", p.name, p.age);
}
```

Output:

```
P2, 21
P3, 22
```

#### Aggregation

```cpp
double average_age = linq::from(&people)
                          .select( [](const Person& p) { return p.age; } )
                          .average();

int people_over_20 = linq::from(&people)
                          .where( [](const Person& p) { return p.age > 20; } )
                          .count();
```

#### Element access

```cpp
const auto over_20 = [](const Person& p) { return p.age > 20; };
const auto under_20 = [](const Person& p) { return p.age < 20; };

optional<Person> first_over_20 = linq::from(&people).first(over_20);
// first_over_20 = P2, 21

optional<Person> last_over_20 = linq::from(&people).last(over_20);
// last_over_20 = P3, 22

optional<Person> first_under_20 = linq::from(&people).last(under_20);
// first_under_20 = empty optional
```

#### Partitioning

```cpp
const vector numbers { 1, 2, 3, 4, 5, 6 };

auto query1 = linq::from(&numbers).skip(3); // 4, 5, 6

auto query2 = linq::from(&numbers)
                   .skip_while( [](int i) { return i < 4; } ); // 4, 5, 6

auto query3 = linq::from(&numbers).take(4); // 1, 2, 3, 4

auto query4 = linq::from(&numbers)
                   .take_while( [](int i) { return i < 4; } ); // 1, 2, 3
```

#### Sorting

```cpp
const vector words { "hello"s, "world"s, "here"s, "are"s, "some"s, "sorted"s, "words"s };

auto query = linq::from(&words)
                  .order_by_ascending( [](const string& word) { return word.size(); } )
                  .then_by_ascending( [](const string& word) { return word; } );
// query = are, here, some, hello, words, world, sorted

auto rev = query.reverse();
// rev = sorted, world, words, hello, some, here, are
```

#### Concatenation

```cpp
const vector numbers1 { 1, 2, 3 };
const vector numbers2 { 4, 5, 6 };

auto range = linq::from(&numbers1)
                  .append(linq::from(&numbers2));

// range = 1, 2, 3, 4, 5, 6
```

#### Removing duplicates

```cpp
const vector numbers { 1, 2, 3, 3, 5, 4, 5, 6, 7 };

const auto query = linq::from(&numbers).distinct();
// query = 1, 2, 3, 5, 4, 6, 7
```

#### Composition

```cpp
auto people = linq::from(&people);
auto age_over_20 = [](const Person& p) { return p.age > 20; };
auto people_over_20 = people.where(age_over_20);
auto person_age = [](const Person& p) { return p.age; };

double average_age = people_over_20.select(person_age).average();
```

#### Producing containers

`to_vector`:

```cpp
const array numbers { 1, 2, 3, 4 };

vector vec = linq::from(&numbers)
                  .where( [](int i) { return i > 1; } )
                  .select_to_string()
                  .to_vector(); // copy elements to a new std::vector

// 'vec' is of type std::vector<std::string>
// with contents ["2", "3", "4"]
```

`to_map`:

```cpp
const array pairs {
    pair{ 1, "str1"s },
    pair{ 5, "str5"s },
    pair{ -10, "str-10"s },
};

const auto query = linq::from(&pairs)
                        .where( [](const auto& p) { return p.first < 20; } );

map my_map = query.to_map();

for (const auto& [key, value] : my_map) {
    println("[{}: {}]", key, value);
}
```

Output:

```
[-10: str-10]
[1: str1]
[5: str5]
```

### Generation

```cpp
auto range1 = linq::from_to(0, 5);           // 0, 1, 2, 3, 4, 5

auto range2 = linq::from_to(0.0, 1.5, 0.5);  // 0, 0.5, 1, 1.5

auto range3 = linq::from_to(0, 3).repeat(1); // 0, 1, 2, 3, 0, 1, 2, 3

auto range4 = linq::generate([](size_t i) {
    if (i < 5)
        return linq::generate_return(i * 2);

    return linq::generate_finish<size_t>();
});
// range4 = 0, 2, 4, 6, 8
```

---

Below you will find a list of all supported functions and operators.

### Query Constructors

- [from](https://github.com/cemderv/linq/wiki/Query-Constructors#from)
- [from_mutable](https://github.com/cemderv/linq/wiki/Query-Constructors#from_mutable)
- [from_copy](https://github.com/cemderv/linq/wiki/Query-Constructors#from_copy)

### Container Producers

- [to_vector](https://github.com/cemderv/linq/wiki/Container-Producers#to_vector)
- [to_map / to_unordered_map](https://github.com/cemderv/linq/wiki/Container-Producers#to_map--to_unordered_map)

### Operators

- [Aggregation](https://github.com/cemderv/linq/wiki/Aggregate-Operators)
    - [aggregate](https://github.com/cemderv/linq/wiki/Aggregate-Operators#aggregate)
    - [average](https://github.com/cemderv/linq/wiki/Aggregate-Operators#average)
    - [count](https://github.com/cemderv/linq/wiki/Aggregate-Operators#count)
    - [max](https://github.com/cemderv/linq/wiki/Aggregate-Operators#max)
    - [min](https://github.com/cemderv/linq/wiki/Aggregate-Operators#min)
    - [sum](https://github.com/cemderv/linq/wiki/Aggregate-Operators#sum)
- [Concatenation](https://github.com/cemderv/linq/wiki/Concatenation-Operators)
    - [append](https://github.com/cemderv/linq/wiki/Concatenation-Operators#append)
- [Element Access](https://github.com/cemderv/linq/wiki/Element-Operators)
    - [element_at](https://github.com/cemderv/linq/wiki/Element-Operators#element_at)
    - [first](https://github.com/cemderv/linq/wiki/Element-Operators#first)
    - [last](https://github.com/cemderv/linq/wiki/Element-Operators#last)
- [Filters](https://github.com/cemderv/linq/wiki/Filter-Operators)
    - [where](https://github.com/cemderv/linq/wiki/Filter-Operators#where)
- [Generation](https://github.com/cemderv/linq/wiki/Generation-Operators)
    - [from_to](https://github.com/cemderv/linq/wiki/Generation-Operators#from_to)
    - [repeat](https://github.com/cemderv/linq/wiki/Generation-Operators#repeat)
    - [generate](https://github.com/cemderv/linq/wiki/Generation-Operators#generate)
- [Join](https://github.com/cemderv/linq/wiki/Join-Operators)
    - [join](https://github.com/cemderv/linq/wiki/Join-Operators#join)
- [Partition](https://github.com/cemderv/linq/wiki/Partition-Operators)
    - [skip](https://github.com/cemderv/linq/wiki/Partition-Operators#skip)
    - [skip_while](https://github.com/cemderv/linq/wiki/Partition-Operators#skip_while)
    - [take](https://github.com/cemderv/linq/wiki/Partition-Operators#take)
    - [take_while](https://github.com/cemderv/linq/wiki/Partition-Operators#take_while)
- [Projection](https://github.com/cemderv/linq/wiki/Projection-Operators)
    - [select](https://github.com/cemderv/linq/wiki/Projection-Operators#select)
    - [select_to_string](https://github.com/cemderv/linq/wiki/Projection-Operators#select_to_string)
    - [select_many](https://github.com/cemderv/linq/wiki/Projection-Operators#select_many)
- [Quantifiers](https://github.com/cemderv/linq/wiki/Quantifier-Operators)
    - [all](https://github.com/cemderv/linq/wiki/Quantifier-Operators#all)
    - [any](https://github.com/cemderv/linq/wiki/Quantifier-Operators#any)
- [Set](https://github.com/cemderv/linq/wiki/Set-Operators)
    - [distinct](https://github.com/cemderv/linq/wiki/Set-Operators#distinct)
- [Sorting](https://github.com/cemderv/linq/wiki/Sorting-Operators)
    - [order_by](https://github.com/cemderv/linq/wiki/Sorting-Operators#order_by)
    - [then_by](https://github.com/cemderv/linq/wiki/Sorting-Operators#then_by)
    - [reverse](https://github.com/cemderv/linq/wiki/Sorting-Operators#reverse)

