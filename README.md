# vector-monitoring-system
a simple concurrent 3D vector monitoring system(c++17)

## Running the tests in a container

Requires podman(or docker).

```sh
podman build -f Dockerfile_noble -t vm-tests .
podman run --rm vm-tests
```

The image is based on Ubuntu 24.04 (noble) and installs the build
dependencies. Running the container configures, builds and runs the tests
via `run_tests.sh`.
