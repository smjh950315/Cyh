#include "filesys.hpp"
#include "numeric.hpp"
#include <cyh/console.hpp>
#include <cyh/text.hpp>
#include <algorithm>
#include <cstring>
#include <unordered_map>
using namespace cyh::console;
using namespace cyh;
namespace fs = std::filesystem;
using std::list;
using std::function;
using std::string;
using std::unordered_map;
namespace cyh::filesys::details {
	int read_bytes_unchecked(std::ifstream* ifstr,
							 void* _buff,
							 uintmax_t* io_size) {
		ifstr->read((char*)_buff, *io_size);
		auto readed = ifstr->gcount();
		*io_size = static_cast<uintmax_t>(readed);
		return 0;
	}
	int write_bytes_unchecked(std::fstream* ofstr,
							  const void* _buff,
							  uintmax_t* io_size) {
		auto _beg = static_cast<uintmax_t>(ofstr->tellp());
		ofstr->write((const char*)_buff, *io_size);
		if (ofstr->bad()) {
			*io_size = 0;
			return -1;
		}
		*io_size = static_cast<uintmax_t>(ofstr->tellp()) - _beg;
		return 0;
	}
}
namespace cyh::filesys::details {
	void append_path_impl_t(std::filesystem::path& _path) {}
	void append_path(std::filesystem::path& _path) {}

	struct _buff {
		union {
			struct vec {
				void* vecPtr;
				size_t elemType; // 0 - char, 1 - uint8
			} vec;;
			struct mem {
				void* memPtr;
				size_t memSize;
			} mem;
		} data{};
		size_t isVec{};
	};

	template<class T>
	static uintmax_t read_to_vec_impl_unchecked(std::ifstream* _ifsptr, uintmax_t _beg, uintmax_t _fsize, void* _buffVec, bool* _successPtr) {
		auto& buffer = *((std::vector<T>*)_buffVec);
		auto bsize = buffer.size();
		buffer.resize(bsize + _fsize);
		uintmax_t iolen = _fsize;
		_ifsptr->seekg(static_cast<std::streamoff>(_beg), std::ios::beg);
		auto code = cyh::filesys::details::read_bytes_unchecked(_ifsptr, buffer.data() + bsize, &iolen);
		if (_successPtr)
			*_successPtr = (code == 0);
		buffer.resize(bsize + iolen);
		return iolen;
	}
	static uintmax_t read_to_mem_impl_unchecked(std::ifstream* _ifsptr, uintmax_t _beg, uintmax_t _fsize, void* _buffMem, bool* _successPtr) {
		uintmax_t iolen = _fsize;
		_ifsptr->seekg(static_cast<std::streamoff>(_beg), std::ios::beg);
		auto code = cyh::filesys::details::read_bytes_unchecked(_ifsptr, _buffMem, &iolen);
		if (_successPtr)
			*_successPtr = (code == 0);
		return iolen;
	}
	static uintmax_t read_bytes_to_obj(const std::filesystem::path& _path,
									   uintmax_t _beg, uintmax_t _szmax,
									   _buff* _buffPtr, bool* _isSuccess) {
		fs::directory_entry e(_path);
		std::error_code ec;
		if (!e.exists(ec) || e.is_directory(ec))
			return 0;
		if (ec)
			return 0;
		auto fsize = e.file_size(ec);
		if (ec)
			return 0;

		if (_beg >= fsize)
			return 0;
		if (_szmax != uintmax_t(-1) && _beg + _szmax < fsize) {
			fsize = _szmax;
		} else {
			fsize = fsize - _beg;
		}

		std::ifstream ifs(_path, std::ios::in | std::ios::binary);
		if (!ifs.is_open()) {
			if (_isSuccess)
				*_isSuccess = false;
			return -1;
		}
		if (_buffPtr->isVec) {
			if (_buffPtr->data.vec.elemType == 0) {
				return read_to_vec_impl_unchecked<char>(&ifs, _beg, fsize, _buffPtr->data.vec.vecPtr, _isSuccess);
			} else {
				return read_to_vec_impl_unchecked<uint8>(&ifs, _beg, fsize, _buffPtr->data.vec.vecPtr, _isSuccess);
			}
		} else {
			fsize = std::min(fsize, static_cast<uintmax_t>(_buffPtr->data.mem.memSize));
			return read_to_mem_impl_unchecked(&ifs, _beg, fsize, _buffPtr->data.mem.memPtr, _isSuccess);
		}
	}
	uintmax_t read_bytes_to_vec(const std::filesystem::path& _path,
								uintmax_t _beg, uintmax_t _szmax,
								void* _stdVec, size_t _vecElemType, bool* _isSuccess) {
		struct _buff _buff { .isVec = 1 };
		_buff.data.vec.vecPtr = _stdVec;
		_buff.data.vec.elemType = static_cast<size_t>(_vecElemType);
		return read_bytes_to_obj(_path, _beg, _szmax, &_buff, _isSuccess);
	}
	static uintmax_t read_bytes_to_mem(const std::filesystem::path& _path,
								uintmax_t _beg, uintmax_t _szmax,
								void* _buffer, size_t _length, bool* _isSuccess) {
		struct _buff _buff { .isVec = 0 };
		_buff.data.mem.memPtr = _buffer;
		_buff.data.mem.memSize = _length;
		return read_bytes_to_obj(_path, _beg, _szmax, &_buff, _isSuccess);
	}
}
namespace cyh::filesys {
	int create_file_ex(const std::filesystem::path& _path,
					   bool _overwrite, bool _create_dir_recursize) {
		fs::directory_entry e(_path);
		std::error_code ec;
		if (e.exists()) {
			if (!_overwrite)
				return -1;
		} else {
			auto parentPath = e.path().parent_path();
			if (!fs::exists(parentPath)) {
				if (!_create_dir_recursize)
					return -1;
				fs::create_directories(parentPath, ec);
				if (ec) return -1;
			}
		}
		try {
			std::ofstream{ _path };
			return 0;
		} catch (...) {
			return -1;
		}
	}
	int create_directory_ex(const std::filesystem::path& _path,
							bool _overwrite, bool _create_dir_recursive) {
		if (!exists(_path.parent_path()) && !_create_dir_recursive) return -1;
		fs::directory_entry e(_path);
		std::error_code ec;
		if (e.exists()) {
			if (!_overwrite)
				return -1;
			fs::remove(e.path(), ec);
			if (ec) return -1;
		}
		fs::create_directories(_path, ec);
		return ec.value();
	}
	int copy_file_ex(const std::filesystem::path& _src, const std::filesystem::path& _dst,
					 bool overwrite, bool create_dir_recursive) {
		std::error_code ec;
		if (fs::exists(_dst)) {
			if (!overwrite) return -1;
			fs::remove(_dst, ec);
			if (ec) return -1;
		}
		auto parentDir = _dst.parent_path();
		if (!fs::exists(parentDir)) {
			if (!create_dir_recursive) return -1;
			fs::create_directories(parentDir, ec);
			if (ec) return -1;
		}
		fs::copy(_src, _dst, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
		return ec.value();
	}
	int create_symbol(const std::filesystem::path& _to, const std::filesystem::path& _newlink,
					  bool hard_if_file) {
		if (fs::exists(_newlink) || !fs::exists(_to)) return -1;
		if (!fs::is_directory(_to)) {
			std::error_code ec;
			if (hard_if_file) {
				fs::create_hard_link(_to, _newlink, ec);
			} else {
				fs::create_symlink(_to, _newlink, ec);
			}
			if (ec) {
				return -1;
			}
			return 0;
		} else {
			std::error_code ec;
			fs::create_directory_symlink(_to, _newlink, ec);
			if (ec) {
				return -1;
			}
		}
		return 0;
	}
	int read_bytes(const std::filesystem::path& _path, uintmax_t _fbeg,
				   void* _buff, uintmax_t* io_size) {
		if (!io_size || !_buff) return -1;
		if (*io_size == 0) return 0;
		cyh::filesys::details::_buff _buffStruct{};
		bool succeed = false;
		*io_size = cyh::filesys::details::read_bytes_to_mem(_path, _fbeg, *io_size, _buff, static_cast<uintmax_t>(*io_size), &succeed);
		return succeed ? 0 : -1;
	}
	int write_bytes(const std::filesystem::path& _path, uintmax_t _fbeg,
					const void* _buff, uintmax_t* io_size,
					bool _create, bool _dirOverwrite,
					const void* _fill, uintmax_t _fill_len) {
		fs::directory_entry e(_path);
		fs::path p(_path);
		std::error_code ec;
		std::fstream ofs;
		using file_size_type = decltype(fs::file_size(std::declval<fs::path>()));
		file_size_type fsize = 0;
		if (e.exists()) {
			if (e.is_directory(ec)) {
				if (ec)
					return ec.value();
				if (!_dirOverwrite)
					return -1;
				// if is directory and _dirOverwrite is true, remove it and init filestream
				fs::remove_all(p, ec);
				if (ec)
					return ec.value();
				ofs.open(_path, std::ios::in | std::ios::out | std::ios::binary);
				if (!ofs.is_open())
					return -1;
			} else {
				if (ec)
					return ec.value();
				if (e.is_block_file(ec)) {
					if (ec)
						return ec.value();
					return -1;
				}
				fsize = e.file_size(ec);
				if (ec) {
					return ec.value();
				}
				ofs.open(_path, std::ios::in | std::ios::out | std::ios::binary);
				if (!ofs.is_open())
					return -1;
			}
		} else {
			if (!_create) return -1;
			// if file not exist, create it if _create is true
			auto parentDir = p.parent_path();
			// if parent dir not exist, create it
			if (!fs::exists(parentDir)) {
				fs::create_directories(parentDir, ec);
				if (ec)
					return ec.value();
			}
			{
				// create empty file
				std::ofstream create_ofs(_path, std::ios::binary);
			}
			ofs.open(_path, std::ios::in | std::ios::out | std::ios::binary);
			// if create failed, return -1
			if (!ofs.is_open())
				return -1;
		}
		// fill if _fbeg > fsize until _fbeg
		if (_fbeg > fsize) {
			ofs.seekp(fsize, std::ios::beg);
			uintmax_t current_pos = fsize;
			while (current_pos < _fbeg) {
				auto maxFill = std::min(_fill_len, _fbeg - current_pos);
				if (0 == cyh::filesys::details::write_bytes_unchecked(&ofs, _fill, &maxFill)) {
					current_pos += maxFill;
				} else {
					ofs.close();
					return -1;
				}
			}
		} else {
			ofs.seekp(_fbeg, std::ios::beg);
		}
		auto code = cyh::filesys::details::write_bytes_unchecked(&ofs, _buff, io_size);
		if (_fbeg + *io_size > fsize) {
			fsize = _fbeg + *io_size;
		}
		ofs.seekp(fsize);
		ofs.close();
		return code;
	}
	int remove_entry(const std::filesystem::path& _path) {
		std::error_code ec;
		if (!fs::exists(_path, ec))
			return -1;

		if (fs::is_block_file(_path, ec))
			return -1;

		auto is_dir = fs::is_directory(_path, ec);
		if (ec) return ec.value();
		if (is_dir)
			fs::remove_all(_path, ec);
		else
			fs::remove(_path, ec);
		if (ec)
			return ec.value();
		return 0;
	}

	uintmax_t read_bytes(const std::filesystem::path& _path, void* _buffer, size_t _buffLen, bool* _isSuccess, file_read_options options) {
		return cyh::filesys::details::read_bytes_to_mem(_path, options.begin, options.count, _buffer, _buffLen, _isSuccess);
	}
};
namespace cyh::filesys {
	static void copy_from_to(const entry_status_ex& src, entry_status_ex& dest) {
		dest.fullpath = src.fullpath;
		dest._placeholder = src._placeholder;
	}
	static void move_from_to(entry_status_ex& src, entry_status_ex& dest) {
		dest.fullpath = std::move(src.fullpath);
		dest._placeholder = src._placeholder;
		src._placeholder = {};
	}
	entry_status_ex::entry_status_ex(const entry_status_ex& other) {
		copy_from_to(other, *this);
	}
	entry_status_ex::entry_status_ex(entry_status_ex&& other) noexcept {
		move_from_to(other, *this);
	}
	entry_status_ex& entry_status_ex::operator=(const entry_status_ex& other) {
		copy_from_to(other, *this);
		return *this;
	}
	entry_status_ex& entry_status_ex::operator=(entry_status_ex&& other) noexcept {
		move_from_to(other, *this);
		return *this;
	}
	static void copy_from_to(const entry_compare_result& src, entry_compare_result& dest) {
		memcpy(dest._placeholder, src._placeholder, sizeof(src._placeholder));
	}
	static void move_from_to(entry_compare_result& src, entry_compare_result& dest) {
		memcpy(dest._placeholder, src._placeholder, sizeof(src._placeholder));
		memset(src._placeholder, 0, sizeof(src._placeholder));
	}
	bool entry_compare_result::is_changed() const {
		return this->details.is_created || this->details.is_deleted || this->details.is_modified;
	}
	entry_compare_result::entry_compare_result() {}
	entry_compare_result::entry_compare_result(const entry_compare_result& other) {
		copy_from_to(other, *this);
	}
	entry_compare_result::entry_compare_result(entry_compare_result&& other) noexcept {
		move_from_to(other, *this);
	}
	entry_compare_result& entry_compare_result::operator=(const entry_compare_result& other) {
		copy_from_to(other, *this);
		return *this;
	}
	entry_compare_result& entry_compare_result::operator=(entry_compare_result&& other) noexcept {
		move_from_to(other, *this);
		return *this;
	}

	entry_compare_result entry_compare_result::compare(const entry_status_ex& lhs, const entry_status_ex& rhs, void* _cmp_buffer, size_t _cmp_buffer_length) {
		constexpr size_t _bufferSize = 4096;
		char _lbuffer[_bufferSize]{};
		char _rbuffer[_bufferSize]{};

		size_t bufferSize;
		char* lbuffer, * rbuffer;

		if (_cmp_buffer && _cmp_buffer_length > 8192) {
			bufferSize = _cmp_buffer_length / 2;
			lbuffer = (char*)_cmp_buffer;
			rbuffer = (char*)_cmp_buffer + bufferSize;
		} else {
			bufferSize = _bufferSize;
			lbuffer = _lbuffer;
			rbuffer = _rbuffer;
		}

		entry_compare_result result_{};
		auto& result = result_.details;
		result.size_diff = rhs.info.size - lhs.info.size;
		result.is_created = 0 == lhs.info.is_exist && 0 != rhs.info.is_exist;
		result.is_deleted = 0 != lhs.info.is_exist && 0 == rhs.info.is_exist;
		if (lhs.info.is_exist && rhs.info.is_exist) {
			if (!lhs.info.is_directory && !rhs.info.is_directory) {
				if (lhs.info.size != rhs.info.size) {
					result.is_modified = true;
				} else {
					std::ifstream lfs{ lhs.fullpath.c_str(), std::ios::binary | std::ios::in };
					std::ifstream rfs{ rhs.fullpath.c_str(), std::ios::binary | std::ios::in };
					size_t file_size = lhs.info.size;
					if (lfs.is_open() && rfs.is_open()) {
						result.is_modified = false;
						size_t current_read = 0;
						while (current_read < file_size) {
							size_t to_read = std::min(bufferSize, file_size - current_read);
							lfs.read((char*)lbuffer, to_read);
							rfs.read((char*)rbuffer, to_read);
							if (lfs.gcount() != rfs.gcount()) {
								result.is_modified = true;
								break;
							}
							if (memcmp(lbuffer, rbuffer, to_read) != 0) {
								result.is_modified = true;
								break;
							}
							current_read += to_read;
						}
					} else {
						result.is_modified = true;
					}
				}
			} else if (lhs.info.is_directory && rhs.info.is_directory) {
				result.is_modified = false;
			} else {
				result.is_difftype = true;
				result.is_modified = true;
			}
		}
		return result_;
	}

	entry_compare_result entry_compare_result::compare(const std::filesystem::path& lhs, const std::filesystem::path& rhs) {
		entry_status_ex lstat = get_status(lhs);
		entry_status_ex rstat = get_status(rhs);
		return compare(lstat, rstat);
	}

	template<class _It, class _Elem>
	void run_fs_iterator(const fs::path& _path, list<_Elem>& dest,
						 const std::function<_Elem(const fs::directory_entry&)>& _result_callback,
						 const std::function<bool(const fs::directory_entry&)> _entry_filter,
						 const std::function<bool(const _Elem&)>& _result_filter) {
		_It _it(_path);
		for (const fs::directory_entry& elem : _it) {
			if (_entry_filter) {
				if (!_entry_filter(elem))
					continue;
			}
			auto e = _result_callback(elem);
			if (_result_filter) {
				if (!_result_filter(e))
					continue;
			}
			dest.push_back(e);
		}
	}

	template<class T>
	int get_entry_(const fs::path& _path, list<T>& dest,
				   const function<T(const fs::directory_entry&)>& _result_callback,
				   const std::function<bool(const fs::directory_entry&)> _entry_filter,
				   const function<bool(const T&)>& _result_filter, bool _recursive) {
		if (!fs::exists(_path)) return -1;
		size_t initSize = dest.size();
		if (_recursive) {
			run_fs_iterator<fs::recursive_directory_iterator>(_path, dest, _result_callback, _entry_filter, _result_filter);
		} else {
			run_fs_iterator<fs::directory_iterator>(_path, dest, _result_callback, _entry_filter, _result_filter);
		}
		return 0;
	}

	static bool is_equal_status_info(const entry_status_ex& lhs, const entry_status_ex& rhs) {
		return lhs._placeholder.hi == rhs._placeholder.hi
			&& lhs._placeholder.mi == rhs._placeholder.mi
			&& lhs._placeholder.lo == rhs._placeholder.lo;
	}

	entry_status_ex get_status(const fs::path& _path) {
		return get_status(fs::directory_entry{ _path });
	}

	entry_status_ex get_status(const std::filesystem::directory_entry& ent) {
		entry_status_ex result{};
		result.info.is_exist = false;
		if (!ent.exists()) return result;
		result.info.is_exist = true;
		result.info.is_directory = ent.is_directory();
		result.info.size = ent.file_size();
		result.info.updated = ent.last_write_time().time_since_epoch().count();
		result.fullpath = ent.path();
		return result;
	}

	int get_subentries_status(const fs::path& _path, list<entry_status_ex>& dest, bool _recursive, const function<bool(const entry_status_ex&)>& _result_filter) {
		std::function<entry_status_ex(const fs::directory_entry&)> _result_callback = [](const fs::directory_entry& e) {
			return get_status(e.path());
			};
		std::function<bool(const fs::directory_entry&)> entry_filter = [](const fs::directory_entry& e) { return true; };
		return get_entry_(_path, dest, _result_callback, entry_filter, _result_filter, _recursive);
	}

	int get_subentries_path(const fs::path& _path, list<fs::path>& dest, bool _recursive, bool _related, function<bool(const fs::directory_entry&)> filter) {
		std::function<fs::path(const fs::directory_entry&)> _result_callback;
		using native_string_type = std::filesystem::path::string_type;
		native_string_type temp;
		if (_related) {
			temp = _path.native();
			auto ncstr = temp.c_str();
			_result_callback = [=](const fs::directory_entry& e) {
				return fs::relative(e.path(), ncstr);
				};
		} else {
			_result_callback = [=](const fs::directory_entry& e) {
				return e.path();
				};
		}
		std::function<bool(const fs::path&)> result_filter = [](const fs::path&) { return true; };
		return get_entry_(_path, dest, _result_callback, filter, result_filter, _recursive);
	}

	int compare_entry(entry_compare_result& result, const entry_status_ex& lhs, const entry_status_ex& rhs) {
		result = entry_compare_result::compare(lhs, rhs);
		return 0;
	}

	int compare_entry(entry_compare_result& result, const std::filesystem::path& lhs, const std::filesystem::path& rhs) {
		return compare_entry(result, get_status(lhs), get_status(rhs));
	}

	void emplace_entry_status_result(std::unordered_map<std::filesystem::path, entry_compare_result>& _result,
									 const std::filesystem::path& _fullpath,
									 const std::filesystem::path& _root,
									 entry_compare_result&& _cr) {
		_result.emplace(fs::relative(_fullpath, _root), std::move(_cr));
	}

	void emplace_entry_status_result2(std::unordered_map<std::filesystem::path, entry_compare_result>& _result,
									  const std::filesystem::path& rpath,
									  entry_compare_result&& _cr) {
		_result.emplace(rpath, std::move(_cr));
	}

	int compare_entries(std::unordered_map<std::filesystem::path, entry_compare_result>& result,
						const std::filesystem::path& lpath, const std::list<entry_status_ex>& lhs,
						const std::filesystem::path& rpath, const std::list<entry_status_ex>& rhs) {
		using native_string_type = std::filesystem::path::string_type;
		using native_string_view = std::basic_string_view<typename native_string_type::value_type>;

#ifdef __WINDOWS__
		using map_key_type = native_string_type;
		static auto get_key_of_map = [](const native_string_view& str) {
			native_string_type s(str.data(), str.length());
			cyh::text::to_lower_case(&s);
			return s;
			};
#else
		using map_key_type = native_string_view;
		static auto get_key_of_map = [](const native_string_view& str) {
			return str;
			};
#endif
		auto initSize = result.size();
		auto& lroot = lpath.native();
		auto& rroot = rpath.native();
		std::unordered_map<map_key_type, const entry_status_ex*> map_relatedpath_statPtr;
		std::vector<char> buffer;
		constexpr size_t bufferSize = size_t(8) * 1024 * 1024;
		buffer.resize(bufferSize);

		for (const auto& l : lhs) {
			auto lpath_view = native_string_view(l.fullpath.native());
			if (lpath_view.find(lroot) != 0) continue;
			map_relatedpath_statPtr[get_key_of_map(lpath_view.substr(lroot.length()))] = &l;
		}

		for (const auto& r : rhs) {
			auto rpath_view = native_string_view(r.fullpath.native());
			if (rpath_view.find(rroot) != 0) continue;
			auto r_path = get_key_of_map(rpath_view.substr(rroot.length()));
			auto fnd = map_relatedpath_statPtr.find(r_path);
			if (fnd == map_relatedpath_statPtr.end()) {
				emplace_entry_status_result(result, r.fullpath, rpath,
											std::move(entry_compare_result::compare(entry_status_ex{}, r, buffer.data(), buffer.capacity())));
			} else {
				emplace_entry_status_result(result, r.fullpath, rpath,
											entry_compare_result::compare(*fnd->second, r, buffer.data(), buffer.capacity()));
				map_relatedpath_statPtr.erase(r_path);
			}
		}

		for (const auto& pair : map_relatedpath_statPtr) {
			result.emplace(pair.first, std::move(entry_compare_result::compare(*pair.second, entry_status_ex{}, buffer.data(), buffer.capacity())));
		}

		return 0;
	}

	int compare_entries(std::unordered_map<std::filesystem::path, entry_compare_result>& result, const std::filesystem::path& lpath, const std::filesystem::path& rpath) {
		std::list<entry_status_ex> lhs, rhs;
		get_subentries_status(lpath, lhs, true);
		get_subentries_status(rpath, rhs, true);
		return compare_entries(result, lpath, lhs, rpath, rhs);
	}
};
