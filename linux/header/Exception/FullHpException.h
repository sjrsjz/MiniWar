#include <exception>
#include <string>

class FullHpException : public std::exception {
private:
	std::string message;
public:
	explicit FullHpException(std::string message) : message(message) {}
	const char* what() const noexcept override {
		return message.c_str();
	}
};
