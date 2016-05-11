#!/bin/bash
make arch=X64 dbg=1 clean
make arch=X64 dbg=1
/bin/cp -rf libs/_tmpBuild/raumserverInstaller.so libs/linux_x64/raumserverInstaller.so
make arch=X64 dbg=1 clean
