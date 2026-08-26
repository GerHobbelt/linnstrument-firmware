FROM debian:bookworm-slim

ARG TARGETARCH
ARG ARDUINO_CLI_VERSION=1.5.1

RUN apt-get update \
    && apt-get install --yes --no-install-recommends ca-certificates curl tar \
    && rm --recursive --force /var/lib/apt/lists/*

RUN case "$TARGETARCH" in \
        amd64) cli_arch=64bit ;; \
        arm64) cli_arch=ARM64 ;; \
        *) echo "Unsupported architecture: $TARGETARCH" >&2; exit 1 ;; \
    esac \
    && curl --fail --location --silent --show-error \
        "https://github.com/arduino/arduino-cli/releases/download/v${ARDUINO_CLI_VERSION}/arduino-cli_${ARDUINO_CLI_VERSION}_Linux_${cli_arch}.tar.gz" \
        | tar --extract --gzip --file - --directory /usr/local/bin arduino-cli

RUN mkdir --parents /opt/arduino/data /opt/arduino/downloads /opt/arduino/user \
    && printf '%s\n' \
        'directories:' \
        '  data: /opt/arduino/data' \
        '  downloads: /opt/arduino/downloads' \
        '  user: /opt/arduino/user' \
        > /etc/arduino-cli.yaml \
    && arduino-cli --config-file /etc/arduino-cli.yaml core update-index \
    && arduino-cli --config-file /etc/arduino-cli.yaml core install arduino:sam@1.6.11

WORKDIR /workspace
