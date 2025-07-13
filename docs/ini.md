# INI

**inih（INI Not Invented Here）**是一个简单的.INI文件用C编写的解析器。它只需要几页代码，而且设计得又小又简单，因此适用于嵌入式系统。

它也或多或少与Python的[ConfigParser](http://docs.python.org/library/configparser.html)风格兼容。INI文件，包括RFC 822风格的多行语法和“name:value”条目。

要使用它，只需给`ini_parse（）`一个ini文件，它就会为解析的每个`name=value`对调用一个回调，为您提供节、名称和值的字符串。

这样做（“SAX风格”）是因为它在低内存嵌入式系统上运行良好，但也因为它有助于KISS实现。

您还可以调用“ini_parse_file（）”直接从“file*”对象解析，调用“ini-parse_string（）”从字符串解析数据，或调用“inioparse_stream（）”使用自定义I/O的自定义fgets样式读取器函数解析。


## 编译时选项

您可以使用预处理器定义来控制inih的各个方面：
### 语法选项 ###

* **多行条目：**默认情况下，inih支持Python ConfigParser风格的多行条目。要禁用，请添加“-DINI_ALLOW_MULTILINE=0”。
* **UTF-8 BOM：**默认情况下，inih允许在INI文件的开头使用UTF-8 BOM序列（0xEF 0xBB 0xBF）。要禁用，请添加“-DINI_ALLOW_BOM=0”。
* **内联注释：**默认情况下，inih允许使用`；`性格。要禁用，请添加“-DINI_ALLOW_INLINE_COMMENTS=0”。您还可以使用`INI_inline_comment_PREFIXES`指定哪个字符开始内联注释。
* **行首注释：**默认情况下，inih允许两者都使用`；`使用“#”在行首开始注释。您可以通过更改“INI_START_COMMENT_PREFIXES”来覆盖此设置。
* **不允许有值：**默认情况下，inih将没有值的名称（行上没有“=”或“：”）视为错误。要允许没有值的名称，请添加“-DINI_ALOW_no_VALUE=1”，inih将调用值设置为NULL的处理程序函数。

### 解析选项

* **第一次出错时停止：**默认情况下，inih会在出错后继续解析文件的其余部分。要停止对第一个错误的解析，请添加“-DINI_stop_on_first_error=1”。
* **报告行号：**默认情况下，`ini_handler`回调函数不会接收行号作为参数。如果需要，请添加“-DINI_HANDLER_LINENO=1”。
* **在新节上调用处理程序：**默认情况下，inih只调用每个`name=value`对上的处理程序。要检测新节（例如，INI文件有多个同名节），请添加“-DINI_CALL_HANDLER_ON_new_SECTION=1”。每次遇到新节时，都会调用处理程序函数，将“section”设置为新节名称，但将“name”和“value”设置为NULL。

### 内存选项

* **堆栈与堆：**默认情况下，inih在堆栈上创建固定大小的行缓冲区。要使用`malloc`在堆上进行分配，请指定`-DINI_USE_STACK=0 `。
* **最大行长：**默认最大行长（堆栈或堆）为200字节。要覆盖此内容，请添加类似“-DINI_MAX_LINE=1000”的内容。请注意，`INI_MAX_LINE`必须比最长线多3个（由于`r`、`n`和NUL）。
* **初始malloc大小：**`INI_INTIAL_ALLOC`指定使用堆时的初始malloc尺寸。默认为200字节。
* **允许realloc:**默认情况下，当使用堆（`-DINI_USE_STACK=0 `）时，inih会分配一个固定大小的缓冲区，大小为`INI_INTIAL_ALLOC`字节。为了使其增长到“INI_MAX_LINE”字节，如果需要，可以加倍，设置“-DINI_allow_REALLOC=1”。
* **自定义分配器：**默认情况下，使用堆时，使用标准库的`malloc`、`free`和`realloc`函数；要使用自定义分配器，请指定“-DINI_custom_allocator=1”（和“-DINI_use_ACK=0”）。您必须定义并链接名为“ini_malloc”、“ini_free”和（如果设置了“ini_ALLOW_RELOC”）“ini_REALLOC”的函数，这些函数必须与“stdlib.h”内存分配函数具有相同的签名。


## 平台特定注意事项

*Windows/Win32原生使用UTF-16文件名，因此要处理Unicode路径，您需要调用`_wfopen（）`打开文件，然后调用`ini_parse_file（）`解析它；inih不包括wchar_t或Unicode处理。