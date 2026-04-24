#ifndef SIMPLE_TYPE_HPP_
#define SIMPLE_TYPE_HPP_

// Mimics Parthenon's MetadataFlag - a simple non-template class
// that will be used as a template parameter elsewhere
class SimpleFlag {
private:
  int value_;

public:
  explicit SimpleFlag(int v);
  int value() const;
};

#endif // SIMPLE_TYPE_HPP_
