import os
from pathlib import Path
import shutil

HERE = Path(os.environ.get('CP_SP_ROUND5_CODEGEN_WORK', Path(__file__).resolve().parent))
for name, style in [('destination', 'destination = {edge.to, edge.weight};'), ('destination_local', 'const Arc arc{edge.to, edge.weight};\n\t\t\tdestination = arc;')]:
    tree = HERE / name
    shutil.copytree(HERE / 'current', tree, dirs_exist_ok=True)
    header = tree / 'components/directed_csr.hpp'
    text = header.read_text()
    text = text.replace('arcs_[--offset_[edge.from]] = {edge.to, edge.weight};', 'auto &destination = arcs_[--offset_[edge.from]];\n\t\t\t' + style)
    header.write_text(text)
