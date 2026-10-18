import runpy
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(root / '.verify-helper'))
sys.argv = ['ci.py', 'judge']
runpy.run_path(str(root / '.verify-helper/ci.py'), run_name='__main__')
