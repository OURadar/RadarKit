all: radarkit.so

radarkit.so: setup.py radarkitmodule.c
	python3 setup.py build_ext --inplace

clean:
	rm -f radarkit.so
