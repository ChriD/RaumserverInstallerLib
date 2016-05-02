#!/bin/bash
make arch=X64 clean
make arch=X64
/bin/cp -rf libs/_tmpBuild/raumserverInstaller.so libs/linux_x64/raumserverInstaller.so
make arch=X64 clean
