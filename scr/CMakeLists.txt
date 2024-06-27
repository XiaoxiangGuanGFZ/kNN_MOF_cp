cmake_minimum_required(VERSION 3.0)

project(kNN_MOF_cp)  # Set your project name here

# Enable the C language
enable_language(C)


# Add your source files here
set(SOURCE_FILES
    main.c
    Func_dataIO.c
    Func_Initialize.c
    Func_kNN_MOF_cp.c
)

# Add the executable target
add_executable(kNN_MOF_cp ${SOURCE_FILES})

# Link against the math library
target_link_libraries(kNN_MOF_cp m)

## cmake -G "MinGW Makefiles" .
## mingw32-make
