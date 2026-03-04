```
EMS Notify Application built using Qt.
```

https://github.com/harsha-deep/EmsNotify/blob/bb826be5b754b6fab2749f6ec66130d52b06c947/EmsNotify/CMakeLists.txt#L14
```
For console debugging, remove WIN32 and compile.
```


Settings Location:
- Windows: ```%APPDATA%\CSG\EmsNotify.ini```

- Linux: ```~/.config/CSG/EmsNotify.ini```

Steps to Build on Linux:
Copy contents of CMakeLists_DebRpm.txt to CMakeLists.txt

```
cmake -B build -S EmsNotify -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_PREFIX_PATH=/usr/libx86_64-linux-gnu/cmake/Qt6
```

```
cmake --build build --config Release
```

```
cd build
```

```
cpack
```