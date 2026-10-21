#!/usr/bin/env python3
import math
import re
import subprocess
import sys
import tempfile
from pathlib import Path


OUTPUT_LINE = re.compile(
    r"^interval\{L1=(\d+), L2=(\d+)\} : "
    r"\{x=([-+0-9.eE]+), y=([-+0-9.eE]+), z=([-+0-9.eE]+)\}$"
)


def main() -> int:
    executable = Path(sys.argv[1])
    with tempfile.TemporaryFile(mode="w+t") as output:
        result = subprocess.run(
            [str(executable), "--duration-ms", "5000"],
            stdout=output,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
            timeout=15,
        )
        if result.returncode != 0:
            print(
                f"program exited with {result.returncode}: {result.stderr}",
                file=sys.stderr,
            )
            return 1

        output.seek(0)
        output_count = 0
        for line_number, line in enumerate(output, start=1):
            match = OUTPUT_LINE.fullmatch(line.rstrip("\n"))
            if match is None:
                print(
                    f"malformed output on line {line_number}: {line!r}",
                    file=sys.stderr,
                )
                return 1

            lower, upper = (int(match.group(index)) for index in (1, 2))
            vector = tuple(float(match.group(index)) for index in (3, 4, 5))
            if not (
                1 <= lower <= 200
                and 1 <= upper <= 200
                and upper - lower >= 20
            ):
                print(f"invalid interval on line {line_number}", file=sys.stderr)
                return 1
            if any(
                not math.isfinite(component) or not 0 <= component <= 100
                for component in vector
            ):
                print(f"invalid vector on line {line_number}", file=sys.stderr)
                return 1

            magnitude_squared = sum(component * component for component in vector)
            if not lower * lower <= magnitude_squared <= upper * upper:
                print(f"vector outside interval on line {line_number}", file=sys.stderr)
                return 1
            output_count += 1

        if output_count == 0:
            print("program produced no qualifying vector output", file=sys.stderr)
            return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
