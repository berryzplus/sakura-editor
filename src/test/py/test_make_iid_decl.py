import importlib.util
from pathlib import Path

import pytest

def _load_module():
    repo_root = Path(__file__).resolve().parents[3]
    module_path = repo_root / "src" / "main" / "py" / "make_iid_decl.py"
    spec = importlib.util.spec_from_file_location("make_iid_decl", module_path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Failed to load module spec: {module_path}")

    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


@pytest.fixture(scope="module")
def make_iid_decl_module():
    return _load_module()


def test_generate_iid_declarations_filters_and_dedups(tmp_path, make_iid_decl_module):
    source = tmp_path / "sakura_i.c"
    output = tmp_path / "out" / "sakura_iid_decl.hpp"
    source.write_text(
        "\n".join(
            [
                "MIDL_DEFINE_GUID(IID, IID_ITrayWnd,0x11111111,0x2222,0x3333,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB);",
                "MIDL_DEFINE_GUID(IID, LIBID_SakuraEditorLib,0xAAAA0000,0xBBBB,0xCCCC,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88);",
                "MIDL_DEFINE_GUID(CLSID, CLSID_TrayWnd,0xDDDD0000,0xEEEE,0xFFFF,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88);",
                "MIDL_DEFINE_GUID(IID, IID_IOther,0x22222222,0x3333,0x4444,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC);",
                "MIDL_DEFINE_GUID(IID, IID_ITrayWnd,0x11111111,0x2222,0x3333,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB);",
            ]
        ),
        encoding="utf-8",
    )

    count = make_iid_decl_module.generate_iid_declarations(source, output)

    text = output.read_text(encoding="utf-8")
    assert count == 2
    assert "#pragma once" in text
    assert "#include <guiddef.h>" in text
    assert "__CRT_UUID_DECL(ITrayWnd,0x11111111,0x2222,0x3333,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB)" in text
    assert "__CRT_UUID_DECL(IOther,0x22222222,0x3333,0x4444,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC)" in text
    assert "LIBID_SakuraEditorLib" not in text
    assert "CLSID_TrayWnd" not in text
    assert text.count("__CRT_UUID_DECL(ITrayWnd") == 1


def test_generate_iid_declarations_raises_when_no_iid(tmp_path, make_iid_decl_module):
    source = tmp_path / "sakura_i.c"
    output = tmp_path / "sakura_iid_decl.hpp"
    source.write_text(
        "\n".join(
            [
                "MIDL_DEFINE_GUID(IID, LIBID_SakuraEditorLib,0xAAAA0000,0xBBBB,0xCCCC,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88);",
                "MIDL_DEFINE_GUID(CLSID, CLSID_TrayWnd,0xDDDD0000,0xEEEE,0xFFFF,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88);",
            ]
        ),
        encoding="utf-8",
    )

    with pytest.raises(RuntimeError, match="No IID definitions matched"):
        make_iid_decl_module.generate_iid_declarations(source, output)


def test_generate_iid_declarations_creates_parent_directory(tmp_path, make_iid_decl_module):
    source = tmp_path / "sakura_i.c"
    output = tmp_path / "nested" / "dir" / "sakura_iid_decl.hpp"
    source.write_text(
        "MIDL_DEFINE_GUID(IID, IID_ITrayWnd,0x11111111,0x2222,0x3333,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB);",
        encoding="utf-8",
    )

    make_iid_decl_module.generate_iid_declarations(source, output)

    assert output.exists()


def test_main_returns_2_when_argument_count_is_invalid(monkeypatch, capsys, make_iid_decl_module):
    monkeypatch.setattr(make_iid_decl_module.sys, "argv", ["make_iid_decl.py"])

    result = make_iid_decl_module.main()
    captured = capsys.readouterr()

    assert result == 2
    assert "Usage: make_iid_decl.py" in captured.err


def test_main_returns_1_when_input_file_is_missing(tmp_path, monkeypatch, capsys, make_iid_decl_module):
    missing = tmp_path / "missing.c"
    output = tmp_path / "sakura_iid_decl.hpp"
    monkeypatch.setattr(
        make_iid_decl_module.sys,
        "argv",
        ["make_iid_decl.py", str(missing), str(output)],
    )

    result = make_iid_decl_module.main()
    captured = capsys.readouterr()

    assert result == 1
    assert "Input file not found" in captured.err


def test_main_returns_0_on_success(tmp_path, monkeypatch, capsys, make_iid_decl_module):
    source = tmp_path / "sakura_i.c"
    output = tmp_path / "sakura_iid_decl.hpp"
    source.write_text(
        "MIDL_DEFINE_GUID(IID, IID_ITrayWnd,0x11111111,0x2222,0x3333,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB);",
        encoding="utf-8",
    )

    monkeypatch.setattr(
        make_iid_decl_module.sys,
        "argv",
        ["make_iid_decl.py", str(source), str(output)],
    )

    result = make_iid_decl_module.main()
    captured = capsys.readouterr()

    assert result == 0
    assert "Generated 1 IID declarations" in captured.out
    assert output.exists()
