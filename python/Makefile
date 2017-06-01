all: rkstruct.so

rkstruct.so: setup.py rkstruct.c
	python3 setup.py build_ext --inplace --include-dirs=/usr/local/include --library-dirs=/usr/local/lib --libraries=RadarKit,fftw3f,netcdf

clean:
	rm -f rkstruct*.so
