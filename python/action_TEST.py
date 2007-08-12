#!/usr/bin/env python
# vim: set fileencoding=utf-8 sw=4 sts=4 et :

#
# Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
#

from paludis import *
import unittest

Log.instance.log_level = LogLevel.WARNING

class TestCase_01_InstallActionOptions(unittest.TestCase):
    def setUp(self):
        global repo1, repo2
        env = TestEnvironment()
        repo1 = FakeRepository(env, "1")
        repo2 = FakeRepository(env, "2")

    def test_01_create(self):
        InstallActionOptions(True, InstallActionDebugOption.values[0], InstallActionChecksOption.values[0], repo1)

    def test_02_data_members(self):
        iao = InstallActionOptions(True, InstallActionDebugOption.values[0],
                InstallActionChecksOption.values[0], repo1)

        self.assertEquals(iao.no_config_protect, True)
        self.assertEquals(iao.debug_build, InstallActionDebugOption.values[0])
        self.assertEquals(iao.checks, InstallActionChecksOption.values[0])
        self.assertEquals(str(iao.destination.name), "1")

        iao.no_config_protect = False
        iao.debug_build = InstallActionDebugOption.values[1]
        iao.checks = InstallActionChecksOption.values[1]
        iao.destination = repo2

        self.assertEquals(iao.no_config_protect, False)
        self.assertEquals(iao.debug_build, InstallActionDebugOption.values[1])
        self.assertEquals(iao.checks, InstallActionChecksOption.values[1])
        self.assertEquals(str(iao.destination.name), "2")

class TestCase_02_FetchActionOptions(unittest.TestCase):
    def test_01_create(self):
        FetchActionOptions(True, True)

    def test_02_data_members(self):
        fao = FetchActionOptions(True, True)

        self.assertEquals(fao.fetch_unneeded, True)
        self.assertEquals(fao.safe_resume, True)

        fao.fetch_unneeded = False
        fao.safe_resume = False

        self.assertEquals(fao.fetch_unneeded, False)
        self.assertEquals(fao.safe_resume, False)

class TestCase_03_UninstallActionOptions(unittest.TestCase):
    def test_01_create(self):
        UninstallActionOptions(True)

    def test_02_data_members(self):
        uao = UninstallActionOptions(True)

        self.assertEquals(uao.no_config_protect, True)
        uao.no_config_protect = False
        self.assertEquals(uao.no_config_protect, False)


class TestCase_04_InstallAction(unittest.TestCase):
    def test_01_create(self):
        env = TestEnvironment()
        repo1 = FakeRepository(env, "1")
        iao = InstallActionOptions(True, InstallActionDebugOption.values[0],
                InstallActionChecksOption.values[0], repo1)
        InstallAction(iao)

class TestCase_05_FetchAction(unittest.TestCase):
    def test_01_create(self):
        FetchAction(FetchActionOptions(True, True))

class TestCase_06_UninstallAction(unittest.TestCase):
    def test_01_create(self):
        UninstallAction(UninstallActionOptions(True))

class TestCase_07_InstalledAction(unittest.TestCase):
    def test_01_create(self):
        InstalledAction()

class TestCase_08_PretendAction(unittest.TestCase):
    def test_01_create(self):
        PretendAction()

class TestCase_09_ConfigAction(unittest.TestCase):
    def test_01_create(self):
        ConfigAction()

class TestCase_10_SupportsActionTests(unittest.TestCase):
    def test_01_create(self):
        SupportsInstallActionTest()
        SupportsFetchActionTest()
        SupportsUninstallActionTest()
        SupportsInstalledActionTest()
        SupportsPretendActionTest()
        SupportsConfigActionTest()


if __name__ == "__main__":
    unittest.main()