# vector-monitoring-system
a simple concurrent 3D vector monitoring system(c++17)

## Running the tests in a container

Requires podman(or docker).

```sh
podman build -f Dockerfile_noble -t vm-tests .
podman run --rm vm-tests
# optional
podman run --rm vm-tests ./run_tests.sh INTEGRATION
podman run --rm vm-tests ./run_tests.sh SANITIZER
```

The image is based on Ubuntu 24.04 (noble) and installs the build
dependencies. Running the container configures, builds and runs the tests via `run_tests.sh`.


The application runs for up to 60 seconds by default. For a shorter run,
`vector_monitoring --duration-ms <milliseconds>` accepts a duration from 1 to
60000 milliseconds.
