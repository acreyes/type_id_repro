#ifndef SIMPLE_PARAMS_HPP_
#define SIMPLE_PARAMS_HPP_

#include <cstring>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>

// Simplified version of Parthenon's Params class
// Key feature: Uses templates + std::type_index for runtime type checking
class SimpleParams {
 private:
  // Base type for type erasure (like Params::base_t)
  struct base_t {
    virtual ~base_t() = default;
  };

  // Template wrapper that stores the actual value (like Params::object_t<T>)
  template <typename T>
  struct object_t : base_t {
    std::unique_ptr<T> pValue;
    explicit object_t(T val) : pValue(std::make_unique<T>(val)) {}
    ~object_t() = default;
  };

  std::map<std::string, std::unique_ptr<base_t>> values_;
  std::map<std::string, std::type_index> types_;

 public:
  SimpleParams() = default;
  SimpleParams(const SimpleParams &) = delete;
  SimpleParams &operator=(const SimpleParams &) = delete;

  // Add a value with type tracking (like Params::Add)
  template <typename T>
  void Add(const std::string &key, T value) {
    if (values_.find(key) != values_.end()) {
      throw std::runtime_error("Key '" + key + "' already exists");
    }
    values_[key] = std::make_unique<object_t<T>>(value);
    types_.emplace(key, std::type_index(typeid(T)));
  }

  // Get a value with type checking (like Params::Get)
  // This is where the typeinfo comparison happens!
  template <typename T>
  const T &Get(const std::string &key) const {
    auto it = values_.find(key);
    if (it == values_.end()) {
      throw std::runtime_error("Key '" + key + "' doesn't exist");
    }

#ifdef USE_STRCMP_FIX
    // WORKAROUND: Compare mangled type names as strings
    // This works across library boundaries even when typeinfo symbols differ
    if (types_.at(key) != std::type_index(typeid(T)) &&
        std::strcmp(types_.at(key).name(), std::type_index(typeid(T)).name()) != 0) {
      throw std::runtime_error("WRONG TYPE FOR KEY '" + key +
                               "' (stored: " + types_.at(key).name() +
                               ", requested: " + std::type_index(typeid(T)).name() + ")");
    }
#else
    // ORIGINAL: Direct typeinfo comparison (fails across library boundaries!)
    if (types_.at(key) != std::type_index(typeid(T))) {
      throw std::runtime_error("WRONG TYPE FOR KEY '" + key +
                               "' (stored: " + types_.at(key).name() +
                               ", requested: " + std::type_index(typeid(T)).name() + ")");
    }
#endif

    auto typed_ptr = dynamic_cast<object_t<T> *>(it->second.get());
    if (!typed_ptr) {
      throw std::runtime_error("Dynamic cast failed for key '" + key + "'");
    }
    return *typed_ptr->pValue;
  }
};

#endif  // SIMPLE_PARAMS_HPP_
