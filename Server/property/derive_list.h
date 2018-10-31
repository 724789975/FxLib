#ifndef __DefiveList_H__
#define __DefiveList_H__

#ifndef __NULL_TYPE_H__
#define __NULL_TYPE_H__
class NullType {};
#endif	//!__NULL_TYPE_H__

namespace DL
{
	template<typename T, typename U>
	class DeriveList : public T, public U
	{
	public:
		typedef U Base;
	};

	template <typename TL> struct Length;
	//{
	//	enum { Value = 1 };
	//};

	template <>
	struct Length<NullType>
	{
		enum { Value = 0 };
	};

	template <typename H, typename T>
	struct Length< DeriveList<H, T> >
	{
		enum { Value = 1 + Length<T>::Value };
	};

	//template <typename T>
	//struct Length<DeriveList<T, NullType> >
	//{
	//	enum { Value = 1 };
	//};

	template <typename TL, unsigned int index> struct TypeAt;

	template <typename H, typename T>
	struct TypeAt<DeriveList<H, T>, 0>
	{
		typedef H Result;
	};

	template <typename H, typename T, unsigned int i>
	struct TypeAt<DeriveList<H, T>, i>
	{
		typedef typename TypeAt<T, i - 1>::Result Result;
	};

	template <typename TL>
	struct TypeAt<TL, 0>
	{
		typedef TL Result;
	};
};

#define DERIDELIST_1(T1) DL::DeriveList<T1,NullType>
#define DERIDELIST_2(T1,T2) DL::DeriveList<T1,DERIDELIST_1(T2)>
#define DERIDELIST_3(T1,T2,T3) DL::DeriveList<T1,DERIDELIST_2(T2,T3)>
#define DERIDELIST_4(T1,T2,T3,T4) DL::DeriveList<T1,DERIDELIST_3(T2,T3,T4)>
#define DERIDELIST_5(T1,T2,T3,T4,T5) DL::DeriveList<T1,DERIDELIST_4(T2,T3,T4,T5)>
#define DERIDELIST_6(T1,T2,T3,T4,T5,T6) DL::DeriveList<T1,DERIDELIST_5(T2,T3,T4,T5,T6)>
#define DERIDELIST_7(T1,T2,T3,T4,T5,T6,T7) DL::DeriveList<T1,DERIDELIST_6(T2,T3,T4,T5,T6,T7)>
#define DERIDELIST_8(T1,T2,T3,T4,T5,T6,T7,T8) DL::DeriveList<T1,DERIDELIST_7(T2,T3,T4,T5,T6,T7,T8)>
#define DERIDELIST_9(T1,T2,T3,T4,T5,T6,T7,T8,T9) DL::DeriveList<T1,DERIDELIST_8(T2,T3,T4,T5,T6,T7,T8,T9)>
#define DERIDELIST_10(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10) DL::DeriveList<T1,DERIDELIST_9(T2,T3,T4,T5,T6,T7,T8,T9,T10)>
#define DERIDELIST_11(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11) DL::DeriveList<T1,DERIDELIST_10(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11)>
#define DERIDELIST_12(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12) DL::DeriveList<T1,DERIDELIST_11(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12)>
#define DERIDELIST_13(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13) DL::DeriveList<T1,DERIDELIST_12(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13)>
#define DERIDELIST_14(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14) DL::DeriveList<T1,DERIDELIST_13(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14)>
#define DERIDELIST_15(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15) DL::DeriveList<T1,DERIDELIST_14(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15)>

#endif // !__DefiveList_H__
