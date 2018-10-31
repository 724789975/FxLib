/**
* TL::Typelists将来自外星球的强大威力带到C++中，让我们体验这种强大吧－－摘自Modern C++ Design
*
*/

#ifndef __TYPELIST_H__
#define __TYPELIST_H__

#ifndef __NULL_TYPE_H__
#define __NULL_TYPE_H__
class NullType {};
#endif	//!__NULL_TYPE_H__

namespace TL
{
	/**
	* \brief 以下几行，就是来自火星的代码
	*
	*/
	template <typename H, typename T>
	struct Typelist
	{
		typedef H Head;
		typedef T Tail;
	};


	/**
	* \brief 计算Typelist的长度
	*
	*/
	template <typename TL> struct Length
	{
		enum { value = 1 };
	};
	template <> struct Length<NullType>
	{
		enum { value = 0 };
	};

	template <typename H, typename T> struct Length< Typelist<H, T> >
	{
		enum { value = 1 + Length<T>::value };
	};


	/**
	* \brief 按索引值，取得对应的类型
	*
	*/
	template <typename TL, unsigned int index> struct TypeAt;

	template <typename H, typename T>
	struct TypeAt<Typelist<H, T>, 0>
	{
		typedef H Result;
	};

	template <typename H, typename T, unsigned int i>
	struct TypeAt<Typelist<H, T>, i>
	{
		typedef typename TypeAt<T, i - 1>::Result Result;
	};


	/**
	* \brief 计算某类型，在Typelist中的索引值
	*
	*/
	template <typename TL, typename T> struct IndexOf;
	template <typename T>
	struct IndexOf<NullType, T>
	{
		enum { value = -1 };
	};

	template <typename T, typename H>
	struct IndexOf<Typelist<H, T>, T>
	{
		enum { value = 0 };
	};

	template <typename H, typename T, typename X>
	struct IndexOf<Typelist<H, T>, X>
	{
	private:
		enum { temp = IndexOf<T, X>::value };
	public:
		enum { value = temp == -1 ? -1 : 1 + temp };
	};
};


#define TYPELIST_0() NullType
#define TYPELIST_1(T1) TL::Typelist<T1,TYPELIST_0()>
#define TYPELIST_2(T1,T2) TL::Typelist<T1,TYPELIST_1(T2)>
#define TYPELIST_3(T1,T2,T3) TL::Typelist<T1,TYPELIST_2(T2,T3)>
#define TYPELIST_4(T1,T2,T3,T4) TL::Typelist<T1,TYPELIST_3(T2,T3,T4)>
#define TYPELIST_5(T1,T2,T3,T4,T5) TL::Typelist<T1,TYPELIST_4(T2,T3,T4,T5)>
#define TYPELIST_6(T1,T2,T3,T4,T5,T6) TL::Typelist<T1,TYPELIST_5(T2,T3,T4,T5,T6)>
#define TYPELIST_7(T1,T2,T3,T4,T5,T6,T7) TL::Typelist<T1,TYPELIST_6(T2,T3,T4,T5,T6,T7)>
#define TYPELIST_8(T1,T2,T3,T4,T5,T6,T7,T8) TL::Typelist<T1,TYPELIST_7(T2,T3,T4,T5,T6,T7,T8)>
#define TYPELIST_9(T1,T2,T3,T4,T5,T6,T7,T8,T9) TL::Typelist<T1,TYPELIST_8(T2,T3,T4,T5,T6,T7,T8,T9)>
#define TYPELIST_10(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10) TL::Typelist<T1,TYPELIST_9(T2,T3,T4,T5,T6,T7,T8,T9,T10)>
#define TYPELIST_11(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11) TL::Typelist<T1,TYPELIST_10(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11)>
#define TYPELIST_12(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12) TL::Typelist<T1,TYPELIST_11(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12)>
#define TYPELIST_13(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13) TL::Typelist<T1,TYPELIST_12(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13)>
#define TYPELIST_14(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14) TL::Typelist<T1,TYPELIST_13(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14)>
#define TYPELIST_15(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15) TL::Typelist<T1,TYPELIST_14(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15)>

#endif

