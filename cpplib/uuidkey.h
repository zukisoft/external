//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef __UUIDKEY_H_
#define __UUIDKEY_H_
#pragma once

#include <stdint.h>
#include <rpcdce.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// uuidkey
//
// Provides specializations required to use uuid_t as a standard library
// collection/container key

namespace std {

	// std::equal_to<uuid_t>
	//
	template<> struct equal_to<uuid_t>
	{
		bool operator()(const uuid_t& lhs, const uuid_t& rhs) const { return (memcmp(&lhs, &rhs, sizeof(uuid_t)) == 0); }
	};

	// std::hash<uuid_t>
	//
	template<> struct hash<uuid_t>
	{
		size_t operator()(const uuid_t& key) const
		{
			// http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-source

	#ifndef _M_X64
			// 32-bit FNV-1a hash
			const size_t fnv_offset_basis{ 2166136261U };
			const size_t fnv_prime{ 16777619U };
	#else
			// 64-bit FNV-1a hash
			const size_t fnv_offset_basis{ 14695981039346656037ULL };
			const size_t fnv_prime{ 1099511628211ULL };
	#endif

			// Calcuate the FNV-1a hash for the uuid by processing each byte
			size_t hash = fnv_offset_basis;
			for(int index = 0; index < sizeof(uuid_t); index++) {

				hash ^= reinterpret_cast<const uint8_t*>(&key)[index];
				hash *= fnv_prime;
			}

			return hash;
		}
	};

	// std::less<uuid_t>
	//
	template<> struct less<uuid_t>
	{
		bool operator()(const uuid_t& lhs, const uuid_t& rhs) const { return (memcmp(&lhs, &rhs, sizeof(uuid_t)) < 0); }
	};

}	// namespace std

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __UUIDKEY_H_
