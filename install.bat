cmake -Bbuild -H. -G"Visual Studio 16 2019" -DLIMA_ENABLE_PYTHON=1 -DCAMERA_ENABLE_TESTS=1 -DCMAKE_INSTALL_PREFIX=%CONDA_PREFIX%\Library -DCMAKE_FIND_ROOT_PATH=%CONDA_PREFIX%\Library
IF %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

cmake --build build --config Release --target install
IF %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
