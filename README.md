<h1>Build Process:</h1>
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/Users/petro/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows<br>
cmake --build .<br>
.\Debug\Simulation.exe