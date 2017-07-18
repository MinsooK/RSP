﻿#ifndef BICOMC_DETAIL_COMPILER_CLANG_H__
#define BICOMC_DETAIL_COMPILER_CLANG_H__

#if !defined(__clang__)
#	error "compiler is not clang"
#endif // !def __clang__

#if !defined(BICOMC_EXPORT)
#	if defined(_MSC_VER) && defined(_WIN32)
#		define BICOMC_EXPORT __declspec(dllexport)
#	else
#		define BICOMC_EXPORT __attribute__((visibility("default")))
#	endif
#endif // !def BICOMC_EXPORT

#if !defined(BICOMC_IS_NULLPTR_SUPPORT_COMPILER)
#	define BICOMC_IS_NULLPTR_SUPPORT_COMPILER \
		(__has_feature(cxx_nullptr))
#endif // !def BICOMC_IS_NULLPTR_SUPPORT_COMPILER
#if !defined(BICOMC_IS_STATIC_ASSERT_SUPPORT_COMPILER)
#	define BICOMC_IS_STATIC_ASSERT_SUPPORT_COMPILER \
		(__has_feature(cxx_static_assert))
#endif // !def BICOMC_IS_STATIC_ASSERT_SUPPORT_COMPILER
#if !defined(BICOMC_IS_THREAD_SAFE_STATIC_INIT_SUPPORT_COMPILER)
#	if defined(_MSC_VER) && _MSC_VER >= 1900
#		define BICOMC_IS_THREAD_SAFE_STATIC_INIT_SUPPORT_COMPILER \
			(1)
#	elif defined(__GNUC__) && __GNUC__ >= 4
#		define BICOMC_IS_THREAD_SAFE_STATIC_INIT_SUPPORT_COMPILER \
			(1)
#	else
#		define BICOMC_IS_THREAD_SAFE_STATIC_INIT_SUPPORT_COMPILER \
			(__cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__))
#	endif
#endif // !def BICOMC_IS_THREAD_SAFE_STATIC_INIT_SUPPORT_COMPILER
#if !defined(BICOMC_IS_CONSTEXPR_SUPPORT_COMPILER)
#	define BICOMC_IS_CONSTEXPR_SUPPORT_COMPILER \
		(__has_feature(cxx_constexpr))
#endif // !def BICOMC_IS_CONSTEXPR_SUPPORT_COMPILER
#if !defined(BICOMC_IS_NOEXCEPT_SUPPORT_COMPILER)
#	define BICOMC_IS_NOEXCEPT_SUPPORT_COMPILER \
		(__has_feature(cxx_noexcept))
#endif // !def BICOMC_IS_NOEXCEPT_SUPPORT_COMPILER
#if !defined(BICOMC_IS_EXPLICITY_DEFAULT_DELETE_SUPPORT_COMPILER)
#	define BICOMC_IS_EXPLICITY_DEFAULT_DELETE_SUPPORT_COMPILER \
		(__has_feature(cxx_deleted_functions))
#endif // !def BICOMC_IS_EXPLICITY_DEFAULT_DELETE_SUPPORT_COMPILER
#if !defined(BICOMC_IS_MOVE_SEMANTIC_SUPPORT_COMPILER)
#	define BICOMC_IS_MOVE_SEMANTIC_SUPPORT_COMPILER \
		(__has_feature(cxx_rvalue_references))
#endif // !def BICOMC_IS_MOVE_SEMANTIC_SUPPORT_COMPILER
#if !defined(BICOMC_IS_VARIADIC_TEMPLATE_SUPPORT_COMPILER)
#	define BICOMC_IS_VARIADIC_TEMPLATE_SUPPORT_COMPILER \
		(__has_feature(cxx_variadic_templates))
#endif // !def BICOMC_IS_VARIADIC_TEMPLATE_SUPPORT_COMPILER
#if !defined(BICOMC_IS_CHAR_16_32_SUPPORT_COMPILER)
#	if defined(_MSC_VER) && _MSC_VER >= 1900 
#		define BICOMC_IS_CHAR_16_32_SUPPORT_COMPILER \
			(1)
#	elif
#		define BICOMC_IS_CHAR_16_32_SUPPORT_COMPILER \
			(__cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__))
#	endif
#endif // !def BICOMC_IS_CHAR_16_32_SUPPORT_COMPILER
#if !defined(BICOMC_IS_UNICODE_STRING_LITERAL_SUPPORT_COMPILER)
#	define BICOMC_IS_UNICODE_STRING_LITERAL_SUPPORT_COMPILER \
		(__has_feature(cxx_unicode_literals))
#endif // !def BICOMC_IS_UNICODE_STRING_LITERAL_SUPPORT_COMPILER
#if !defined(BICOMC_IS_STD_INT_SUPPORT_COMPILER)
#	define BICOMC_IS_STD_INT_SUPPORT_COMPILER \
		(__has_include(<cstdint>))
#endif // !def BICOMC_IS_STD_INT_SUPPORT_COMPILER
#if !defined(BICOMC_IS_TYPE_TRAITS_SUPPORT_COMPILER)
#	define BICOMC_IS_TYPE_TRAITS_SUPPORT_COMPILER	\
		(__has_include(<type_traits>))
#endif // !def BICOMC_IS_TYPE_TRAITS_SUPPORT_COMPILER
#if !defined(BICOMC_IS_MUTEX_SUPPORT_COMPILER)
#	define BICOMC_IS_MUTEX_SUPPORT_COMPILER \
		(__has_include(<mutex>))
#endif // !def BICOMC_IS_MUTEX_SUPPORT_COMPILER
#if !defined(BICOMC_IS_CODE_CVT_UTF8_SUPPORT_COMPILER)
#	define BICOMC_IS_CODE_CVT_UTF8_SUPPORT_COMPILER \
		(__has_include(<codecvt>))
#endif // !def BICOMC_IS_CODE_CVT_UTF8_SUPPORT_COMPILER
#if !defined(BICOMC_IS_EXTENED_EQUAL_SUPPORT_COMPILER)
#	define BICOMC_IS_EXTENED_EQUAL_SUPPORT_COMPILER \
		(__cplusplus >= 201103L)
#endif // !def BICOMC_IS_EXTENED_EQUAL_SUPPORT_COMPILER
#if !defined(BICOMC_IS_ARRAY_SUPPORT_COMPILER)
#	define BICOMC_IS_ARRAY_SUPPORT_COMPILER \
		(__has_include(<array>))
#endif // !def BICOMC_IS_ARRAY_SUPPORT_COMPILER

#endif // !def BICOMC_DETAIL_COMPILER_CLANG_H__
