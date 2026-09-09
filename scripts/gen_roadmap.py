"""Generate the SCORE portfolio-optimization roadmap as an Excalidraw board.

Declarative: edit ``scripts/roadmap_boards.json`` and re-run. Hand-edits to the
emitted ``.excalidraw`` will be overwritten on the next run.

    python scripts/gen_roadmap.py            # regenerate roadmap/ROADMAP.excalidraw
    python scripts/gen_roadmap.py --selftest # validate structure without writing

The board is laid out as vertical "phase" columns of rounded-rectangle cards,
with bound text and elbow arrows encoding the dependency graph. Colors come from
the Excalidraw open-color palette so it renders identically in the app.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SPEC = ROOT / "scripts" / "roadmap_boards.json"
OUT = ROOT / "roadmap" / "ROADMAP.excalidraw"

# open-color (stroke, fill) pairs, keyed by the friendly names used in the spec.
PALETTE = {
    "gray": ("#495057", "#dee2e6"),
    "blue": ("#1971c2", "#a5d8ff"),
    "violet": ("#6741d9", "#d0bfff"),
    "green": ("#2f9e44", "#b2f2bb"),
    "orange": ("#e8590c", "#ffd8a8"),
}

# Layout geometry (px).
COL_W = 340
COL_GAP = 80
CARD_W = 300
CARD_H = 132
CARD_GAP = 56
TOP = 220           # y of the first card row
HEAD_Y = 150        # y of column heading text
TITLE_Y = 40


def _seed(n: int) -> int:
    """Deterministic pseudo-seed so re-runs produce identical files (git-clean)."""
    return (n * 2654435761) % 2147483647


class _Builder:
    def __init__(self) -> None:
        self.elements: list[dict] = []
        self._n = 0
        self.card_bounds: dict[str, tuple[float, float, float, float]] = {}

    def _id(self, prefix: str) -> str:
        self._n += 1
        return f"{prefix}{self._n:04d}"

    def text_line(self, x, y, text, size=28, color="#1e1e1e", align="left", width=None):
        eid = self._id("t")
        w = width if width is not None else max(120, int(len(text) * size * 0.55))
        self.elements.append({
            "id": eid, "type": "text", "x": x, "y": y,
            "width": w, "height": size * 1.25, "angle": 0,
            "strokeColor": color, "backgroundColor": "transparent",
            "fillStyle": "solid", "strokeWidth": 1, "strokeStyle": "solid",
            "roughness": 0, "opacity": 100, "groupIds": [], "frameId": None,
            "roundness": None, "seed": _seed(self._n), "version": 1,
            "versionNonce": _seed(self._n), "isDeleted": False, "boundElements": [],
            "updated": 1, "link": None, "locked": False,
            "text": text, "originalText": text, "fontSize": size,
            "fontFamily": 3, "textAlign": align, "verticalAlign": "top",
            "lineHeight": 1.25, "containerId": None,
        })
        return eid

    def card(self, cid, x, y, title, body, color, done=False):
        stroke, fill = PALETTE[color]
        rect_id = f"r_{cid}"
        txt_id = f"tx_{cid}"
        label = ("✅ " if done else "") + title + "\n\n" + body
        self.elements.append({
            "id": rect_id, "type": "rectangle", "x": x, "y": y,
            "width": CARD_W, "height": CARD_H, "angle": 0,
            "strokeColor": stroke, "backgroundColor": fill,
            "fillStyle": "solid", "strokeWidth": 2, "strokeStyle": "solid",
            "roughness": 0, "opacity": 100, "groupIds": [], "frameId": None,
            "roundness": {"type": 3}, "seed": _seed(hash(cid) & 0xFFFFFF), "version": 1,
            "versionNonce": _seed(hash(cid) & 0xFFFFFF), "isDeleted": False,
            "boundElements": [{"type": "text", "id": txt_id}],
            "updated": 1, "link": None, "locked": False,
        })
        self.elements.append({
            "id": txt_id, "type": "text", "x": x + 12, "y": y + 12,
            "width": CARD_W - 24, "height": CARD_H - 24, "angle": 0,
            "strokeColor": "#1e1e1e", "backgroundColor": "transparent",
            "fillStyle": "solid", "strokeWidth": 1, "strokeStyle": "solid",
            "roughness": 0, "opacity": 100, "groupIds": [], "frameId": None,
            "roundness": None, "seed": _seed(hash(txt_id) & 0xFFFFFF), "version": 1,
            "versionNonce": _seed(hash(txt_id) & 0xFFFFFF), "isDeleted": False,
            "boundElements": [], "updated": 1, "link": None, "locked": False,
            "text": label, "originalText": label, "fontSize": 15,
            "fontFamily": 3, "textAlign": "center", "verticalAlign": "middle",
            "lineHeight": 1.25, "containerId": rect_id,
        })
        self.card_bounds[cid] = (x, y, CARD_W, CARD_H)

    def arrow(self, src, dst):
        sx, sy, sw, sh = self.card_bounds[src]
        dx, dy, dw, dh = self.card_bounds[dst]
        # exit right-center of src, enter left-center of dst (or top if same column-ish)
        start = (sx + sw, sy + sh / 2)
        if abs(dx - sx) < 5:  # same column -> vertical link
            start = (sx + sw / 2, sy + sh)
            end = (dx + dw / 2, dy)
        else:
            end = (dx, dy + dh / 2)
        aid = self._id("a")
        self.elements.append({
            "id": aid, "type": "arrow", "x": start[0], "y": start[1],
            "width": end[0] - start[0], "height": end[1] - start[1], "angle": 0,
            "strokeColor": "#868e96", "backgroundColor": "transparent",
            "fillStyle": "solid", "strokeWidth": 2, "strokeStyle": "solid",
            "roughness": 0, "opacity": 100, "groupIds": [], "frameId": None,
            "roundness": {"type": 2}, "seed": _seed(self._n + 7), "version": 1,
            "versionNonce": _seed(self._n + 7), "isDeleted": False, "boundElements": [],
            "updated": 1, "link": None, "locked": False,
            "points": [[0, 0], [end[0] - start[0], end[1] - start[1]]],
            "lastCommittedPoint": None,
            "startBinding": {"elementId": f"r_{src}", "focus": 0, "gap": 6},
            "endBinding": {"elementId": f"r_{dst}", "focus": 0, "gap": 6},
            "startArrowhead": None, "endArrowhead": "arrow",
            "elbowed": False,
        })


def build(spec: dict) -> dict:
    b = _Builder()
    b.text_line(90, TITLE_Y, spec["title"], size=34, color="#1e1e1e")
    b.text_line(90, TITLE_Y + 48, spec["subtitle"], size=18, color="#495057")

    for ci, col in enumerate(spec["columns"]):
        x = 90 + ci * (COL_W + 0)
        b.text_line(x, HEAD_Y, col["heading"], size=20, color=PALETTE[col["color"]][0], width=COL_W - 20)
        y = TOP
        for node in col["nodes"]:
            b.card(node["id"], x, y, node["title"], node["body"],
                   col["color"], done=node.get("done", False))
            y += CARD_H + CARD_GAP

    for src, dst in spec["edges"]:
        b.arrow(src, dst)

    return {
        "type": "excalidraw", "version": 2, "source": "score-roadmap-generator",
        "elements": b.elements,
        "appState": {"gridSize": None, "viewBackgroundColor": "#ffffff"},
        "files": {},
    }


def _validate(spec: dict, doc: dict) -> None:
    ids = {n["id"] for col in spec["columns"] for n in col["nodes"]}
    for src, dst in spec["edges"]:
        assert src in ids, f"edge source {src!r} is not a known node"
        assert dst in ids, f"edge dest {dst!r} is not a known node"
    rects = {e["id"] for e in doc["elements"] if e["type"] == "rectangle"}
    for cid in ids:
        assert f"r_{cid}" in rects, f"card {cid} missing rectangle"
    # every bound text must point at a real container
    by_id = {e["id"]: e for e in doc["elements"]}
    for e in doc["elements"]:
        if e["type"] == "text" and e.get("containerId"):
            assert e["containerId"] in by_id, f"text {e['id']} dangling container"


def main(argv=None) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--selftest", action="store_true",
                    help="validate structure, do not write the file")
    args = ap.parse_args(argv)

    spec = json.loads(SPEC.read_text())
    doc = build(spec)
    _validate(spec, doc)

    if args.selftest:
        n_cards = sum(len(c["nodes"]) for c in spec["columns"])
        print(f"selftest OK: {n_cards} cards, {len(spec['edges'])} edges, "
              f"{len(doc['elements'])} elements")
        return 0

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(doc, indent=2))
    print(f"wrote {OUT.relative_to(ROOT)}  ({len(doc['elements'])} elements)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
