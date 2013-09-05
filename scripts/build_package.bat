:: Setup Visual Studio environment
call setup_vs.bat %1

:: Goto sources directory
cd ..

:: Cleanup
rd /S /Q build_package

:: Build
mkdir build_package
cd build_package
cmake -G %generator% -DOGS_USE_QT=ON -DOGS_PACKAGING=ON -DOGS_PACKAGING_ZIP=ON ..
cmake ..
devenv OGS-5-GUI.sln /Build Release /Project PACKAGE

cd ..
cd scripts
