from pathlib import Path
from types import SimpleNamespace
import unittest


ROOT = Path(__file__).resolve().parent.parent
STATIC = ROOT / '.verify-helper/docs/static'


class MobileLayout(unittest.TestCase):
    def test_generated_assets_override_the_themes_touch_zoom_script(self):
        from onlinejudge_verify.documentation.build import load_static_files
        destination = ROOT / 'preview'
        files = load_static_files(site_render_config=SimpleNamespace(
            destination_dir=destination, static_dir=STATIC, config_yml={}))
        for name in ('assets/js/scale.fix.js', 'assets/css/library.css', '_includes/head-custom.html'):
            self.assertEqual(files[destination / name], (STATIC / name).read_bytes())

    def test_responsive_styles_load_in_the_theme_head(self):
        head = (STATIC / '_includes/head-custom.html').read_text()
        self.assertIn('/assets/css/library.css', head)
        self.assertIn('relative_url', head)
        self.assertNotIn('/assets/css/library.css', (STATIC / '_layouts/page.html').read_text())


if __name__ == '__main__':
    unittest.main()
