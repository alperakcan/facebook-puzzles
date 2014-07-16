
target=hoppity

all: ${target}

${target}: ${target}.c
	$(CC) $(CFLAGS) -Wall -g -o ${target} $^

clean:
	$(RM) ${target}
	$(RM) -r ${target}.dSYM

release: clean
	tar -cjvf ../${target}.tar.bz2 .

test: ${target}
	for i in `find ../input/${target}/ -name "*.txt"`; do echo "testing.. $$i"; ./${target} $$i; done
