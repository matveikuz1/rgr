CXX = g++
CXXFLAGS = -fPIC -I.
LDFLAGS = -shared
LIBS = -L. -lhill -lvigenere -lrichelieu

all: main

# Создание динамических библиотек для шифров
libhill.so: hill.o
	$(CXX) $(LDFLAGS) -o $@ $^

libvigenere.so: vigenere.o
	$(CXX) $(LDFLAGS) -o $@ $^

librichelieu.so: richelieu.o
	$(CXX) $(LDFLAGS) -o $@ $^

# Компиляция объектных файлов для библиотек (с -fPIC)
hill.o: hill.cpp hill.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

vigenere.o: vigenere.cpp vigenere.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

richelieu.o: richelieu.cpp richelieu.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Компиляция file.cpp в объектный файл (БЕЗ -fPIC, так как не будет .so)
file.o: file.cpp file.h
	$(CXX) -I. -c $< -o $@

# Компиляция main.cpp + линковка с file.o и динамическими библиотеками
main: main.cpp file.o libhill.so libvigenere.so librichelieu.so
	$(CXX) main.cpp file.o -o rgr_main $(LIBS) -I.

clean:
	rm -f *.o *.so main

.PHONY: all clean
