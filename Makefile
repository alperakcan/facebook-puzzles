
all:
	make -C hoppity
	make -C meepmeep
	make -C liarliar
	make -C breathalyzer
	make -C gattaca
	make -C dancebattle

clean:
	make -C hoppity clean
	make -C meepmeep clean
	make -C liarliar clean
	make -C breathalyzer clean
	make -C gattaca clean
	make -C dancebattle clean
	$(RM) *.tar.bz2

test:
	make -C hoppity test
	make -C meepmeep test
	make -C liarliar test
	make -C breathalyzer test
	make -C gattaca test
	make -C dancebattle test

release:
	make -C hoppity release
	make -C meepmeep release
	make -C liarliar release
	make -C breathalyzer release
	make -C gattaca release
	make -C dancebattle release
