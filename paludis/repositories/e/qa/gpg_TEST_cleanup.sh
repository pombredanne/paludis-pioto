#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

if [ -d gpg_TEST_dir ] ; then
    rm -fr gpg_TEST_dir
else
    true
fi