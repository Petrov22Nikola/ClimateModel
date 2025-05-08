<h1>Build process:</h1>
```
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/Users/petro/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows  
cmake --build .
.\Debug\Simulation.exe
```