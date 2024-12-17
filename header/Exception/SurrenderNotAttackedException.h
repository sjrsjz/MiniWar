#include <exception>
#include <string>

class SurrenderNotAttackedException : public std::exception {
	std::string message;
public:
	SurrenderNotAttackedException(std::string message) : message(message) {}
	const char* what() const throw() {
		return message.c_str();
	}
};
