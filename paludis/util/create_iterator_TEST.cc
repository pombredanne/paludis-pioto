/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <algorithm>
#include <vector>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace
{
    struct Foo
    {
        int value;

        Foo(const int i) :
            value(i)
        {
        }
    };

    std::ostream & operator<< (std::ostream & s, const Foo & y)
    {
        s << "<" << y.value << ">";
        return s;
    }
}

namespace test_cases
{
    struct CreateIteratorTest : TestCase
    {
        CreateIteratorTest() : TestCase("create iterator") { }

        void run()
        {
            std::vector<int> x;
            x.push_back(1);
            x.push_back(2);
            x.push_back(3);

            std::vector<Foo> y;
            std::copy(x.begin(), x.end(), create_inserter<Foo>(std::back_inserter(y)));

            TEST_CHECK_EQUAL(join(y.begin(), y.end(), " "), "<1> <2> <3>");
        }
    } create_iterator_test;
}

