"""Render RNS-PYPAL markdown reference documents to standalone HTML.

This intentionally small renderer covers the markdown subset used by the class
reference documents: headings, paragraphs, bullet lists, fenced code blocks,
inline code, and simple pipe tables. It avoids an external Markdown dependency
so the docs can be refreshed in a bare development environment.
"""

from __future__ import annotations

import html
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[1]
DOCS = [
    (ROOT / "docs" / "PPM_UNSIGNED_REFERENCE.md", ROOT / "docs" / "PPM_UNSIGNED_REFERENCE.html"),
    (ROOT / "docs" / "PPM_REFERENCE.md", ROOT / "docs" / "PPM_REFERENCE.html"),
    (ROOT / "docs" / "PPMDIGIT_REFERENCE.md", ROOT / "docs" / "PPMDIGIT_REFERENCE.html"),
    (ROOT / "docs" / "MIXED_RADIX_REFERENCE.md", ROOT / "docs" / "MIXED_RADIX_REFERENCE.html"),
    (ROOT / "docs" / "SPPM_REFERENCE.md", ROOT / "docs" / "SPPM_REFERENCE.html"),
    (ROOT / "docs" / "SPMF_REFERENCE.md", ROOT / "docs" / "SPMF_REFERENCE.html"),
]

STYLE = """:root{color-scheme:light dark;--bg:#f7f7fb;--panel:#fff;--text:#20242c;--muted:#5d6574;--line:#d9deea;--accent:#315cbd;--code:#eef1f7}@media(prefers-color-scheme:dark){:root{--bg:#111318;--panel:#1a1d24;--text:#eceff6;--muted:#aab2c2;--line:#333947;--accent:#8fb0ff;--code:#262b36}}body{margin:0;background:var(--bg);color:var(--text);font:16px/1.58 system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif}.page{display:grid;grid-template-columns:minmax(220px,300px) minmax(0,900px);gap:2rem;max-width:1280px;margin:0 auto;padding:2rem}nav{position:sticky;top:1rem;align-self:start;max-height:calc(100vh - 2rem);overflow:auto;background:var(--panel);border:1px solid var(--line);border-radius:16px;padding:1rem}nav h2{margin-top:0;font-size:1rem}nav ul{list-style:none;padding-left:0;margin:0}nav li{margin:.35rem 0}.toc-level-3{margin-left:1rem;font-size:.92rem}a{color:var(--accent);text-decoration:none}a:hover{text-decoration:underline}main{background:var(--panel);border:1px solid var(--line);border-radius:18px;padding:2rem 2.25rem;box-shadow:0 8px 28px rgba(0,0,0,.06)}h1,h2,h3{line-height:1.2}h1{margin-top:0;font-size:2.2rem}h2{margin-top:2.2rem;border-top:1px solid var(--line);padding-top:1.4rem}h3{margin-top:1.6rem;color:var(--muted)}table{border-collapse:collapse;width:100%;margin:1rem 0 1.4rem;font-size:.95rem}th,td{border:1px solid var(--line);padding:.55rem .7rem;vertical-align:top}th{text-align:left;background:var(--code)}code{background:var(--code);border-radius:5px;padding:.12rem .32rem;font-family:"Cascadia Code",Consolas,monospace;font-size:.92em}pre{background:var(--code);border:1px solid var(--line);border-radius:12px;padding:1rem;overflow:auto}pre code{background:transparent;padding:0}.meta{color:var(--muted);font-size:.95rem;margin-bottom:1.5rem}@media(max-width:900px){.page{grid-template-columns:1fr;padding:1rem}nav{position:static;max-height:none}main{padding:1.25rem}}"""


def slugify(text: str, used: set[str]) -> str:
    slug = re.sub(r"[^a-z0-9]+", "-", text.lower()).strip("-") or "section"
    base = slug
    n = 2
    while slug in used:
        slug = f"{base}-{n}"
        n += 1
    used.add(slug)
    return slug


def inline(text: str) -> str:
    escaped = html.escape(text)
    return re.sub(r"`([^`]+)`", lambda m: f"<code>{m.group(1)}</code>", escaped)


def split_table_row(line: str) -> list[str]:
    row = line.strip().strip("|")
    return [cell.strip() for cell in row.split("|")]


def is_table_separator(line: str) -> bool:
    cells = split_table_row(line)
    return bool(cells) and all(re.fullmatch(r":?-{3,}:?", cell) for cell in cells)


def render_markdown(markdown: str) -> tuple[str, list[tuple[int, str, str]]]:
    lines = markdown.splitlines()
    out: list[str] = []
    toc: list[tuple[int, str, str]] = []
    used: set[str] = set()
    i = 0

    while i < len(lines):
        stripped = lines[i].strip()

        if not stripped:
            i += 1
            continue

        if stripped.startswith("```"):
            language = stripped[3:].strip()
            i += 1
            code_lines = []
            while i < len(lines) and not lines[i].strip().startswith("```"):
                code_lines.append(lines[i])
                i += 1
            if i < len(lines):
                i += 1
            class_attr = f' class="language-{html.escape(language)}"' if language else ""
            out.append(f"<pre><code{class_attr}>" + html.escape("\n".join(code_lines)) + "</code></pre>")
            continue

        heading = re.match(r"^(#{1,3})\s+(.+)$", stripped)
        if heading:
            level = len(heading.group(1))
            text = heading.group(2).strip()
            toc_text = re.sub(r"`([^`]+)`", r"\1", text)
            anchor = slugify(toc_text, used)
            toc.append((level, anchor, toc_text))
            out.append(f'<h{level} id="{anchor}">{inline(text)}</h{level}>')
            i += 1
            continue

        if stripped.startswith("|") and i + 1 < len(lines) and is_table_separator(lines[i + 1]):
            headers = split_table_row(stripped)
            i += 2
            rows = []
            while i < len(lines) and lines[i].strip().startswith("|"):
                rows.append(split_table_row(lines[i]))
                i += 1
            table = ["<table><thead><tr>"]
            table.extend(f"<th>{inline(cell)}</th>" for cell in headers)
            table.append("</tr></thead><tbody>")
            for row in rows:
                table.append("<tr>")
                table.extend(f"<td>{inline(cell)}</td>" for cell in row)
                table.append("</tr>")
            table.append("</tbody></table>")
            out.append("".join(table))
            continue

        if stripped.startswith("- "):
            items = []
            while i < len(lines) and lines[i].strip().startswith("- "):
                items.append(lines[i].strip()[2:].strip())
                i += 1
            out.append("<ul>" + "".join(f"<li>{inline(item)}</li>" for item in items) + "</ul>")
            continue

        para = [stripped]
        i += 1
        while i < len(lines):
            nxt = lines[i].strip()
            if not nxt or nxt.startswith("#") or nxt.startswith("```") or nxt.startswith("- "):
                break
            if nxt.startswith("|") and i + 1 < len(lines) and is_table_separator(lines[i + 1]):
                break
            para.append(nxt)
            i += 1
        out.append(f"<p>{inline(' '.join(para))}</p>")

    return "\n".join(out), toc


def render_doc(md_path: Path, html_path: Path) -> None:
    markdown = md_path.read_text(encoding="utf-8")
    body, toc = render_markdown(markdown)
    title = toc[0][2] if toc else md_path.stem
    toc_items = "".join(
        f'<li class="toc-level-{level}"><a href="#{anchor}">{html.escape(text)}</a></li>'
        for level, anchor, text in toc
    )
    document = (
        '<!doctype html><html lang="en"><head><meta charset="utf-8">'
        '<meta name="viewport" content="width=device-width, initial-scale=1">'
        f"<title>{html.escape(title)}</title><style>{STYLE}</style></head><body>"
        '<div class="page"><nav aria-label="Table of contents"><h2>Contents</h2>'
        f"<ul>{toc_items}</ul></nav><main>"
        f'<div class="meta">Generated from <code>{html.escape(str(md_path.relative_to(ROOT)))}</code>.</div>'
        f"{body}</main></div></body></html>"
    )
    html_path.write_text(document, encoding="utf-8", newline="\n")


def main() -> None:
    for md_path, html_path in DOCS:
        render_doc(md_path, html_path)
        print(f"wrote {html_path.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
