#!/usr/bin/env python3
from __future__ import annotations

import html
import json
import os
import signal
import subprocess
import threading
import time
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Optional
from urllib.parse import parse_qs, urlparse

from convert_recipe_to_yaml import default_output_path, parse_recipe, render_yaml


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_METADATA_ROOTS = [
    Path("/home/cj/usb_1T_disk/Dropbox/19-22-ing/.nppm-meta"),
    Path("/home/cj/Dropbox/19-22-ing/.nppm-meta"),
]
DEFAULT_DROPBOX_ROOTS = [
    Path("/home/cj/usb_1T_disk/Dropbox"),
    Path("/home/cj/Dropbox"),
]
ACTIVE_RECIPE_NAME = "active_recipe.txt"
ACTIVE_RECIPES_STATE_NAME = "active_recipes.yaml"
DEFAULT_SUBDIR = "defaultS"
APP_BINARY = REPO_ROOT / "build/apps/nppm/x_mainmoil"
DEFAULT_DEMO_TIMEOUT_SECONDS = 10.0
VISIBLE_RECIPE_KINDS = {"cast", "script", "yaml"}
RUNNABLE_RECIPE_KINDS = {"cast", "yaml"}
ACTIVE_RECIPE_KINDS = ("cast", "yaml")


class AppState:
    def __init__(self) -> None:
        self.process: Optional[subprocess.Popen[str]] = None
        self.log_lines: list[str] = []
        self.lock = threading.Lock()
        self.demo_thread: Optional[threading.Thread] = None
        self.demo_stop_event = threading.Event()
        self.demo_timeout_seconds = DEFAULT_DEMO_TIMEOUT_SECONDS
        self.demo_recipe_names: list[str] = []
        self.demo_current_recipe = ""
        self.demo_deadline_monotonic: Optional[float] = None
        self.demo_switch_at_unix: Optional[float] = None

    def append_log(self, line: str) -> None:
        with self.lock:
            self.log_lines.append(line.rstrip("\n"))
            self.log_lines = self.log_lines[-400:]

    def snapshot(self) -> dict:
        with self.lock:
            process = self.process
            running = process is not None and process.poll() is None
            pid = process.pid if running else None
            demo_running = self.demo_thread is not None and self.demo_thread.is_alive()
            countdown_seconds = None
            if demo_running and self.demo_deadline_monotonic is not None:
                countdown_seconds = max(0.0, self.demo_deadline_monotonic - time.monotonic())
            return {
                "running": running,
                "pid": pid,
                "demo_running": demo_running,
                "demo_timeout_seconds": self.demo_timeout_seconds,
                "demo_recipes": list(self.demo_recipe_names),
                "demo_current_recipe": self.demo_current_recipe,
                "demo_countdown_seconds": countdown_seconds,
                "demo_switch_at_unix_ms": (
                    int(self.demo_switch_at_unix * 1000)
                    if self.demo_switch_at_unix is not None
                    else None
                ),
                "log": "\n".join(self.log_lines[-120:]),
            }


STATE = AppState()


def detect_metadata_root() -> Path:
    env_meta = os.environ.get("CJ_NPPM_META")
    if env_meta:
        path = Path(env_meta)
        if path.exists():
            return path

    local_meta = REPO_ROOT / ".nppm-meta"
    if local_meta.exists():
        return local_meta

    for path in DEFAULT_METADATA_ROOTS:
        if path.exists():
            return path

    raise FileNotFoundError("Unable to locate metadata root")


def detect_dropbox_root() -> Path | None:
    env_root = os.environ.get("CJ_NPPM_DROPBOX_ROOT")
    if env_root:
        path = Path(env_root)
        if path.exists():
            return path

    for path in DEFAULT_DROPBOX_ROOTS:
        if path.exists():
            return path

    return None


def runtime_env() -> dict[str, str]:
    env = os.environ.copy()
    env.setdefault("DISPLAY", env.get("DISPLAY", ":0"))
    dropbox_root = detect_dropbox_root()
    if dropbox_root is not None:
        env.setdefault("CJ_NPPM_DROPBOX_ROOT", str(dropbox_root))
    return env


def recipe_dir() -> Path:
    return detect_metadata_root() / DEFAULT_SUBDIR


def active_recipe_path() -> Path:
    return recipe_dir() / ACTIVE_RECIPE_NAME


def active_recipes_state_path() -> Path:
    return recipe_dir() / ACTIVE_RECIPES_STATE_NAME


def recipe_kind_for_name(recipe_name: str) -> str:
    return recipe_item_by_name(recipe_name)["kind"]


def list_recipe_items() -> list[dict]:
    directory = recipe_dir()
    actual_files = sorted(
        p for p in directory.iterdir()
        if p.is_file() and p.suffix.lower() in {".cast", ".script", ".yml", ".yaml"}
    )

    items: list[dict] = []
    for path in actual_files:
        name = path.name
        suffix = path.suffix.lower()
        if suffix == ".cast":
            kind = "cast"
        elif suffix == ".script":
            kind = "script"
        elif suffix in {".yml", ".yaml"}:
            kind = "yaml"
        else:
            kind = "other"
        items.append({
            "name": name,
            "kind": kind,
            "path": str(path),
        })
    return items


def recipe_item_by_name(recipe_name: str) -> dict:
    for item in list_recipe_items():
        if item["name"] == recipe_name:
            return item
    raise ValueError(f"Unknown recipe file: {recipe_name}")


def list_recipe_files() -> list[str]:
    files = [
        item["name"]
        for item in list_recipe_items()
        if item["kind"] in RUNNABLE_RECIPE_KINDS
    ]
    if not files:
        raise FileNotFoundError("No runnable .cast or .yml/.yaml files found in metadata root")
    return files


def recipe_file_path(recipe_name: str, *, allowed_kinds: set[str] | None = None) -> Path:
    item = recipe_item_by_name(recipe_name)
    if allowed_kinds is not None and item["kind"] not in allowed_kinds:
        expected = ", ".join(sorted(allowed_kinds))
        raise ValueError(f"{recipe_name} is type {item['kind']}, expected one of: {expected}")
    return recipe_dir() / recipe_name


def read_legacy_active_recipe() -> str:
    path = active_recipe_path()
    if not path.exists():
        return ""
    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if line and not line.startswith("#"):
            try:
                recipe_file_path(line, allowed_kinds=RUNNABLE_RECIPE_KINDS)
                return line
            except Exception:
                return ""
    return ""


def write_legacy_active_recipe(recipe_name: str) -> None:
    recipe_file_path(recipe_name, allowed_kinds=RUNNABLE_RECIPE_KINDS)
    active_recipe_path().write_text(
        "# active recipe for NPPM\n" + recipe_name + "\n",
        encoding="utf-8",
    )


def empty_active_state() -> dict:
    return {
        "active": {kind: "" for kind in ACTIVE_RECIPE_KINDS},
        "default_kind": "yaml",
    }


def parse_scalar_value(value: str) -> str:
    text = value.strip()
    if not text:
        return ""
    if (text.startswith('"') and text.endswith('"')) or (text.startswith("'") and text.endswith("'")):
        return text[1:-1]
    return text


def sanitize_active_state(state: dict) -> dict:
    normalized = empty_active_state()
    active = state.get("active", {})
    if isinstance(active, dict):
        for kind in ACTIVE_RECIPE_KINDS:
            recipe_name = active.get(kind, "")
            if isinstance(recipe_name, str):
                recipe_name = recipe_name.strip()
                if recipe_name:
                    try:
                        recipe_file_path(recipe_name, allowed_kinds=RUNNABLE_RECIPE_KINDS)
                        if recipe_kind_for_name(recipe_name) == kind:
                            normalized["active"][kind] = recipe_name
                    except Exception:
                        pass
    default_kind = state.get("default_kind", "yaml")
    if default_kind in ACTIVE_RECIPE_KINDS:
        normalized["default_kind"] = default_kind
    return normalized


def derive_initial_active_state() -> dict:
    state = empty_active_state()
    legacy_recipe = read_legacy_active_recipe()
    if legacy_recipe:
        try:
            kind = recipe_kind_for_name(legacy_recipe)
            state["active"][kind] = legacy_recipe
            state["default_kind"] = kind
        except Exception:
            pass
    return state


def read_active_state() -> dict:
    path = active_recipes_state_path()
    if not path.exists():
        return derive_initial_active_state()

    state = empty_active_state()
    current_section = ""
    for raw_line in path.read_text(encoding="utf-8").splitlines():
        stripped = raw_line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        if raw_line.startswith((" ", "\t")):
            if current_section == "active":
                key, sep, value = stripped.partition(":")
                if sep and key in ACTIVE_RECIPE_KINDS:
                    state["active"][key] = parse_scalar_value(value)
            continue

        key, sep, value = stripped.partition(":")
        if not sep:
            continue
        key = key.strip()
        if key == "active":
            current_section = "active"
        elif key == "default_kind":
            state["default_kind"] = parse_scalar_value(value)
            current_section = ""
        else:
            current_section = ""

    return sanitize_active_state(state)


def resolved_default_active_recipe(active_state: dict) -> str:
    default_kind = active_state["default_kind"]
    preferred_order = [default_kind] + [kind for kind in ACTIVE_RECIPE_KINDS if kind != default_kind]
    for kind in preferred_order:
        recipe_name = active_state["active"].get(kind, "")
        if recipe_name:
            return recipe_name
    return ""


def write_active_state(active_state: dict) -> dict:
    normalized = sanitize_active_state(active_state)
    path = active_recipes_state_path()
    lines = [
        "active:",
        f'  cast: "{normalized["active"]["cast"]}"',
        f'  yaml: "{normalized["active"]["yaml"]}"',
        f'default_kind: {normalized["default_kind"]}',
        "",
    ]
    path.write_text("\n".join(lines), encoding="utf-8")
    legacy_recipe = resolved_default_active_recipe(normalized)
    if legacy_recipe:
        write_legacy_active_recipe(legacy_recipe)
    return normalized


def read_active_recipe(kind: str | None = None) -> str:
    active_state = read_active_state()
    if kind in ACTIVE_RECIPE_KINDS:
        return active_state["active"].get(kind, "")
    return resolved_default_active_recipe(active_state)


def write_active_recipe(recipe_name: str, kind: str | None = None, default_kind: str | None = None) -> dict:
    recipe_file_path(recipe_name, allowed_kinds=RUNNABLE_RECIPE_KINDS)
    recipe_kind = kind or recipe_kind_for_name(recipe_name)
    if recipe_kind not in ACTIVE_RECIPE_KINDS:
        raise ValueError(f"Unsupported recipe kind: {recipe_kind}")
    active_state = read_active_state()
    active_state["active"][recipe_kind] = recipe_name
    active_state["default_kind"] = default_kind if default_kind in ACTIVE_RECIPE_KINDS else recipe_kind
    return write_active_state(active_state)


def read_recipe_file(recipe_name: str) -> str:
    return recipe_file_path(recipe_name, allowed_kinds=VISIBLE_RECIPE_KINDS).read_text(encoding="utf-8")


def write_recipe_file(recipe_name: str, content: str) -> None:
    path = recipe_file_path(recipe_name, allowed_kinds=VISIBLE_RECIPE_KINDS)
    path.write_text(content, encoding="utf-8")


def convert_recipe_file_to_yaml(recipe_name: str, content: str | None = None) -> dict:
    path = recipe_file_path(recipe_name, allowed_kinds={"cast", "script"})
    prior_active_state = read_active_state()
    prior_demo_recipes = get_demo_recipe_names()
    STATE.append_log(f"[convert {recipe_name}] starting YAML transfer")
    if content is not None:
        path.write_text(content, encoding="utf-8")
    output_path = default_output_path(path)
    parsed = parse_recipe(path)
    output_path.write_text(render_yaml(parsed), encoding="utf-8")
    new_recipe_name = output_path.name
    source_kind = recipe_kind_for_name(recipe_name)
    if prior_active_state["active"].get(source_kind) == recipe_name:
        prior_active_state["active"]["yaml"] = new_recipe_name
        if prior_active_state["default_kind"] == source_kind:
            prior_active_state["default_kind"] = "yaml"
        write_active_state(prior_active_state)
    if recipe_name in prior_demo_recipes:
        updated_demo_recipes = [
            new_recipe_name if item == recipe_name else item
            for item in prior_demo_recipes
        ]
        set_demo_recipe_names(updated_demo_recipes)
    STATE.append_log(f"[convert {recipe_name}] wrote {new_recipe_name}")
    active_state = read_active_state()
    return {
        "ok": True,
        "recipe": new_recipe_name,
        "path": str(output_path),
        "active_recipe": read_active_recipe(),
        "active_recipes": active_state["active"],
        "default_kind": active_state["default_kind"],
        "demo_recipes": get_demo_recipe_names(),
    }


def normalize_demo_recipe_names(recipe_names: list[str]) -> list[str]:
    available = list_recipe_files()
    requested = set(recipe_names)
    selected = [recipe for recipe in available if recipe in requested]
    return selected


def get_demo_recipe_names() -> list[str]:
    available = list_recipe_files()
    with STATE.lock:
        stored = list(STATE.demo_recipe_names)
    selected = [recipe for recipe in available if recipe in set(stored)]
    return selected or available


def set_demo_recipe_names(recipe_names: list[str]) -> list[str]:
    selected = normalize_demo_recipe_names(recipe_names)
    with STATE.lock:
        STATE.demo_recipe_names = list(selected)
    return selected


def _stream_output(process: subprocess.Popen[str]) -> None:
    assert process.stdout is not None
    for line in process.stdout:
        STATE.append_log(line)
    process.wait()
    STATE.append_log(f"[process exited with code {process.returncode}]")
    with STATE.lock:
        if STATE.process is process:
            STATE.process = None


def run_app() -> None:
    with STATE.lock:
        if STATE.process is not None and STATE.process.poll() is None:
            return

        env = runtime_env()
        process = subprocess.Popen(
            [str(APP_BINARY)],
            cwd=REPO_ROOT,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )
        STATE.process = process
        STATE.log_lines.append(f"[started pid {process.pid}]")
        threading.Thread(target=_stream_output, args=(process,), daemon=True).start()


def check_recipe(recipe_name: str) -> dict:
    path = recipe_file_path(recipe_name, allowed_kinds=RUNNABLE_RECIPE_KINDS)
    env = runtime_env()
    process = subprocess.run(
        [str(APP_BINARY), "--check-recipe", recipe_name],
        cwd=REPO_ROOT,
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )
    output = process.stdout.rstrip()
    if output:
        for line in output.splitlines():
            STATE.append_log(f"[check {recipe_name}] {line}")
    STATE.append_log(f"[check {recipe_name}] exited with code {process.returncode}")
    return {
        "ok": process.returncode == 0,
        "recipe": recipe_name,
        "path": str(path),
        "exit_code": process.returncode,
        "output": output,
    }


def stop_app() -> None:
    with STATE.lock:
        process = STATE.process
    if process is None or process.poll() is not None:
        return
    process.send_signal(signal.SIGINT)


def _run_recipe_and_wait(recipe_name: str) -> None:
    write_active_recipe(recipe_name)
    STATE.append_log(f"[continuous demo selected {recipe_name}]")
    run_app()
    with STATE.lock:
        timeout_seconds = STATE.demo_timeout_seconds
        STATE.demo_current_recipe = recipe_name
        if timeout_seconds <= 0:
            STATE.demo_deadline_monotonic = None
            STATE.demo_switch_at_unix = None
        else:
            STATE.demo_deadline_monotonic = time.monotonic() + timeout_seconds
            STATE.demo_switch_at_unix = time.time() + timeout_seconds
        deadline = STATE.demo_deadline_monotonic
    try:
        while True:
            with STATE.lock:
                process = STATE.process
            if process is None or process.poll() is not None:
                return
            if deadline is not None and time.monotonic() >= deadline:
                STATE.append_log(
                    f"[continuous demo timeout after {timeout_seconds:.1f}s, stopping app]"
                )
                stop_app()
                deadline = None
            if STATE.demo_stop_event.wait(0.1):
                stop_app()
                return
    finally:
        with STATE.lock:
            STATE.demo_deadline_monotonic = None
            STATE.demo_switch_at_unix = None
            STATE.demo_current_recipe = ""


def _continuous_demo_worker() -> None:
    try:
        recipes = get_demo_recipe_names()
        if not recipes:
            STATE.append_log("[continuous demo aborted: no recipes selected]")
            return
        while not STATE.demo_stop_event.is_set():
            for recipe in recipes:
                if STATE.demo_stop_event.is_set():
                    return
                _run_recipe_and_wait(recipe)
                if STATE.demo_stop_event.wait(0.1):
                    return
    finally:
        with STATE.lock:
            STATE.demo_thread = None
            STATE.demo_deadline_monotonic = None
            STATE.demo_switch_at_unix = None
            STATE.demo_current_recipe = ""
        STATE.demo_stop_event.clear()
        STATE.append_log("[continuous demo stopped]")


def start_continuous_demo(timeout_seconds: float = DEFAULT_DEMO_TIMEOUT_SECONDS) -> None:
    recipes = get_demo_recipe_names()
    if not recipes:
        raise ValueError("Select at least one recipe for Continuous Demo")
    with STATE.lock:
        if STATE.demo_thread is not None and STATE.demo_thread.is_alive():
            return
        STATE.demo_timeout_seconds = max(0.0, timeout_seconds)
        STATE.demo_stop_event.clear()
        thread = threading.Thread(target=_continuous_demo_worker, daemon=True)
        STATE.demo_thread = thread
        if STATE.demo_timeout_seconds > 0:
            STATE.log_lines.append(
                f"[continuous demo started with {len(recipes)} recipes, auto-stop {STATE.demo_timeout_seconds:.1f}s per recipe]"
            )
        else:
            STATE.log_lines.append(
                f"[continuous demo started with {len(recipes)} recipes, manual exit per recipe]"
            )
        thread.start()


def stop_continuous_demo() -> None:
    STATE.demo_stop_event.set()
    stop_app()


HTML_PAGE = """<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>NPPM Control</title>
  <style>
    :root {
      --bg: #f3f0e8;
      --panel: #fffaf0;
      --ink: #1f2933;
      --accent: #8f3b1b;
      --accent-2: #244f47;
      --border: #d8cbb8;
    }
    body {
      margin: 0;
      font-family: "Iosevka Aile", "IBM Plex Sans", sans-serif;
      background: radial-gradient(circle at top left, #fff6d9, var(--bg) 45%);
      color: var(--ink);
    }
    .wrap {
      max-width: 920px;
      margin: 0 auto;
      padding: 32px 20px 48px;
    }
    .hero {
      margin-bottom: 24px;
    }
    h1 {
      margin: 0 0 8px;
      font-size: 2.2rem;
      letter-spacing: -0.04em;
    }
    .sub {
      color: #5b6570;
      max-width: 760px;
    }
    .grid {
      display: grid;
      grid-template-columns: 1.1fr 0.9fr;
      gap: 20px;
    }
    .card {
      background: var(--panel);
      border: 1px solid var(--border);
      border-radius: 18px;
      padding: 18px;
      box-shadow: 0 14px 40px rgba(71, 53, 35, 0.08);
    }
    h2 {
      margin-top: 0;
      font-size: 1.1rem;
    }
    label, .meta {
      display: block;
      margin-bottom: 8px;
      color: #5b6570;
      font-size: 0.95rem;
    }
    button {
      font: inherit;
    }
    .actions {
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
    }
    .field-row {
      display: flex;
      align-items: end;
      gap: 12px;
      margin-bottom: 14px;
      flex-wrap: wrap;
    }
    .field {
      min-width: 180px;
      flex: 1 1 220px;
    }
    button {
      border: 0;
      border-radius: 999px;
      padding: 11px 16px;
      cursor: pointer;
      color: white;
      background: var(--accent);
    }
    input[type="number"] {
      width: 100%;
      padding: 12px;
      border-radius: 12px;
      border: 1px solid var(--border);
      background: white;
      box-sizing: border-box;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      margin-bottom: 14px;
      background: white;
      border: 1px solid var(--border);
      border-radius: 14px;
      overflow: hidden;
    }
    th, td {
      padding: 10px 12px;
      border-bottom: 1px solid var(--border);
      text-align: left;
      vertical-align: middle;
    }
    th {
      color: #5b6570;
      font-size: 0.9rem;
      background: #faf4ea;
    }
    tr:last-child td {
      border-bottom: 0;
    }
    td.compact, th.compact {
      width: 72px;
      text-align: center;
    }
    .recipe-name {
      font-family: "Iosevka Term", "SFMono-Regular", monospace;
      font-size: 0.92rem;
    }
    button.alt {
      background: var(--accent-2);
    }
    button.neutral {
      background: #6b7280;
    }
    pre {
      margin: 0;
      min-height: 360px;
      max-height: 520px;
      overflow: auto;
      padding: 14px;
      border-radius: 14px;
      background: #15191d;
      color: #d7e2f0;
      font-family: "Iosevka Term", "SFMono-Regular", monospace;
      font-size: 0.9rem;
      line-height: 1.4;
    }
    .status-dot {
      display: inline-block;
      width: 10px;
      height: 10px;
      border-radius: 50%;
      margin-right: 8px;
      background: #dc2626;
    }
    .status-dot.running {
      background: #16a34a;
    }
    .countdown {
      color: var(--accent);
      font-weight: 600;
      min-height: 1.2em;
    }
    .editor-card {
      grid-column: 1 / -1;
      display: none;
    }
    .editor-card.open {
      display: block;
    }
    textarea {
      width: 100%;
      min-height: 420px;
      padding: 14px;
      border-radius: 14px;
      border: 1px solid var(--border);
      background: #fffdf8;
      box-sizing: border-box;
      font-family: "Iosevka Term", "SFMono-Regular", monospace;
      font-size: 0.92rem;
      line-height: 1.45;
      resize: vertical;
    }
    .editor-meta {
      margin: 0 0 10px;
      color: #5b6570;
      font-size: 0.95rem;
    }
    .editor-tools {
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
      margin-bottom: 12px;
      align-items: end;
    }
    .editor-tools .field {
      min-width: 180px;
      flex: 1 1 220px;
    }
    .editor-layout {
      display: grid;
      grid-template-columns: 160px 1fr;
      gap: 12px;
      align-items: start;
    }
    .editor-side-actions {
      display: flex;
      flex-direction: column;
      gap: 10px;
    }
    .editor-side-actions button {
      width: 100%;
    }
    input[type="text"] {
      width: 100%;
      padding: 12px;
      border-radius: 12px;
      border: 1px solid var(--border);
      background: white;
      box-sizing: border-box;
    }
    .filters {
      display: flex;
      flex-wrap: wrap;
      gap: 12px;
      margin-bottom: 12px;
      align-items: center;
    }
    .filters label {
      display: inline-flex;
      align-items: center;
      gap: 6px;
      margin: 0;
      font-size: 0.92rem;
      color: var(--ink);
    }
    .filters input[type="radio"] {
      margin: 0;
    }
    .legacy-note {
      margin: 8px 0 12px;
      color: var(--accent);
      font-size: 0.92rem;
    }
    .recipe-name.legacy {
      color: #8f3b1b;
    }
    @media (max-width: 780px) {
      .grid {
        grid-template-columns: 1fr;
      }
    }
  </style>
</head>
<body>
  <div class="wrap">
    <div class="hero">
      <h1>NPPM Control Panel</h1>
      <div class="sub">Select recipe files from defaultS, switch between cast and yaml views, edit the current file type, and run or check the supported runnable recipe types. Legacy script files remain available for conversion.</div>
    </div>
    <div class="grid">
      <div class="card">
        <h2>Recipe Selection</h2>
        <label id="recipe-list-label">Cast Recipe Files</label>
        <div class="filters">
          <label><input id="filter-cast" type="radio" name="recipe-type" onchange="setRecipeKind('cast')"> cast</label>
          <label><input id="filter-yaml" type="radio" name="recipe-type" checked onchange="setRecipeKind('yaml')"> yaml</label>
        </div>
        <table>
          <thead>
            <tr>
              <th class="compact">Active</th>
              <th class="compact">
                Demo
                <div style="margin-top: 6px;">
                  <button type="button" class="neutral" style="padding: 5px 10px;" onclick="toggleAllDemo()">All</button>
                </div>
              </th>
              <th>Recipe file</th>
            </tr>
          </thead>
          <tbody id="recipe-table"></tbody>
        </table>
        <div class="field-row">
          <div class="field">
            <label for="demo-timeout">Continuous Demo timeout (seconds)</label>
            <input id="demo-timeout" type="number" min="0" step="1" value="10">
          </div>
        </div>
        <div class="field-row">
          <div class="field">
            <label for="legacy-script-select">Legacy Script Converter</label>
            <select id="legacy-script-select" style="width: 100%; padding: 12px; border-radius: 12px; border: 1px solid var(--border); background: white; box-sizing: border-box;"></select>
          </div>
          <div class="field" style="flex: 0 0 auto; min-width: 200px;">
            <label>&nbsp;</label>
            <button type="button" class="neutral" style="width: 100%;" onclick="openLegacyScript()">Open Legacy Script</button>
          </div>
        </div>
        <p class="legacy-note">Script recipes are legacy migration inputs. Open them here, then use <strong>Transfer to YAML</strong>.</p>
        <div class="actions">
          <button id="save-selection-button" onclick="saveRecipe()">Save Selection</button>
          <button id="check-recipe-button" onclick="checkRecipe()">Check Recipe</button>
          <button class="alt" onclick="openEditor()">Open Editor</button>
          <button id="run-app-button" class="alt" onclick="runApp()">Run App</button>
          <button id="demo-button" class="alt" onclick="toggleDemo()">Continuous Demo</button>
          <button class="neutral" onclick="stopApp()">Stop App</button>
        </div>
        <p class="meta" id="meta-root"></p>
        <p class="meta" id="dropbox-root"></p>
        <p class="meta" id="active-recipe"></p>
      </div>
      <div class="card">
        <h2>Status</h2>
        <p class="meta"><span id="status-dot" class="status-dot"></span><span id="status-text">Stopped</span></p>
        <p class="meta countdown" id="demo-countdown"></p>
        <pre id="log"></pre>
      </div>
      <div id="editor-card" class="card editor-card">
        <h2>Recipe Editor</h2>
        <p id="editor-path" class="editor-meta"></p>
        <div class="editor-tools">
          <div class="field">
            <label for="editor-find">Find</label>
            <input id="editor-find" type="text">
          </div>
          <div class="field">
            <label for="editor-replace">Replace</label>
            <input id="editor-replace" type="text">
          </div>
        </div>
        <div class="editor-layout">
          <div class="editor-side-actions">
            <button type="button" class="neutral" onclick="findNextInEditor()">Find Next</button>
            <button type="button" class="neutral" onclick="replaceNextInEditor()">Replace Next</button>
            <button type="button" class="neutral" onclick="replaceAllInEditor()">Replace All</button>
          </div>
          <textarea id="recipe-editor" spellcheck="false"></textarea>
        </div>
        <div class="actions" style="margin-top: 12px;">
          <button onclick="saveEditor()">Save Recipe</button>
          <button id="convert-yaml-button" class="alt" onclick="convertEditorToYaml()">Transfer to YAML</button>
          <button class="neutral" onclick="closeEditor()">Close Editor</button>
        </div>
      </div>
    </div>
  </div>
  <script>
    let persistTimer = null;
    let lastStatus = null;
    let editorOpen = false;
    let editorRecipe = '';
    let selectedRecipeKind = 'yaml';
    let selectedActiveRecipes = {cast: '', yaml: ''};
    let selectedDemoRecipes = new Set();

    async function fetchJson(url, options) {
      const response = await fetch(url, options);
      const data = await response.json();
      if (!response.ok) {
        throw new Error(data.error || response.statusText);
      }
      return data;
    }

    function setRecipeKind(kind) {
      selectedRecipeKind = ['cast', 'yaml'].includes(kind) ? kind : 'cast';
      refresh().catch((error) => console.error(error));
    }

    async function refresh() {
      const hadStatus = lastStatus !== null;
      const data = await fetchJson('/api/status');
      lastStatus = data;
      selectedActiveRecipes = {
        cast: data.active_recipes?.cast || '',
        yaml: data.active_recipes?.yaml || '',
      };
      if (!hadStatus && data.default_kind && ['cast', 'yaml'].includes(data.default_kind)) {
        selectedRecipeKind = data.default_kind;
      }
      selectedDemoRecipes = new Set(Array.isArray(data.demo_recipes) ? data.demo_recipes : []);
      const table = document.getElementById('recipe-table');
      table.innerHTML = '';
      const legacyScripts = (data.recipe_items || []).filter((item) => item.kind === 'script');
      const legacySelect = document.getElementById('legacy-script-select');
      const previousLegacyValue = legacySelect.value;
      legacySelect.innerHTML = '';
      for (const item of legacyScripts) {
        const option = document.createElement('option');
        option.value = item.name;
        option.textContent = item.name;
        legacySelect.appendChild(option);
      }
      if (legacyScripts.length === 0) {
        const option = document.createElement('option');
        option.value = '';
        option.textContent = '(no legacy scripts found)';
        legacySelect.appendChild(option);
        legacySelect.disabled = true;
      } else {
        legacySelect.disabled = false;
        legacySelect.value = legacyScripts.some((item) => item.name === previousLegacyValue)
          ? previousLegacyValue
          : legacyScripts[0].name;
      }
      document.getElementById('filter-cast').checked = selectedRecipeKind === 'cast';
      document.getElementById('filter-yaml').checked = selectedRecipeKind === 'yaml';
      document.getElementById('save-selection-button').disabled = false;
      document.getElementById('check-recipe-button').disabled = false;
      document.getElementById('run-app-button').disabled = false;
      document.getElementById('demo-button').disabled = false;
      document.getElementById('recipe-list-label').textContent =
        selectedRecipeKind === 'cast'
          ? 'Cast Recipe Files'
          : 'YAML Recipe Files';
      const visibleItems = (data.recipe_items || []).filter((item) => item.kind === selectedRecipeKind);
      for (const item of visibleItems) {
        const recipe = item.name;
        const row = document.createElement('tr');
        row.innerHTML = `
          <td class="compact"><input type="radio" name="active-recipe" value="${escapeHtml(recipe)}"></td>
          <td class="compact"><input type="checkbox" name="demo-recipe" value="${escapeHtml(recipe)}"></td>
          <td class="recipe-name">${escapeHtml(recipe)}</td>
        `;
        const activeInput = row.querySelector('input[type="radio"]');
        const demoInput = row.querySelector('input[type="checkbox"]');
        activeInput.checked = recipe === selectedActiveRecipes[selectedRecipeKind];
        demoInput.checked = selectedDemoRecipes.has(recipe);
        const selectable = item.kind === 'cast' || item.kind === 'yaml';
        activeInput.disabled = !selectable;
        demoInput.disabled = !selectable;
        activeInput.addEventListener('change', () => {
          selectedActiveRecipes[selectedRecipeKind] = recipe;
          queuePersistSelection();
        });
        demoInput.addEventListener('change', () => {
          if (demoInput.checked) {
            selectedDemoRecipes.add(recipe);
          } else {
            selectedDemoRecipes.delete(recipe);
          }
          queuePersistSelection();
        });
        table.appendChild(row);
      }
      document.getElementById('meta-root').textContent = 'Metadata root: ' + data.metadata_root;
      document.getElementById('dropbox-root').textContent = 'Dropbox root: ' + (data.dropbox_root || '(not detected)');
      document.getElementById('active-recipe').textContent =
        'Default active recipe: ' + (data.active_recipe || '(none)') + ` [${data.default_kind || 'yaml'}]`;
      document.getElementById('status-text').textContent = data.running ? 'Running' : 'Stopped';
      document.getElementById('status-dot').className = 'status-dot' + (data.running ? ' running' : '');
      document.getElementById('log').textContent = data.log || '[no output yet]';
      const demoButton = document.getElementById('demo-button');
      demoButton.textContent = data.demo_running ? 'Stop Demo' : 'Continuous Demo';
      renderCountdown();
      const timeoutInput = document.getElementById('demo-timeout');
      if (document.activeElement !== timeoutInput) {
        timeoutInput.value = String(data.demo_timeout_seconds || 10);
      }
      const availableEditorRecipes = new Set((data.recipe_items || []).map((item) => item.name));
      if (editorOpen && editorRecipe && !availableEditorRecipes.has(editorRecipe)) {
        closeEditor();
      }
    }

    function renderCountdown() {
      const countdown = document.getElementById('demo-countdown');
      if (!lastStatus || !lastStatus.demo_running) {
        countdown.textContent = '';
        return;
      }
      if (typeof lastStatus.demo_switch_at_unix_ms === 'number') {
        const remainingMs = Math.max(0, lastStatus.demo_switch_at_unix_ms - Date.now());
        const remainingSeconds = Math.ceil(remainingMs / 1000);
        countdown.textContent = `Switching from ${lastStatus.demo_current_recipe || 'current recipe'} in ${remainingSeconds}s`;
        return;
      }
      countdown.textContent = `Running ${lastStatus.demo_current_recipe || 'current recipe'} until manual exit`;
    }

    function escapeHtml(text) {
      return text
        .replaceAll('&', '&amp;')
        .replaceAll('<', '&lt;')
        .replaceAll('>', '&gt;')
        .replaceAll('"', '&quot;')
        .replaceAll("'", '&#39;');
    }

    function currentActiveRecipe() {
      const selected = selectedActiveRecipes[selectedRecipeKind];
      if (selected) {
        return selected;
      }
      const active = document.querySelector('input[name="active-recipe"]:checked');
      if (active) {
        return active.value;
      }
      if (lastStatus && lastStatus.active_recipes && lastStatus.active_recipes[selectedRecipeKind]) {
        return lastStatus.active_recipes[selectedRecipeKind];
      }
      return '';
    }

    function currentVisibleRecipe() {
      const active = document.querySelector('input[name="active-recipe"]:checked');
      if (active) {
        return active.value;
      }
      const firstVisible = document.querySelector('input[name="active-recipe"]');
      return firstVisible ? firstVisible.value : '';
    }

    function currentDemoRecipes() {
      return Array.from(selectedDemoRecipes);
    }

    function toggleAllDemo() {
      const inputs = Array.from(document.querySelectorAll('input[name="demo-recipe"]'));
      if (inputs.length === 0) {
        return;
      }
      const shouldCheckAll = inputs.some((input) => !input.checked);
      for (const input of inputs) {
        input.checked = shouldCheckAll;
        if (shouldCheckAll) {
          selectedDemoRecipes.add(input.value);
        } else {
          selectedDemoRecipes.delete(input.value);
        }
      }
      queuePersistSelection();
    }

    async function persistSelection(refreshAfter = false) {
      const recipe = currentActiveRecipe();
      if (!recipe) {
        throw new Error('No recipe is currently selected.');
      }
      const demoRecipes = currentDemoRecipes();
      await fetchJson('/api/active-recipe', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({recipe, kind: selectedRecipeKind, default_kind: selectedRecipeKind, demo_recipes: demoRecipes}),
      });
      if (refreshAfter) {
        await refresh();
      }
    }

    function queuePersistSelection() {
      if (persistTimer) {
        clearTimeout(persistTimer);
      }
      persistTimer = setTimeout(() => {
        persistTimer = null;
        persistSelection().catch((error) => console.error(error));
      }, 200);
    }

    async function saveRecipe() {
      await persistSelection(true);
    }

    async function runApp() {
      await persistSelection();
      await fetchJson('/api/run', {method: 'POST'});
      await refresh();
    }

    async function checkRecipe() {
      const recipe = currentActiveRecipe();
      if (!recipe) {
        return;
      }
      await fetchJson('/api/check-recipe', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({recipe}),
      });
      await refresh();
    }

    async function stopApp() {
      await fetchJson('/api/stop', {method: 'POST'});
      await refresh();
    }

    async function toggleDemo() {
      const isRunning = document.getElementById('demo-button').textContent === 'Stop Demo';
      const timeoutSeconds = Number(document.getElementById('demo-timeout').value || 10);
      const demoRecipes = currentDemoRecipes();
      await fetchJson('/api/continuous-demo', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({
          action: isRunning ? 'stop' : 'start',
          timeout_seconds: Number.isFinite(timeoutSeconds) && timeoutSeconds > 0 ? timeoutSeconds : 10,
          demo_recipes: demoRecipes,
        }),
      });
      await refresh();
    }

    async function openEditorForRecipe(recipe) {
      if (!recipe) {
        return;
      }
      const data = await fetchJson(`/api/recipe-content?recipe=${encodeURIComponent(recipe)}`);
      editorOpen = true;
      editorRecipe = recipe;
      document.getElementById('editor-card').classList.add('open');
      document.getElementById('editor-path').textContent = `Editing: ${data.path}`;
      document.getElementById('recipe-editor').value = data.content;
      const convertButton = document.getElementById('convert-yaml-button');
      convertButton.style.display = (recipe.endsWith('.cast') || recipe.endsWith('.script')) ? '' : 'none';
    }

    async function openEditor() {
      await openEditorForRecipe(currentVisibleRecipe());
    }

    async function openLegacyScript() {
      const recipe = document.getElementById('legacy-script-select').value;
      if (!recipe) {
        return;
      }
      await openEditorForRecipe(recipe);
    }

    function closeEditor() {
      editorOpen = false;
      editorRecipe = '';
      document.getElementById('editor-card').classList.remove('open');
      document.getElementById('editor-path').textContent = '';
      document.getElementById('recipe-editor').value = '';
    }

    async function saveEditor() {
      if (!editorRecipe) {
        return;
      }
      const content = document.getElementById('recipe-editor').value;
      await fetchJson('/api/recipe-content', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({recipe: editorRecipe, content}),
      });
      await refresh();
    }

    async function convertEditorToYaml() {
      if (!editorRecipe) {
        return;
      }
      if (!(editorRecipe.endsWith('.cast') || editorRecipe.endsWith('.script'))) {
        return;
      }
      const content = document.getElementById('recipe-editor').value;
      const data = await fetchJson('/api/convert-to-yaml', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({recipe: editorRecipe, content}),
      });
      selectedRecipeKind = 'yaml';
      await refresh();
      await openEditorForRecipe(data.recipe);
    }

    function findNextInEditor() {
      const editor = document.getElementById('recipe-editor');
      const needle = document.getElementById('editor-find').value;
      if (!needle) {
        return;
      }
      const haystack = editor.value;
      const startIndex = editor.selectionEnd || 0;
      let index = haystack.indexOf(needle, startIndex);
      if (index === -1 && startIndex > 0) {
        index = haystack.indexOf(needle, 0);
      }
      if (index === -1) {
        return;
      }
      editor.focus();
      editor.setSelectionRange(index, index + needle.length);
    }

    function replaceNextInEditor() {
      const editor = document.getElementById('recipe-editor');
      const needle = document.getElementById('editor-find').value;
      const replacement = document.getElementById('editor-replace').value;
      if (!needle) {
        return;
      }
      const selected = editor.value.slice(editor.selectionStart, editor.selectionEnd);
      if (selected === needle) {
        const before = editor.value.slice(0, editor.selectionStart);
        const after = editor.value.slice(editor.selectionEnd);
        const start = editor.selectionStart;
        editor.value = before + replacement + after;
        editor.focus();
        editor.setSelectionRange(start, start + replacement.length);
        return;
      }
      findNextInEditor();
    }

    function replaceAllInEditor() {
      const editor = document.getElementById('recipe-editor');
      const needle = document.getElementById('editor-find').value;
      const replacement = document.getElementById('editor-replace').value;
      if (!needle) {
        return;
      }
      editor.value = editor.value.split(needle).join(replacement);
      editor.focus();
    }

    refresh();
    setInterval(refresh, 1000);
    setInterval(renderCountdown, 200);
  </script>
</body>
</html>
"""


class Handler(BaseHTTPRequestHandler):
    def _send_json(self, payload: dict, status: int = 200) -> None:
        body = json.dumps(payload).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _read_json(self) -> dict:
        length = int(self.headers.get("Content-Length", "0"))
        raw = self.rfile.read(length) if length else b"{}"
        return json.loads(raw.decode("utf-8"))

    def do_GET(self) -> None:
        parsed = urlparse(self.path)
        if parsed.path == "/":
            body = HTML_PAGE.encode("utf-8")
            self.send_response(HTTPStatus.OK)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if parsed.path == "/api/status":
            try:
                active_state = read_active_state()
                payload = STATE.snapshot()
                payload["recipes"] = list_recipe_files()
                payload["recipe_items"] = list_recipe_items()
                payload["active_recipe"] = read_active_recipe()
                payload["active_recipes"] = active_state["active"]
                payload["default_kind"] = active_state["default_kind"]
                payload["metadata_root"] = str(detect_metadata_root())
                dropbox_root = detect_dropbox_root()
                payload["dropbox_root"] = str(dropbox_root) if dropbox_root is not None else ""
                self._send_json(payload)
            except Exception as exc:  # pragma: no cover - simple UI server
                self._send_json({"error": str(exc)}, status=500)
            return

        if parsed.path == "/api/recipe-content":
            try:
                params = parse_qs(parsed.query)
                recipe_name = params.get("recipe", [""])[0]
                path = recipe_file_path(recipe_name)
                if recipe_item_by_name(recipe_name)["kind"] not in VISIBLE_RECIPE_KINDS:
                    raise ValueError(f"Unsupported recipe file type: {recipe_name}")
                self._send_json({
                    "recipe": recipe_name,
                    "path": str(path),
                    "content": read_recipe_file(recipe_name),
                })
            except Exception as exc:  # pragma: no cover - simple UI server
                self._send_json({"error": str(exc)}, status=500)
            return

        self._send_json({"error": "Not found"}, status=404)

    def do_POST(self) -> None:
        try:
            if self.path == "/api/active-recipe":
                payload = self._read_json()
                active_state = write_active_recipe(
                    payload["recipe"],
                    kind=payload.get("kind"),
                    default_kind=payload.get("default_kind"),
                )
                set_demo_recipe_names(payload.get("demo_recipes", []))
                self._send_json({
                    "ok": True,
                    "active_recipe": resolved_default_active_recipe(active_state),
                    "active_recipes": active_state["active"],
                    "default_kind": active_state["default_kind"],
                })
                return

            if self.path == "/api/run":
                run_app()
                self._send_json({"ok": True})
                return

            if self.path == "/api/check-recipe":
                payload = self._read_json()
                self._send_json(check_recipe(payload["recipe"]))
                return

            if self.path == "/api/stop":
                stop_app()
                self._send_json({"ok": True})
                return

            if self.path == "/api/continuous-demo":
                payload = self._read_json()
                action = payload.get("action", "start")
                if action == "stop":
                    stop_continuous_demo()
                else:
                    set_demo_recipe_names(payload.get("demo_recipes", []))
                    start_continuous_demo(
                        float(payload.get("timeout_seconds", DEFAULT_DEMO_TIMEOUT_SECONDS) or DEFAULT_DEMO_TIMEOUT_SECONDS)
                    )
                self._send_json({"ok": True})
                return

            if self.path == "/api/recipe-content":
                payload = self._read_json()
                write_recipe_file(payload["recipe"], payload["content"])
                self._send_json({"ok": True})
                return

            if self.path == "/api/convert-to-yaml":
                payload = self._read_json()
                self._send_json(
                    convert_recipe_file_to_yaml(payload["recipe"], payload.get("content"))
                )
                return
        except Exception as exc:  # pragma: no cover - simple UI server
            self._send_json({"error": str(exc)}, status=500)
            return

        self._send_json({"error": "Not found"}, status=404)

    def log_message(self, format: str, *args) -> None:
        return


def main() -> None:
    host = os.environ.get("NPPM_CONTROL_HOST", "127.0.0.1")
    port = int(os.environ.get("NPPM_CONTROL_PORT", "8080"))
    server = ThreadingHTTPServer((host, port), Handler)
    print(f"NPPM control panel: http://{host}:{port}")
    server.serve_forever()


if __name__ == "__main__":
    main()
