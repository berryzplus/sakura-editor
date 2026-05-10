import re
import sys
from pathlib import Path

PATTERN = re.compile(r"MIDL_DEFINE_GUID\(IID, IID_(I\w+),(.+?)\);")


def generate_iid_declarations(source_path: Path, output_path: Path) -> int:
    source_text = source_path.read_text(encoding="utf-8")
    matches = PATTERN.findall(source_text)

    if not matches:
        raise RuntimeError(f"No IID definitions matched in: {source_path}")

    seen = set()
    ordered = []
    for name, guid_args in matches:
        decl = f"__CRT_UUID_DECL({name},{guid_args})"
        if decl not in seen:
            seen.add(decl)
            ordered.append(decl)

    lines = [
        "// This file is auto-generated from src/main/resources/sakura_i.c. Do not edit.",
        "#pragma once",
        "",
        "#include <guiddef.h>",
        "",
    ]
    lines.extend(ordered)
    lines.append("")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text("\n".join(lines), encoding="utf-8", newline="\n")
    return len(ordered)


def main() -> int:
    if len(sys.argv) != 3:
        print("Usage: make_iid_decl.py <input_sakura_i.c> <output_sakura_iid_decl.hpp>", file=sys.stderr)
        return 2

    source_path = Path(sys.argv[1])
    output_path = Path(sys.argv[2])

    if not source_path.is_file():
        print(f"Input file not found: {source_path}", file=sys.stderr)
        return 1

    try:
        count = generate_iid_declarations(source_path, output_path)
    except Exception as exc:
        print(f"Failed to generate IID declaration header: {exc}", file=sys.stderr)
        return 1

    print(f"Generated {count} IID declarations: {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
