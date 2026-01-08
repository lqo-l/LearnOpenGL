编译命令:

```bash
cd test 

g++ gl_uniform_size.cpp  -I../external -I../include -L../lib -lglfw3 -lglad -lopengl32 -lgdi32 -luser32 -o tmp.exe
```

终端乱码解决方案：
```bash
chcp 65001 tmp.exe  #65001是UTF-8，接下来的字符输出使用UTF-8
```