#!/usr/bin/env python3
"""
Verify that for every header that includes a `*.generated.h`, at least one
corresponding implementation file contains `#include UE_INLINE_GENERATED_CPP_BY_NAME(<BaseName>)`
and that this include appears after all other `#include` directives.

Usage:
  python3 bin/check_inline_generated_cpp_includes.py [--root <path>]

Exit codes:
  0 = OK (no violations)
  1 = Violations found

Notes:
  - Scans only under `<root>/Source/` to avoid traversing host repos.
  - Excludes files under `Tests/` because automation test types can legitimately omit inline generated includes.
  - A "corresponding" .cpp is any source file that contains the macro include for the base name.
    If none contain the macro include, we also try to find a `.cpp` named `<BaseName>.cpp` or any
    `.cpp` that includes `<BaseName>.h` and report a missing-include violation.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from typing import Dict, List, Set, Tuple


HEADER_GENERATED_INCLUDE_RE = re.compile(r"^\s*#\s*include\s*[\"<]([^\">]+)\.generated\.h[\">]", re.MULTILINE)
CPP_INCLUDE_RE = re.compile(r"^\s*#\s*include\b.*", re.MULTILINE)
CPP_INLINE_GEN_PREFIX_RE = re.compile(r"^\s*#\s*include\s+UE_INLINE_GENERATED_CPP_BY_NAME\(.*\)\s*$")


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8", errors="ignore")
    except Exception:
        return ""


def is_test_file(path: Path) -> bool:
    return "Tests" in path.parts


def find_generated_headers(source_root: Path) -> Dict[Path, Set[str]]:
    headers: Dict[Path, Set[str]] = {}
    for p in source_root.rglob("*.h"):
        if is_test_file(p):
            continue
        text = read_text(p)
        matches = HEADER_GENERATED_INCLUDE_RE.findall(text)
        if matches:
            bases = {Path(m).stem.replace(".generated", "") for m in matches}
            headers[p] = bases
    return headers


def list_cpp_files(source_root: Path) -> List[Path]:
    return [path for path in source_root.rglob("*.cpp") if not is_test_file(path)]


def cpp_contains_macro_for(text: str, base: str) -> bool:
    pattern = re.compile(
        r"^\s*#\s*include\s+UE_INLINE_GENERATED_CPP_BY_NAME\(\s*" + re.escape(base) + r"\s*\)\s*$", re.MULTILINE
    )
    return bool(pattern.search(text))


def macro_is_after_all_other_includes(text: str, base: str) -> bool:
    # Ensure the macro include for `base` appears after the last non-macro include.
    includes = [(m.start(), m.group(0)) for m in CPP_INCLUDE_RE.finditer(text)]
    if not includes:
        return False
    macro_line_re = re.compile(
        r"^\s*#\s*include\s+UE_INLINE_GENERATED_CPP_BY_NAME\(\s*" + re.escape(base) + r"\s*\)\s*$"
    )
    idx_macro = None
    last_non_macro_idx = -1
    for i, (_, line) in enumerate(includes):
        if macro_line_re.match(line):
            if idx_macro is None:
                idx_macro = i
        elif not CPP_INLINE_GEN_PREFIX_RE.match(line):
            last_non_macro_idx = i
    if idx_macro is None:
        return False
    return idx_macro > last_non_macro_idx


def cpp_includes_header(text: str, base: str) -> bool:
    # Heuristically check whether this .cpp includes `<Base>.h` (any path)
    pat = re.compile(r"^\s*#\s*include\s*[\"<][^\">]*" + re.escape(base) + r"\.h[\">]", re.MULTILINE)
    return bool(pat.search(text))


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--root",
        type=Path,
        default=Path(__file__).resolve().parent.parent,
        help="Plugin root (defaults to repository root containing Source/)",
    )
    args = ap.parse_args()

    plugin_root: Path = args.root
    source_root: Path = plugin_root / "Source"
    if not source_root.exists():
        print(f"[WARN] No Source/ directory under {plugin_root}")
        return 0

    headers = find_generated_headers(source_root)
    if not headers:
        print("OK: No headers include any *.generated.h; nothing to check.")
        return 0

    cpp_files = list_cpp_files(source_root)
    cpp_text_cache: Dict[Path, str] = {p: read_text(p) for p in cpp_files}

    violations: List[str] = []

    for header_path, bases in headers.items():
        for base in bases:
            # Find candidate .cpp files
            candidates: List[Tuple[Path, str]] = []
            for p, txt in cpp_text_cache.items():
                if cpp_contains_macro_for(txt, base):
                    candidates.append((p, txt))

            if not candidates:
                # Search for likely owning .cpp to report a more helpful error.
                name_match = [p for p in cpp_files if p.stem == base]
                include_match = [p for p, txt in cpp_text_cache.items() if cpp_includes_header(txt, base)]
                hints: List[str] = []
                if name_match:
                    hints.append("found candidate by name: " + ", ".join(str(p) for p in name_match))
                if include_match:
                    hints.append("found candidate including header: " + ", ".join(str(p) for p in include_match))
                hint_text = (" (" + "; ".join(hints) + ")") if hints else ""
                violations.append(
                    f"Missing UE_INLINE_GENERATED_CPP_BY_NAME({base}) for header {header_path}{hint_text}"
                )
                continue

            # Verify placement/order in all candidates that include the macro
            for p, txt in candidates:
                if not macro_is_after_all_other_includes(txt, base):
                    violations.append(
                        f"Incorrect include order in {p}: UE_INLINE_GENERATED_CPP_BY_NAME({base}) must appear after all other #include directives."
                    )

    if violations:
        print("Generated-include check failed:\n" + "\n".join("- " + v for v in violations))
        return 1

    print("OK: All inline-generated includes present and correctly ordered.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
