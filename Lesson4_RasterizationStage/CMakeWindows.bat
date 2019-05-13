@echo off
if not exist Build mkdir Build
pushd Build
cmake -G "Visual Studio 15 2017" ../
popd
