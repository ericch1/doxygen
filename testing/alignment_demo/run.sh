#!/bin/sh
# Usage: ./run.sh [path/to/doxygen]
set -e
DOXYGEN=${1:-doxygen}
cd "$(dirname "$0")"
"$DOXYGEN" Doxyfile

echo "===== brief and detailed description, taken from the XML output ====="
python3 - <<'PY'
import xml.etree.ElementTree as ET, glob
for f in sorted(glob.glob('out/xml/class*.xml')):
    t = ET.parse(f)
    cls = t.getroot().find('compounddef').findtext('compoundname')
    for m in t.iter('memberdef'):
        def txt(tag):
            e = m.find(tag)
            return ' '.join(''.join(e.itertext()).split()) if e is not None else ''
        print(f"{cls+'::'+m.findtext('name'):<28} brief={txt('briefdescription')!r:<28} detailed={txt('detaileddescription')!r}")
PY

echo
echo "===== how the C++ comments are rewritten (-d commentcnv) ====="
"$DOXYGEN" -d commentcnv Doxyfile 2>&1 | awk '/CommentCnv: .*_lines\.h/,/^\]$/'
