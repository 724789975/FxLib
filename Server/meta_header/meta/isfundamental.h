/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tencent is pleased to support the open source community by making behaviac available.
//
// Copyright (C) 2015-2017 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at http://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __META_ISFUNDAMENTAL_H__
#define __META_ISFUNDAMENTAL_H__

    namespace Meta {
        template< typename Type >
        struct IsFundamental {
            enum {
                Result = 0
            };
        };

#define IS_FUNDAMENTAL( Type )     \
    template<>                          \
    struct IsFundamental< Type >        \
    {                                   \
        enum                            \
        {                               \
            Result = 1                  \
        };                              \
    };

		IS_FUNDAMENTAL(uint8_t)
		IS_FUNDAMENTAL(uint16_t)
		IS_FUNDAMENTAL(uint32_t)

		IS_FUNDAMENTAL(int8_t)
		IS_FUNDAMENTAL(int16_t)

		IS_FUNDAMENTAL(signed long)
		IS_FUNDAMENTAL(unsigned long)

		IS_FUNDAMENTAL(int)

		IS_FUNDAMENTAL(char)
		IS_FUNDAMENTAL(bool)

#if !CCDEFINE_64BITS
		IS_FUNDAMENTAL(int64_t)
		IS_FUNDAMENTAL(uint64_t)
#else
		IS_FUNDAMENTAL(long long)
		IS_FUNDAMENTAL(unsigned long long)
#endif//CDEFINE_64BITS

		IS_FUNDAMENTAL(float)
		IS_FUNDAMENTAL(double)

		IS_FUNDAMENTAL(void)
    }

#endif	//__META_ISFUNDAMENTAL_H__
