#!/bin/bash

export PATH=~/Qt/5.11.3/gcc_64/bin/:$PATH

echo 'Updating tr files for IFC import plugin and all dependent libraries'


lupdate ../../../externals/QtExt/projects/Qt/QtExt.pro
lupdate ../../projects/Qt/ImportIFCPlugin.pro

