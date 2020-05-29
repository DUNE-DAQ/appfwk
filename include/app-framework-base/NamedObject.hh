#ifndef APP_FRAMEWORK_BASE_NAMEDOBJECT_HH_
#define APP_FRAMEWORK_BASE_NAMEDOBJECT_HH_ 

class NamedObject {
 public:
  NamedObject(const std::string& name) : name_(name) {}
  virtual ~NamedObject() = default;

  const std::string& get_name() const {return name_; }

 private:
  std::string name_;
};

#endif // APP_FRAMEWORK_BASE_NAMEDOBJECT_HH_
