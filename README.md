# random_notes
Random Notes from Sickbed

Some summaries from the process of learning C++
Currently, the main focus is on updating
- Open source library
- C++ syntax


# build

This project is maintained using xmake.
The basic usage of xmake can be found in the xmake.md documentation.

```shell
cd random_notes

xmake f -c  # clean

xmake   # Build all by default

xmake -b ini    # Only build inih

xmake -b [target]   # Build your declared goals

# This is a command I commonly use to build the specified target and display detailed compilation information (v), 
# and it is the complete build of the specified target (a)
xmake -avb  [target]    

# Switching the xmake rule allows debugging using GDB
xmake f -m debug

# Run the specified target program
xmake run [target]

# Debug the specified target program
xmake run -d [target]
```

# docs directory

- ini.md: Ini.md: Translated the REAMDE.md of the open source library inih

# Open source library
I will learn by adding comments to the open-source libraries I am currently studying, incorporating test programs, and using xmake for project management


# inih

inih is a simple .INI file parser written in C. It is designed to be small, compact, portable, and efficient. It is used in many applications, including some of the most popular Linux applications.

website: https://github.com/benhoyt/inih.git

## More test cases

see ./example/inih

# cmockery

CMockery is a lightweight unit testing and mock framework designed specifically for the C language, developed and maintained by Google. Its design goal is to provide concise and easy-to-use testing tools, particularly suitable for C language projects.

website: https://github.com/google/cmockery.git

## fix and enhance

- Warning caused by type conversion processing: See ./docs/cmockery/README.md
- Add new assertions: assert_floats_equal、assert_null、assert_non_null
- Custom assertion failure handling function： custom_assert_failed

## More test cases

see ./example/cmockery

# miniz 

Miniz is a lightweight, high-performance compression and decompression library that focuses on providing zlib compatible features while also being small in size and easy to integrate. It is widely used in scenarios that require efficient processing of DEFLATE compression algorithms (commonly used in formats such as ZIP, PNG, gzip, etc.), especially suitable for embedded systems, game development, or lightweight applications that have strict requirements for library size and memory usage.


## Coming soon



# readerwriterqueue

## Coming soon

# muduo

## Coming soon

# workflow

## Coming soon