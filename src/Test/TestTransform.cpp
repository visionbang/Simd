/*
* Tests for Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2017 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "Test/TestUtils.h"
#include "Test/TestPerformance.h"
#include "Test/TestData.h"

namespace Test
{
    namespace
    {
        struct FuncTI
        {
            typedef void(*FuncPtr)(const uint8_t * src, size_t srcStride, size_t width, size_t height, size_t pixelSize, SimdTransformType transform, uint8_t * dst, size_t dstStride);

            FuncPtr func;
            String desc;

            FuncTI(const FuncPtr & f, const String & d) : func(f), desc(d) {}

            void Update(::SimdTransformType transform, size_t size)
            {
                std::stringstream ss;
                ss << desc << "[";
                switch (transform)
                {
                case ::SimdTransformRotate0:            ss << "N0"; break;
                case ::SimdTransformRotate90:           ss << "N1"; break;
                case ::SimdTransformRotate180:          ss << "N2"; break;
                case ::SimdTransformRotate270:          ss << "N3"; break;
                case ::SimdTransformTransposeRotate0:   ss << "T0"; break;
                case ::SimdTransformTransposeRotate90:  ss << "T1"; break;
                case ::SimdTransformTransposeRotate180: ss << "T2"; break;
                case ::SimdTransformTransposeRotate270: ss << "T3"; break;
                default:
                    assert(0);
                }
                ss << "-" << size << "]";
                desc = ss.str();
            }


            void Call(const View & src, SimdTransformType transform, View & dst) const
            {
                TEST_PERFORMANCE_TEST(desc);
                func(src.data, src.stride, src.width, src.height, src.PixelSize(), transform, dst.data, dst.stride);
            }
        };
    }

#define FUNC_TI(function) \
    FuncTI(function, std::string(#function))

    bool TransformImageAutoTest(::SimdTransformType transform, View::Format format, int width, int height, FuncTI f1, FuncTI f2)
    {
        bool result = true;

        f1.Update(transform, View::PixelSize(format));
        f2.Update(transform, View::PixelSize(format));

        TEST_LOG_SS(Info, "Test " << f1.desc << " & " << f2.desc << " [" << width << ", " << height << "].");

        View s(width, height, format, NULL, TEST_ALIGN(width));
        FillRandom(s);

        Size ds = Simd::TransformSize(s.Size(), transform);
        View d1(ds.x, ds.y, format, NULL, TEST_ALIGN(width));
        View d2(ds.x, ds.y, format, NULL, TEST_ALIGN(width));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(s, transform, d1));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(s, transform, d2));

        result = result && Compare(d1, d2, 0, true, 32);

        return result;
    }

    bool TransformImageAutoTest(const FuncTI & f1, const FuncTI & f2)
    {
        bool result = true;

        for (::SimdTransformType transform = ::SimdTransformRotate0; transform <= ::SimdTransformTransposeRotate270; transform = ::SimdTransformType(transform + 1))
        {
            for (View::Format format = View::Gray8; format <= View::Bgra32; format = View::Format(format + 1))
            {
                result = result && TransformImageAutoTest(transform, format, W, H, f1, f2);
                result = result && TransformImageAutoTest(transform, format, W + O, H - O, f1, f2);
            }
        }

        return result;
    }

    bool TransformImageAutoTest()
    {
        bool result = true;

        result = result && TransformImageAutoTest(FUNC_TI(Simd::Base::TransformImage), FUNC_TI(SimdTransformImage));

        return result;
    }
}
