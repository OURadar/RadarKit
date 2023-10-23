ifeq ($(HOMEBREW_PREFIX), )
	ifeq ($(MACHINE), arm64)
		HOMEBREW_PREFIX = /opt/homebrew
	else
		HOMEBREW_PREFIX = /usr/local
	endif
endif

all: install

.PHONY: all

.FORCE:

install:
	@if [ ! -f ${HOMEBREW_PREFIX}/lib/libradarkit.a ]; then \
		echo "libradarkit.a not found. Please build and install RadarKit first."; \
		exit 1; \
	fi
	python -m pip install .

uninstall:
	python -m pip uninstall -y radarkit

clean:
	rm -rf radarkit/__pycache__
	rm -rf build dist *.egg-info .ipynb_checkpoints
