#pragma once
#include "typedef.hpp"
#include "exceptions.hpp"
#include "bstream.hpp"
#include <fstream>
#include <filesystem>
#include <functional>
#include <list>
#include <unordered_map>
namespace cyh {
	namespace filesys {
		struct file_read_options {
			uintmax_t begin{ 0 };
			uintmax_t count{ uintmax_t(-1) };
		};
		struct file_write_options {
			uintmax_t begin{ 0 };
			uintmax_t count{ uintmax_t(-1) };
			bool fill{};
			bool create{};
			bool overwrite{};
		};
		namespace details {
			void append_path_impl_t(std::filesystem::path& _path);
			void append_path(std::filesystem::path& _path);

			template<class T>
			void append_path_impl_sv_t(std::filesystem::path& _path, const std::basic_string_view<T>& _sv) {
				std::basic_string_view<T> sv = _sv;
				if (sv.starts_with(static_cast<T>(std::filesystem::path::preferred_separator)))
					sv.remove_prefix(1);
				_path /= sv;
			}

			template<class T, class V>
			constexpr bool is_construct_sv_v = std::is_constructible_v<std::basic_string_view<T>, V>;

			template<class T, class V>
			void append_path_impl_sv_(std::filesystem::path& _path, const V& v) requires(!std::is_constructible_v<std::basic_string_view<T>, V>) {}
			template<class T, class V>
			void append_path_impl_sv_(std::filesystem::path& _path, const V& v) requires(std::is_constructible_v<std::basic_string_view<T>, V>) {
				append_path_impl_sv_t(_path, std::basic_string_view<T>(v));
			}

			template<class T>
			static void append_path_impl_t(std::filesystem::path& _path, const T& val) requires(std::is_same_v<T, std::filesystem::path>) {
				_path /= val;
			}
			template<class T>
			static void append_path_impl_t(std::filesystem::path& _path, const T& val) requires(!std::is_same_v<T, std::filesystem::path>) {
				if constexpr (is_construct_sv_v<char, T>) {
					append_path_impl_sv_<char>(_path, val);
				} else if constexpr (is_construct_sv_v<wchar_t, T>) {
					append_path_impl_sv_<char>(_path, val);
				} else if constexpr (is_construct_sv_v<char8_t, T>) {
					append_path_impl_sv_<char8_t>(_path, val);
				} else {
					append_path_impl_sv_<char>(_path, val);
				}
			}
			template<class T, class ...Xs>
			static void append_path(std::filesystem::path& _path, const T& _rpath, Xs..._paths) {
				append_path_impl_t(_path, _rpath);
				append_path(_path, _paths...);
			}

			int read_bytes_unchecked(std::ifstream* ifstr,
									 void* _buff,
									 uintmax_t* io_size);
			int write_bytes_unchecked(std::fstream* ofstr,
									  const void* _buff,
									  uintmax_t* io_size);

			// _vecElemType: 0 - char, 1 - uint8
			uintmax_t read_bytes_to_vec(const std::filesystem::path& _path,
										uintmax_t _beg, uintmax_t _szmax, 
										void* _stdVec, size_t _vecElemType,
										bool* _isSuccess);
		};

		int create_file_ex(const std::filesystem::path& _path,
						   bool _overwrite, bool _create_dir_recursize);
		int create_directory_ex(const std::filesystem::path& _path,
								bool _overwrite, bool _create_dir_recursive);
		int copy_file_ex(const std::filesystem::path& _src, const std::filesystem::path& _dst,
						 bool overwrite, bool create_dir_recursive);
		int create_symbol(const std::filesystem::path& _to, const std::filesystem::path& _newlink,
						  bool hard_if_file);
		int read_bytes(const std::filesystem::path& _path, uintmax_t _fbeg,
					   void* _buff, uintmax_t* io_size);
		int write_bytes(const std::filesystem::path& _path, uintmax_t _fbeg,
						const void* _buff, uintmax_t* io_size,
						bool _create, bool _dirOverwrite,
						const void* _fill = nullptr, uintmax_t _fill_len = 0);
		int remove_entry(const std::filesystem::path& _path);

		uintmax_t read_bytes(const std::filesystem::path& _path, void* _buffer, size_t _buffLen, bool* _isSuccess = 0, file_read_options options = {});
		template<class T>
		uintmax_t read_bytes(const std::filesystem::path& _path, std::vector<T>& buffer, bool* _isSuccess = 0, file_read_options options = {}) {
			static_assert(std::is_same_v<std::decay_t<T>, char> || std::is_same_v<std::decay_t<T>, uint8>);
			if constexpr (std::is_same_v<std::decay_t<T>, char>) {
				return cyh::filesys::details::read_bytes_to_vec(_path, options.begin, options.count, &buffer, 0, _isSuccess);
			} else {
				return cyh::filesys::details::read_bytes_to_vec(_path, options.begin, options.count, &buffer, 1, _isSuccess);
			}
		}
		template<class T>
		std::vector<T> read_bytes(const std::filesystem::path& _path, bool* _isSuccess = 0, file_read_options options = {}) {
			std::vector<T> result;
			read_bytes<T>(_path, result, _isSuccess, options);
			return result;
		}
		
		// Cached information of std::filesystem::directory_entry
		struct entry_status_ex {
			std::filesystem::path fullpath{};
			union {
				struct {
					uint64 hi{}, mi{}, lo{};
				} _placeholder{};
				struct {
					int64 updated{};
					uint64 size;
					int is_exist;
					int is_directory;
				} info;
			};
			entry_status_ex() {}
			entry_status_ex(const entry_status_ex& other);
			entry_status_ex(entry_status_ex&& other) noexcept;
			entry_status_ex& operator=(const entry_status_ex& other);
			entry_status_ex& operator=(entry_status_ex&& other) noexcept;
			~entry_status_ex() {}
		};

		struct entry_compare_result {
			union {
				uint64 _placeholder[3];
				struct {
					int64 size_diff;
					int is_deleted;
					int is_modified;
					int is_created;
					int is_difftype;
				} details{};
			};
			bool is_changed() const;
			entry_compare_result();
			entry_compare_result(const entry_compare_result&);
			entry_compare_result(entry_compare_result&&) noexcept;
			entry_compare_result& operator=(const entry_compare_result&);
			entry_compare_result& operator=(entry_compare_result&&) noexcept;
			~entry_compare_result() {}
			static entry_compare_result compare(const entry_status_ex& lhs, const entry_status_ex& rhs, void* _cmp_buffer = 0, size_t _cmp_buffer_length = 0);
			static entry_compare_result compare(const std::filesystem::path& lhs, const std::filesystem::path& rhs);
		};

		entry_status_ex get_status(const std::filesystem::path& _path);

		entry_status_ex get_status(const std::filesystem::directory_entry& _ent);

		int get_subentries_status(const std::filesystem::path& _path, std::list<entry_status_ex>& dest, bool _recursive = false,
								  const std::function<bool(const entry_status_ex&)>& _result_filter = {});

		int get_subentries_path(const std::filesystem::path& _path, std::list<std::filesystem::path>& dest, bool _recursive, bool _related = false,
								std::function<bool(const std::filesystem::directory_entry&)> filter = {});

		int compare_entry(entry_compare_result& result, const entry_status_ex& lhs, const entry_status_ex& rhs);

		int compare_entry(entry_compare_result& result, const std::filesystem::path& lhs, const std::filesystem::path& rhs);

		int compare_entries(std::unordered_map<std::filesystem::path, entry_compare_result>& result,
							const std::filesystem::path& lpath, const std::list<entry_status_ex>& lhs,
							const std::filesystem::path& rpath, const std::list<entry_status_ex>& rhs);

		int compare_entries(std::unordered_map<std::filesystem::path, entry_compare_result>& result, const std::filesystem::path& lpath, const std::filesystem::path& rpath);


		template <typename... Args>
		std::filesystem::path new_path(const std::filesystem::path& p, Args... args) {
			std::filesystem::path result = p;
			cyh::filesys::details::append_path(result, std::forward<Args>(args)...);
			return result;
		}
	};

	class file_stream : public cyh::iobstream_<std::fstream> {
	public:
		file_stream() {}
		file_stream(const std::filesystem::path& _path, std::fstream::openmode _openmode = std::ios::in | std::ios::out | std::ios::binary) {
			this->open(_path, _openmode);
		}
		void open(const std::filesystem::path& _path, std::fstream::openmode _openmode = std::ios::in | std::ios::out | std::ios::binary) {
			this->open_stream(_path.string(), _openmode);
		}
	};
};
