#pragma once
#include <cyh/typedef.hpp>
#include <cyh/exceptions.hpp>
#include <cyh/container.hpp>
#include <cyh/numeric.hpp>
#include <cyh/cloneable.hpp>
#include <vector>
namespace cyh::table {
	class itable {
		itable* no_const() const { return const_cast<itable*>(this); }
	public:
		using size_type = uint64;

		virtual std::vector<std::string> column_names() const { return std::vector<std::string>(this->col_size()); }
		virtual std::vector<std::vector<cloneable>> row_datas(size_type _rowBegin, size_type _rowCount, int _colBegin, int _colCount) const {
			auto rowSize = this->row_size();
			if (_rowBegin >= rowSize)
				return {};
			auto maxRow = std::min(rowSize, (_rowBegin + _rowCount));
			std::vector<std::vector<cloneable>> result;
			while (_rowBegin < maxRow) {
				result.push_back(row_data(_rowBegin, _colBegin, _colCount));
				++_rowBegin;
			}
			return result;
		}
		virtual std::vector<std::string> stringified_row_data(size_type _rowIndex, int _colBeg = 0, int _colCount = numeric::max<int>()) const {
			auto rowData = this->row_data(_rowIndex, _colBeg, _colCount);
			auto colCount = rowData.size();
			if (colCount == 0) return {};
			std::vector<std::string> result;
			result.resize(colCount);
			for (auto col = 0; col < colCount; ++col) {
				result[col] = rowData[col].to_string();
			}
			return result;
		}
		virtual std::vector<std::vector<std::string>> stringified_row_datas(size_type _rowIndexBeg, size_type _rowCount, int _colIndexBeg = 0, int _colCount = numeric::max<int>()) const {
			std::vector<std::vector<std::string>> result;
			auto currentIndex = _rowIndexBeg;
			auto maxRowIndex = _rowIndexBeg + _rowCount;
			for (; currentIndex < maxRowIndex; ++currentIndex) {
				auto rowValues = this->stringified_row_data(currentIndex, _colIndexBeg, _colCount);
				if (rowValues.size() == 0) break;
				result.push_back(rowValues);
			}
			return result;
		}

		virtual size_type row_size() const = 0;
		virtual size_type col_size() const = 0;
		virtual std::vector<cloneable> row_data(size_type _rowIndex, int _colBegin, int _colCount) const = 0;

		virtual ~itable() {}
	};
};
