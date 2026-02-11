FROM python:3.12.10

ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1
ENV LD_LIBRARY_PATH=/usr/local/lib

# Install Python dependencies early for layer caching
COPY python/requirements.txt /tmp/requirements.txt
COPY .devcontainer/sh-root-bashrc /root/.bashrc

RUN pip install -U pip

# System packages (add more only if needed)
RUN apt update && \
    apt install -y bc git libfftw3-dev libnetcdf-dev libssl-dev libarchive-dev && \
    git clone https://git.arrc.ou.edu/radar/radarkit.git /tmp/radarkit && \
    cd /tmp/radarkit && \
    git checkout dev0

RUN if [ "$(uname -m)" == "aarch64" ] && [ ! -f /workspace/data/radarkit-fft-wisdom ] ; then \
        echo "Downloading DFT wisdom from ARRC server..." ; \
        curl -o /workspace/data/radarkit-fft-wisdom https://arrc.ou.edu/share/radarkit-fft-wisdom-aarch64 ; \
    fi

WORKDIR /workspace

# RUN ls > /tmp/contents.txt
