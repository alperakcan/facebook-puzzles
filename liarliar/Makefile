
target=liarliar

all: ${target}

${target}: \
	fileio.c \
	stringtable.c \
	${target}.c
	gcc $(CFLAGS) -Wall -s -O99 -o ${target} $^

clean:
	$(RM) ${target}
	$(RM) -r ${target}.dSYM
	${RM} *.in.out

release: clean
	rm -rf tmp
	mkdir tmp
	cp Makefile tmp/Makefile
	cp liarliar.c tmp/liarliar.c
	cp ../common/common.h tmp/
	cp ../common/fileio.[ch] tmp/
	cp ../common/stringtable.[ch] tmp/
	tar -C tmp -cjvf ../${target}.tar.bz2 .
	rm -rf tmp

test: ${target}
	for i in `find ../input/${target}/ -name "*.in" | sort`; do \
	  echo "testing.. $$i"; \
	  dn=`dirname $$i`; \
	  bn=`basename $$i`; \
	  in=$$bn; \
	  on=`echo $$in | cut -d "." -f 1`.out; \
	  time ./${target} $$i > $$in.out && \
	  diff $$in.out $$dn/$$on > /dev/null; \
	  if [ "$$?" -eq "0" ]; then \
	    echo "result: `cat $$in.out`"; \
	    echo "OK"; \
	    rm $$in.out; \
	  else \
	    echo "result: `cat $$in.out`"; \
	    echo "should: `cat $$dn/$$on`"; \
	    exit; \
	  fi; \
	done
