#pragma once

#include <string>
#include <memory>
#include <cinttypes>

// (more) chars
typedef unsigned char	uchar;

// integer
typedef int8_t		i8;
typedef uint8_t		u8;

typedef int16_t		i16;
typedef uint16_t	u16;

typedef int32_t		i32;
typedef uint32_t	u32;

typedef int64_t		i64;
typedef uint64_t	u64;

// decimals
typedef float		f32;
typedef double		f64;

// pointer size dependent
typedef std::ptrdiff_t  isz;
typedef std::size_t     usz;


// pointer wrapper
template<class T>
using ptrS = std::shared_ptr<T>;
template<class T>
using ptrU = std::unique_ptr<T>;
template<class T>
using ptrW = std::weak_ptr<T>;

// always needed
using std::string;
