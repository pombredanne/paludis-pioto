/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#include <paludis/repositories/unavailable/unavailable_repository.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/package_id.hh>
#include <paludis/package_database.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <tr1/memory>

using namespace paludis;
using namespace paludis::unavailable_repository;
using namespace test;

namespace test_cases
{
    struct UnavailableRepositoryCreationTest : TestCase
    {
        UnavailableRepositoryCreationTest() : TestCase("creation") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<UnavailableRepository> repo(new UnavailableRepository(
                        make_named_values<UnavailableRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::location>(FSEntry::cwd() / "unavailable_repository_TEST_dir" / "repo1"),
                            value_for<n::name>(RepositoryName("unavailable")),
                            value_for<n::sync>(""),
                            value_for<n::sync_options>("")
                        )));
            env.package_database()->add_repository(1, repo);
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "unavailable");
        }
    } test_creation;

    struct UnavailableRepositoryContentsTest : TestCase
    {
        UnavailableRepositoryContentsTest() : TestCase("contents") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<UnavailableRepository> repo(new UnavailableRepository(
                        make_named_values<UnavailableRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::location>(FSEntry::cwd() / "unavailable_repository_TEST_dir" / "repo2"),
                            value_for<n::name>(RepositoryName("unavailable")),
                            value_for<n::sync>(""),
                            value_for<n::sync_options>("")
                        )));
            env.package_database()->add_repository(1, repo);
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "unavailable");

            std::tr1::shared_ptr<const PackageIDSequence> contents(
                    env[selection::AllVersionsSorted(generator::All())]);
            TEST_CHECK(contents);

            TEST_CHECK_EQUAL(
                    join(indirect_iterator(contents->begin()), indirect_iterator(contents->end()), " "),
                    "cat-one/pkg-one-1:0::unavailable (in ::bar) "
                    "cat-one/pkg-one-1:0::unavailable (in ::foo) "
                    "cat-one/pkg-one-2:0::unavailable (in ::foo) "
                    "cat-one/pkg-one-3:0::unavailable (in ::foo) "
                    "cat-one/pkg-one-3:3::unavailable (in ::bar) "
                    "cat-one/pkg-six-3:0::unavailable (in ::bar) "
                    "cat-one/pkg-two-1:1::unavailable (in ::foo) "
                    "cat-one/pkg-two-2:2::unavailable (in ::foo) "
                    "cat-two/pkg-six-1:0::unavailable (in ::bar)"
                    );

            const std::tr1::shared_ptr<FakeRepository> hide_bar(new FakeRepository(make_named_values<FakeRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::name>(RepositoryName("bar"))
                            )));
            env.package_database()->add_repository(2, hide_bar);
            repo->invalidate();

            std::tr1::shared_ptr<const PackageIDSequence> contents_without_bar(
                    env[selection::AllVersionsSorted(generator::All())]);
            TEST_CHECK(contents);

            TEST_CHECK_EQUAL(
                    join(indirect_iterator(contents_without_bar->begin()),
                        indirect_iterator(contents_without_bar->end()), " "),
                    "cat-one/pkg-one-1:0::unavailable (in ::foo) "
                    "cat-one/pkg-one-2:0::unavailable (in ::foo) "
                    "cat-one/pkg-one-3:0::unavailable (in ::foo) "
                    "cat-one/pkg-two-1:1::unavailable (in ::foo) "
                    "cat-one/pkg-two-2:2::unavailable (in ::foo)"
                    );
        }
    } test_contents;
}

