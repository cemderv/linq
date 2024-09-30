#include <format>
#include <iostream>
#include <linq.hpp>
#include <snitch/snitch.hpp>
#include <span>
#include <string>
#include <vector>

struct person {
  std::string name;
  int         age{};
};

static const std::vector<person> general_people{
    {.name = "P1", .age = 20},
    {.name = "P2", .age = 21},
    {.name = "P3", .age = 22},
    {.name = "P4", .age = 10},
    {.name = "P5", .age = -10},
    {.name = "P6", .age = 391},
};

template <typename T>
struct mock_vector : std::vector<T> {
  mock_vector(std::initializer_list<T> list)
      : std::vector<T>(std::move(list)) {
  }

  auto begin() {
    ++begin_call_count;
    return std::vector<T>::begin();
  }

  auto begin() const {
    ++begin_call_count;
    return std::vector<T>::begin();
  }

  auto cbegin() const {
    ++begin_call_count;
    return std::vector<T>::cbegin();
  }

  void reset_count() {
    begin_call_count = 0;
  }

  mutable int begin_call_count{};
};

TEST_CASE("basic") {
  SECTION("direct link") {
    std::vector<std::string> lines;

    for (const auto& [name, age] : linq::from(&general_people)) {
      lines.push_back(std::format("{}: {}", name, age));
    }

    REQUIRE(lines.size() == 6);
    REQUIRE(lines.at(0) == "P1: 20");
    REQUIRE(lines.at(1) == "P2: 21");
    REQUIRE(lines.at(2) == "P3: 22");
    REQUIRE(lines.at(3) == "P4: 10");
    REQUIRE(lines.at(4) == "P5: -10");
    REQUIRE(lines.at(5) == "P6: 391");
  }
}

TEST_CASE("where") {
  SECTION("simple") {
    auto query = linq::from(&general_people).where([](const person& p) { return p.age > 20; });

    std::vector<std::string> lines;

    for (const auto& [name, age] : query) {
      lines.push_back(std::format("{}: {}", name, age));
    }

    REQUIRE(lines.size() == 3);
    REQUIRE(lines.at(0) == "P2: 21");
    REQUIRE(lines.at(1) == "P3: 22");
    REQUIRE(lines.at(2) == "P6: 391");
  }

  SECTION("compound") {
    auto query = linq::from(&general_people).where([](const person& p) {
      return (p.age > 20 && p.age < 391) || p.name == "P5";
    });

    std::vector<std::string> lines;

    for (const auto& [name, age] : query) {
      lines.push_back(std::format("{}: {}", name, age));
    }

    REQUIRE(lines.size() == 3);
    REQUIRE(lines.at(0) == "P2: 21");
    REQUIRE(lines.at(1) == "P3: 22");
    REQUIRE(lines.at(2) == "P5: -10");
  }
}

TEST_CASE("begin() count") {
  {
    const mock_vector nums{1, 2, 3, 4};

    auto query = linq::from(&nums);
    REQUIRE(nums.begin_call_count == 0);

    for (const auto& _ : query)
      ;

    REQUIRE(nums.begin_call_count == 1);
  }

  {
    const mock_vector nums{1, 2, 3, 4};

    auto query = linq::from(&nums).where([](int num) { return num > 0; });
    REQUIRE(nums.begin_call_count == 0);

    for (const auto& _ : query)
      ;

    REQUIRE(nums.begin_call_count == 1);
  }
}

TEST_CASE("item mutability") {
  SECTION("with const container, non-const query") {
    const std::vector nums{1, 2, 3, 4};
    auto              query = linq::from(&nums);

    for (auto&& item : query) {
      static_assert(std::is_const_v<std::remove_reference_t<decltype(item)>>, "expected the item to be immutable");
    }
  }

  SECTION("with const container, const query") {
    const std::vector nums{1, 2, 3, 4};
    const auto        query = linq::from(&nums);

    for (auto&& item : query) {
      static_assert(std::is_const_v<std::remove_reference_t<decltype(item)>>, "expected the item to be immutable");
    }
  }

  SECTION("with non-const container, const query") {
    std::vector nums{1, 2, 3, 4};
    const auto  query = linq::from(&nums);

    for (auto&& item : query) {
      static_assert(std::is_const_v<std::remove_reference_t<decltype(item)>>, "expected the item to be immutable");
    }
  }

  SECTION("with non-const container, non-const query") {
    std::vector nums{1, 2, 3, 4};
    auto        query = linq::from(&nums);

    for (auto&& item : query) {
      static_assert(std::is_const_v<std::remove_reference_t<decltype(item)>>, "expected the item to be immutable");
    }
  }

  // now with linq::from_mutable()

  SECTION("const query") {
    std::vector nums{1, 2, 3, 4};
    const auto  query = linq::from_mutable(&nums);

    for (auto&& item : query) {
      static_assert(!std::is_const_v<std::remove_reference_t<decltype(item)>>, "expected the item to be mutable");
    }
  }

  SECTION("non-const query") {
    std::vector nums{1, 2, 3, 4};
    auto        query = linq::from_mutable(&nums);

    for (auto&& item : query) {
      static_assert(!std::is_const_v<std::remove_reference_t<decltype(item)>>, "expected the item to be mutable");
    }
  }
}

TEST_CASE("query copying") {
  const std::vector nums{1, 2, 3, 4};

  auto query1 = linq::from(&nums);

  std::vector<const int*> nums_ptrs;
  for (const auto& num : query1) {
    nums_ptrs.push_back(std::addressof(num));
  }

  REQUIRE(nums_ptrs.size() == 4);
  REQUIRE(nums_ptrs.at(0) == std::addressof(nums.at(0)));
  REQUIRE(nums_ptrs.at(1) == std::addressof(nums.at(1)));
  REQUIRE(nums_ptrs.at(2) == std::addressof(nums.at(2)));
  REQUIRE(nums_ptrs.at(3) == std::addressof(nums.at(3)));

  auto query2 = query1;

  REQUIRE(nums_ptrs.size() == 4);
  REQUIRE(nums_ptrs.at(0) == std::addressof(nums.at(0)));
  REQUIRE(nums_ptrs.at(1) == std::addressof(nums.at(1)));
  REQUIRE(nums_ptrs.at(2) == std::addressof(nums.at(2)));
  REQUIRE(nums_ptrs.at(3) == std::addressof(nums.at(3)));

  for (const auto& num : query2) {
    nums_ptrs.push_back(std::addressof(num));
  }

  REQUIRE(nums_ptrs.size() == 8);
  REQUIRE(nums_ptrs.at(0) == std::addressof(nums.at(0)));
  REQUIRE(nums_ptrs.at(1) == std::addressof(nums.at(1)));
  REQUIRE(nums_ptrs.at(2) == std::addressof(nums.at(2)));
  REQUIRE(nums_ptrs.at(3) == std::addressof(nums.at(3)));
  REQUIRE(nums_ptrs.at(4) == std::addressof(nums.at(0)));
  REQUIRE(nums_ptrs.at(5) == std::addressof(nums.at(1)));
  REQUIRE(nums_ptrs.at(6) == std::addressof(nums.at(2)));
  REQUIRE(nums_ptrs.at(7) == std::addressof(nums.at(3)));
}

TEST_CASE("from(initializer_list)") {
  std::vector<std::string> lines;

  for (auto num : linq::from({1, 2, 3})) {
    lines.push_back(std::to_string(num));
  }

  REQUIRE(lines.size() == 3);
  REQUIRE(lines.at(0) == "1");
  REQUIRE(lines.at(1) == "2");
  REQUIRE(lines.at(2) == "3");

  // initializer_list query to std::vector
  lines = linq::from({1, 2, 3}).select_to_string().to_vector();

  REQUIRE(lines.size() == 3);
  REQUIRE(lines.at(0) == "1");
  REQUIRE(lines.at(1) == "2");
  REQUIRE(lines.at(2) == "3");
}

TEST_CASE("from_to") {
  SECTION("0 to 10 with default step") {
    const auto range = linq::from_to(0, 10);
    REQUIRE(range.count() == 11);

    const std::vector list = range.to_vector();
    REQUIRE(list.size() == 11);
    REQUIRE(list == std::vector{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
  }

  SECTION("0 to 10 with step 2") {
    const auto range = linq::from_to(0, 10, 2);
    REQUIRE(range.count() == 6);

    const std::vector list = range.to_vector();
    REQUIRE(list.size() == 6);
    REQUIRE(list == std::vector{0, 2, 4, 6, 8, 10});
  }

  SECTION("0 to 10 with step 3") {
    const auto range = linq::from_to(0, 10, 3);
    REQUIRE(range.count() == 4);

    const std::vector list = range.to_vector();
    REQUIRE(list.size() == 4);
    REQUIRE(list == std::vector{0, 3, 6, 9});
  }

  struct custom_addable {
    double value = 0.0;

    custom_addable(double value)
        : value(value) {
    }

    bool operator==(const custom_addable&) const = default;
    bool operator!=(const custom_addable&) const = default;

    custom_addable& operator+=(const custom_addable& other) {
      value += other.value;
      return *this;
    }

    bool operator<(const custom_addable& other) const {
      return value < other.value;
    }
  };

  SECTION("custom addable") {
    const auto range = linq::from_to(custom_addable{0}, custom_addable{10});
    REQUIRE(range.count() == 11);

    const std::vector list = range.to_vector();
    REQUIRE(list.size() == 11);
    REQUIRE(list == std::vector{custom_addable{0},
                                custom_addable{1},
                                custom_addable{2},
                                custom_addable{3},
                                custom_addable{4},
                                custom_addable{5},
                                custom_addable{6},
                                custom_addable{7},
                                custom_addable{8},
                                custom_addable{9},
                                custom_addable{10}});
  }
}

TEST_CASE("generate") {
  const auto range = linq::generate([](size_t iteration) {
    if (iteration < 10) {
      return linq::generate_return(iteration * 2);
    }

    return linq::generate_finish<size_t>();
  });

  REQUIRE(range.count() == 10);

  const auto list = range.to_vector();

  REQUIRE(list.size() == 10);
  REQUIRE(list == std::vector<size_t>{0, 2, 4, 6, 8, 10, 12, 14, 16, 18});
}

TEST_CASE("distinct") {
}

TEST_CASE("select") {
}

TEST_CASE("select_many") {
}

TEST_CASE("reverse") {
}

TEST_CASE("take") {
}

TEST_CASE("take_while") {
}

TEST_CASE("skip") {
}

TEST_CASE("skip_while") {
}

TEST_CASE("append") {
  SECTION("simple append of two ranges") {
    std::vector nums1{1, 2, 3, 4};
    std::vector nums2{5, 6, 7, 8};

    auto query1 = linq::from(&nums1);
    auto query2 = linq::from(&nums2);
    auto query3 = query1.append(query2);

    std::vector<int> all_nums = query3.to_vector();

    REQUIRE(all_nums.size() == 8);
    REQUIRE(all_nums.at(0) == 1);
    REQUIRE(all_nums.at(1) == 2);
    REQUIRE(all_nums.at(2) == 3);
    REQUIRE(all_nums.at(3) == 4);
    REQUIRE(all_nums.at(4) == 5);
    REQUIRE(all_nums.at(5) == 6);
    REQUIRE(all_nums.at(6) == 7);
    REQUIRE(all_nums.at(7) == 8);

    nums1.push_back(9);
    nums2.push_back(10);

    all_nums = query3.to_vector();

    REQUIRE(all_nums.size() == 10);
    REQUIRE(all_nums.at(0) == 1);
    REQUIRE(all_nums.at(1) == 2);
    REQUIRE(all_nums.at(2) == 3);
    REQUIRE(all_nums.at(3) == 4);
    REQUIRE(all_nums.at(4) == 9);
    REQUIRE(all_nums.at(5) == 5);
    REQUIRE(all_nums.at(6) == 6);
    REQUIRE(all_nums.at(7) == 7);
    REQUIRE(all_nums.at(8) == 8);
    REQUIRE(all_nums.at(9) == 10);
  }

  SECTION("cyclic append") {
  }
}

TEST_CASE("repeat") {
}

TEST_CASE("join") {
}

TEST_CASE("order_by") {
}

TEST_CASE("order_by_ascending") {
}

TEST_CASE("order_by_descending") {
}

TEST_CASE("then_by") {
}

TEST_CASE("then_by_ascending") {
}

TEST_CASE("then_by_descending") {
}

TEST_CASE("sum") {
}

TEST_CASE("min") {
  constexpr std::array ints{1, -2, 3, -4, 5, -6, 7, -8, 9};

  REQUIRE(linq::from(&ints).min() == -8);
  REQUIRE(linq::from(&ints).where([](int value) { return value >= 3; }).min() == 3);
  REQUIRE(!linq::from(&ints).where([](int value) { return value < -10; }).min().has_value());

  constexpr std::array floats{-0.5f, 0.25f, 0.75f, 1.0f, 5.0f, -20.0f, 3.25f};

  REQUIRE(linq::from(&floats).min() == -20.0f);
  REQUIRE(!linq::from(&floats).where([](float value) { return value > 10; }).min().has_value());
  REQUIRE(linq::from(&floats).where([](float value) { return value > 1; }).min() == 3.25f);

  constexpr std::array doubles{-0.5, 0.25, 0.75, 1.0, 5.0, -20.0, 3.25};

  REQUIRE(linq::from(&doubles).min() == -20.0);
  REQUIRE(!linq::from(&doubles).where([](double value) { return value > 20; }).min().has_value());
  REQUIRE(linq::from(&doubles).where([](double value) { return value > 1; }).min() == 3.25);
}

TEST_CASE("max") {
  constexpr std::array ints{1, -2, 3, -4, 5, -6, 7, -8, 9};

  REQUIRE(linq::from(&ints).max() == 9);
  REQUIRE(linq::from(&ints).where([](int value) { return value >= 3; }).max() == 9);
  REQUIRE(!linq::from(&ints).where([](int value) { return value < -10; }).max().has_value());

  constexpr std::array floats{-0.5f, 0.25f, 0.75f, 1.0f, 5.0f, -20.0f, 3.25f};

  REQUIRE(linq::from(&floats).max() == 5.0f);
  REQUIRE(!linq::from(&floats).where([](float value) { return value > 10; }).max().has_value());
  REQUIRE(linq::from(&floats).where([](float value) { return value < 8; }).max() == 5.0f);

  constexpr std::array doubles{-0.5, 0.25, 0.75, 1.0, 5.0, -20.0, 3.25};

  REQUIRE(linq::from(&doubles).max() == 5.0);
  REQUIRE(!linq::from(&doubles).where([](double value) { return value > 10; }).max().has_value());
  REQUIRE(linq::from(&doubles).where([](double value) { return value < 8; }).max() == 5.0);
}

TEST_CASE("average") {
  constexpr std::array ints{1, 2, 3, 4};

  {
    const auto avg = linq::from(&ints).average().value();
    static_assert(std::is_same_v<std::remove_const_t<decltype(avg)>, long double>);
    REQUIRE(avg == (1.0 + 2.0 + 3.0 + 4.0) / 4.0);
  }

  // Average with custom type
  struct value {
    int num{};

    value operator+(const value& other) const {
      return {.num = num + other.num};
    }

    value operator/(size_t other) const {
      return {.num = static_cast<int>(num / other)};
    }

    value& operator+=(const value& other) {
      num += other.num;
      return *this;
    }

    bool operator==(const value& other) const {
      return num == other.num;
    }
  };

  {
    const std::array vals{value{1}, value{2}, value{3}, value{4}};
    const auto       avg = linq::from(&vals).average().value();
    REQUIRE(avg == value{2});
  }
}

TEST_CASE("aggregate") {
}

TEST_CASE("first") {
}

TEST_CASE("last") {
}

TEST_CASE("any") {
}

TEST_CASE("all") {
}

TEST_CASE("count") {
}

TEST_CASE("element_at") {
}

TEST_CASE("from_copy") {
}

TEST_CASE("to_vector") {
  const std::array  nums{1, 2, 3, 4};
  const std::vector vec = linq::from(&nums).to_vector();

  REQUIRE(vec.size() == 4);
  REQUIRE(vec.at(0) == 1);
  REQUIRE(vec.at(1) == 2);
  REQUIRE(vec.at(2) == 3);
  REQUIRE(vec.at(3) == 4);
}

TEST_CASE("from(span)") {
  const std::array           nums{1, 2, 3, 4};
  const std::span<const int> nums_span{nums};
  const auto                 query = linq::from(nums_span);
  const std::vector          vec   = query.to_vector();
}
