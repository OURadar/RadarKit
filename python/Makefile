all: radarkit.so

radarkit.so: setup.py radarkit.c
	python3 setup.py build_ext --inplace

clean:
	rm -f radarkit.so
