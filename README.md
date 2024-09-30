# linq - LINQ for C++

linq is a header-only LINQ library for C++ 14 and newer.
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

## Example

```cpp
#include <linq.hpp>

struct Person {
  string name;
  int age;
};

int main() {
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
}
```

### Query Constructors

- [from](https://github.com/cemderv/linq/wiki/Query-Constructors#from)
- [from_mutable](https://github.com/cemderv/linq/wiki/Query-Constructors#from_mutable)
- [from_copy](https://github.com/cemderv/linq/wiki/Query-Constructors#from_copy)

### Container Producers

- [to_vector](https://github.com/cemderv/linq/wiki/Container-Producers#to_vector)

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

