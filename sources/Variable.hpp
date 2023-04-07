#include <string>
using std::string;

class Variable{
    private:
    std::string key;
    std::string value;

    public:
    Variable(std::string key, std::string value);
    ~Variable();
    std::string getKey();
    std::string getValue();
};