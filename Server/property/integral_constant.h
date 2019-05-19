#ifndef __IntegralConstant_H__
#define __IntegralConstant_H__

template <class T, T val>
struct integral_constant
{
	typedef integral_constant<T, val>  type;
	typedef T                          value_type;
	static const T value = val;
};

typedef integral_constant<bool, true>  true_type;
typedef integral_constant<bool, false> false_type;

template<typename, typename> struct is_same : public false_type { };
template<typename _Tp> struct is_same<_Tp, _Tp>: public true_type { };


#endif
