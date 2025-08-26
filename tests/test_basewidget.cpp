#include "compose/widgets/BaseWidget.h"
#include "lvgl.h"
#include <catch2/catch_all.hpp>
#include <string>
#include <compose/widgets/Window.h>

TEST_CASE("BaseWidget LVGL user data storage", "[BaseWidget]")
{
  lv_init();
  Compose::Window w { Compose::Window::Backend::SDL, {} };

  lv_obj_t* obj = lv_obj_create(nullptr);
  REQUIRE(obj != nullptr);

  BaseWidget widget(obj);

  SECTION("Store and retrieve string data")
  {
    std::string testKey = "test_key";
    std::string testValue = "test_value";

    auto& data = widget.ensureDataForKeyExistsOwning<std::string>(testKey);
    data = testValue;

    auto* retrievedData = widget.getData<std::string>(testKey);
    REQUIRE(retrievedData != nullptr);
    REQUIRE(*retrievedData == testValue);
  }

  SECTION("Store and retrieve integer data")
  {
    std::string testKey = "int_key";
    int testValue = 42;

    auto& data = widget.ensureDataForKeyExistsOwning<int>(testKey);
    data = testValue;

    auto* retrievedData = widget.getData<int>(testKey);
    REQUIRE(retrievedData != nullptr);
    REQUIRE(*retrievedData == testValue);
  }

  SECTION("Non-owning data storage")
  {
    std::string testKey = "non_owning_key";
    std::string* testValue = new std::string("non_owning_value");

    auto& data = widget.ensureDataForKeyExistsNonOwning<std::string>(testKey, [testValue]() { return testValue; });

    auto* retrievedData = widget.getData<std::string>(testKey);
    REQUIRE(retrievedData != nullptr);
    REQUIRE(*retrievedData == "non_owning_value");
  }

  SECTION("Get non-existent data returns nullptr")
  {
    auto* retrievedData = widget.getData<std::string>("non_existent_key");
    REQUIRE(retrievedData == nullptr);
  }

  SECTION("Multiple data entries")
  {
    auto& data1 = widget.ensureDataForKeyExistsOwning<int>("key1");
    auto& data2 = widget.ensureDataForKeyExistsOwning<std::string>("key2");

    data1 = 123;
    data2 = "hello";

    auto* retrieved1 = widget.getData<int>("key1");
    auto* retrieved2 = widget.getData<std::string>("key2");

    REQUIRE(retrieved1 != nullptr);
    REQUIRE(retrieved2 != nullptr);
    REQUIRE(*retrieved1 == 123);
    REQUIRE(*retrieved2 == "hello");
  }
}