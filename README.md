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
#include <print>
#include <string>
#include <linq.hpp>

struct Person {
  std::string name;
  int age;
};

int main() {
  const vector<Person> people {
    { .name = "P1", .age = 20 },
    { .name = "P2", .age = 21 },
    { .name = "P3", .age = 22 },
  };
  
  auto query = linq::from(&people)
                    .where([](const Person& p) { return p.age > 20; });
                    
  for (const auto& p : query) {
    println("{}, {}", p.name, p.age);
  }

  return 0;
}
```


