# vector-monitoring-system
a simple concurrent 3D vector monitoring system(c++17)

## Architecture and main flow

```text
2 Updaters ──write──┐
                    v
               SharedVector <──read── 5 ThresholdLoggers ──print──> Console
```

`SharedVector` stores the single 3D vector and protects it with a shared mutex:
loggers take concurrent read snapshots while updaters make exclusive updates.
Each updater generates components from 0 to 100; each logger checks the
magnitude against its own randomly selected inclusive range. Console output is
serialized so lines from different loggers do not interleave.

At startup, `main` creates the workers with random frequencies and ranges,
starts them, waits for the selected runtime, then stops and joins them. The
default runtime is 60 seconds; `--duration-ms <milliseconds>` selects a shorter
runtime.

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
