default:
	$(MAKE) -C ./part2
	$(MAKE) -C ./part3

.PHONY: default clean

clean:
	$(MAKE) -C ./part2 clean
	$(MAKE) -C ./part3 clean