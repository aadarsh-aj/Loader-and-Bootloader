all:
	$(MAKE) -C loader
	$(MAKE) -C launcher
	$(MAKE) -C test

clean:
	$(MAKE) -C loader clean
	$(MAKE) -C launcher clean
	$(MAKE) -C test clean
	rm -rf bin