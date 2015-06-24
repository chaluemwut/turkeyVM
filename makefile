all:
	dos2unix *.c
	dos2unix *.h
	ctags -R
	clang -g vm.c class.c alloc.c exception.c linkedlist.c resolve.c testvm.c Command-line.c execute.c interp.c control.c native.c string.c reflect.c testUTF8.c dll.c cast.c -o vm -ldl

clean:
	rm -rf vmp
