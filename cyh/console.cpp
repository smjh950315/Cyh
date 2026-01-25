#include "console.hpp"
#include "text.hpp"
namespace cyh {
	std::ostream& operator << (std::ostream& _cout, const ch8* u8str) {
		_cout << to_string(u8str);
		return _cout;
	}
	std::ostream& operator << (std::ostream& _cout, const ch16* u16str) {
		_cout << to_string(u16str);
		return _cout;
	}
	std::ostream& operator << (std::ostream& _cout, const ch32* u32str) {
		_cout << to_string(u32str);
		return _cout;
	}
	std::ostream& operator << (std::ostream& _cout, const std::u8string& u8str) {
		_cout << to_string(u8str);
		return _cout;
	}
	std::ostream& operator << (std::ostream& _cout, const std::u16string& u16str) {
		_cout << to_string(u16str);
		return _cout;
	}
	std::ostream& operator << (std::ostream& _cout, const std::u32string& u32str) {
		_cout << to_string(u32str);
		return _cout;
	}
};
#include <cstring>
namespace cyh::console {
	constexpr int console_buffer_length = 128;
#ifdef __WINDOWS__
#define POpen _popen
#define PClose _pclose
#define CMD_GET_PATH "cd"
#else
#define POpen popen
#define PClose pclose
#define CMD_GET_PATH "pwd"
#endif // __WINDOWS__
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-result"
#endif // __GNUC__
	void execute(const char* _command, std::string* recv)
	{
		if (_command == nullptr) return;
		if (recv) {
			std::vector<char> buffer(console_buffer_length);
			auto f = POpen(_command, "r");
			while (fgets(buffer.data(), console_buffer_length, f) != NULL) {
				recv->append(buffer.data());
			}
			PClose(f);
		} else {
			std::system(_command);
		}
	}
	std::string console_path()
	{
		std::string result;
		execute(CMD_GET_PATH, &result);
		if (result.back() == '\n') result.pop_back();
		return result;
	}
	std::string readline()
	{
		std::string result;
		std::getline(std::cin, result);
		return result;
	}
	size_t read_args(std::unordered_map<std::string, std::vector<std::string>>& arg_result, const char* _prefix, int argc, const char** argv)
	{
		if (argc <= 0 || _prefix == nullptr) return 0;
		if (strlen(_prefix) == 0) return 0;
		auto original_argc = arg_result.size();
		std::vector<std::string_view> args{};
		size_t new_arg_count = static_cast<size_t>(argc);
		args.reserve(argc);
		for (size_t i = 0; i < new_arg_count; ++i) {
			args.push_back(argv[i]);
		}
		std::string_view current_arg_name;
		for (size_t i = 0; i < new_arg_count; ++i) {
			if (args[i].starts_with(_prefix)) {
				current_arg_name = args[i].substr(strlen(_prefix));
			} else {
				arg_result[std::string(current_arg_name)].push_back(std::string(args[i]));
			}
		}
		return arg_result.size() - original_argc;
	}
	std::unordered_map<std::string, std::vector<std::string>> read_args(const char* _prefix, int argc, const char** argv)
	{
		std::unordered_map<std::string, std::vector<std::string>> result;
		read_args(result, _prefix, argc, argv);
		return result;
	}
};

