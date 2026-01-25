#pragma once
#include "my_vec.hpp"
namespace cyh::container {
	struct POS_FLAGS {
		static constexpr uint32 Mask = 0x000000FF;
		enum RELATIVE_POS : uint32 {
			NONE = 0b0000,
			LEFT = 0b0110,
			RIGHT = 0b1100,
			// 0b0110 0000
			TOP = LEFT << 4,
			// 0b1100 0000
			BOTTOM = RIGHT << 4,
		};
		static constexpr bool HasFlag(RELATIVE_POS flags, RELATIVE_POS flag) { return (flags & flag) ^ flag; }
	};

	template<class T>
	struct Size2D_ : MyVec2<T> {
		constexpr Size2D_() {}
		constexpr Size2D_(T w, T h) : MyVec2<T>(w, h) {}
		constexpr T area() const { return this->x * this->y; }
	};
	template<class T>
	struct Point2D_ : MyVec2<T> {
		constexpr Point2D_() {}
		constexpr Point2D_(T x, T y) : MyVec2<T>(x, y) {}
		constexpr int offset(const Size2D_<T>& sz, int chnn) const {
			if (chnn == 0 || sz.area() == 0) return 0;
			return sz.w * chnn * (this->y) + chnn * (this->x);
		}
		constexpr POS_FLAGS::RELATIVE_POS PosRelateTo(const Point2D_<T>& other) const {
			uint32 flag{};
			auto relateX = this->x - other.x;
			auto relateY = this->y - other.y;
			if (relateX < 0) {
				flag |= POS_FLAGS::LEFT;
			} else if (relateX > 0) {
				flag |= POS_FLAGS::RIGHT;
			}
			if (relateY < 0) {
				flag |= POS_FLAGS::TOP;
			} else if (relateY > 0) {
				flag |= POS_FLAGS::BOTTOM;
			}
			return (POS_FLAGS::RELATIVE_POS)flag;
		}
	};

	template<class T>
	constexpr bool operator >(const Size2D_<T>& lhs, const Size2D_<T>& rhs) { return lhs.area() > rhs.area(); }
	template<class T>
	constexpr bool operator <(const Size2D_<T>& lhs, const Size2D_<T>& rhs) { return lhs.area() < rhs.area(); }
	template<class T>
	constexpr bool operator >=(const Size2D_<T>& lhs, const Size2D_<T>& rhs) { return !(lhs < rhs); }
	template<class T>
	constexpr bool operator <=(const Size2D_<T>& lhs, const Size2D_<T>& rhs) { return !(lhs > rhs); }

	template<class T>
	struct Rect_ : MyVec4<T> {
		static constexpr Rect_<T> GetMaxRectOnMain(const Rect_<T>& main, const Rect_<T>& sub) {
			if (main.area() == 0 || sub.area() == 0) { return Rect_<T>{ sub.Begin() }; }
			T mx1 = main.x, mx2 = main.w + mx1 - 1;
			T my1 = main.y, my2 = main.h + my1 - 1;
			T sx1 = sub.x, sx2 = sub.w + sx1 - 1;
			T sy1 = sub.y, sy2 = sub.h + sy1 - 1;
			if (sx2 < mx1 || sy2 < my1 || sx1 > mx2 || sy1 > my2) { return Rect_<T>{}; }
			T x1 = numeric::get_inrange(sx1, mx1, mx2);
			T y1 = numeric::get_inrange(sy1, my1, my2);
			T x2 = numeric::get_inrange(sx2, mx1, mx2);
			T y2 = numeric::get_inrange(sy2, my1, my2);
			return Rect_<T>{ x1, y1, x2 - x1 + 1, y2 - y1 + 1 };
		}

		constexpr Rect_() {}
		constexpr Rect_(T x, T y, T w, T h) : MyVec4<T>(x, y, w, h) {}
		constexpr Rect_(const Point2D_<T>& pt) : MyVec4<T>(pt, Size2D_<T>{}) {}
		constexpr Rect_(const Point2D_<T>& pt, const Size2D_<T>& sz) : MyVec4<T>(pt, sz) {}
		constexpr T area() const { return this->w * this->h; }

		constexpr Point2D_<T> Begin() const { return Point2D_<T>(this->x, this->y); }
		constexpr Point2D_<T> End() const { return Point2D_<T>(this->x, this->y + this->h); }
		constexpr Point2D_<T> LeftTop() const { return this->Begin(); }
		// 右下角"座標"，(開始的位置 + 長度 - 1)，因此必須先檢查 Area 是否為 0
		constexpr Point2D_<T> RightBottom() const { return Point2D_<T>(this->x + this->w - 1, this->y + this->h - 1); }
		constexpr Size2D_<T> Size() const { return Size2D_<T>(this->w, this->h); }
	};

	template<class T>
	constexpr bool operator >(const Rect_<T>& lhs, const Rect_<T>& rhs) { return lhs.Size() > rhs.Size(); }
	template<class T>
	constexpr bool operator <(const Rect_<T>& lhs, const Rect_<T>& rhs) { return lhs.Size() < rhs.Size(); }
	template<class T>
	constexpr bool operator >=(const Rect_<T>& lhs, const Rect_<T>& rhs) { return !(lhs < rhs); }
	template<class T>
	constexpr bool operator <=(const Rect_<T>& lhs, const Rect_<T>& rhs) { return !(lhs > rhs); }

	using Rect = Rect_<int>;
	using Size2D = Size2D_<int>;
	using Point2D = Point2D_<int>;
};
