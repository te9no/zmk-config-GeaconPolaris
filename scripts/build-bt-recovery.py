#!/usr/bin/env python3
"""Build a selected Polaris target, with explicit pinned bridge preparation."""
import argparse
import pathlib
import subprocess
import yaml
p = argparse.ArgumentParser()
p.add_argument("target", help="artifact-name from build.yaml")
args = p.parse_args()
root = pathlib.Path(__file__).resolve().parents[1]
workspace = pathlib.Path(subprocess.check_output(["west", "topdir"], text=True).strip())
zmk = pathlib.Path(subprocess.check_output(["west", "list", "zmk", "-f", "{abspath}"], text=True).strip())
zephyr = pathlib.Path(subprocess.check_output(["west", "list", "zephyr", "-f", "{abspath}"], text=True).strip())
target = next(t for t in yaml.safe_load((root / "build.yaml").read_text())["include"]
              if t.get("artifact-name") == args.target)
subprocess.run(["python3", str(root / "scripts/apply-bt-recovery.py"), str(zmk)], check=True)
subprocess.run(["west", "build", "-s", str(zmk / "app"), "-d", str(workspace / "build-bt-recovery" / args.target),
               "-b", target["board"], "-S", target["snippet"] + " bt-recovery", "-o=-j4", "--",
               "-DZMK_CONFIG=" + str(root / "config"), "-DZMK_EXTRA_MODULES=" + str(root),
               "-DZephyr_DIR=" + str(zephyr / "share/zephyr-package/cmake"),
               "-DSHIELD=" + target["shield"]], check=True)
