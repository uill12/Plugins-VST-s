"""Small deterministic tests of the ChatGPT-first build selector. No Windows compilation is implied."""
import importlib.util
from pathlib import Path
from unittest import TestCase, main, mock

SRC = Path(__file__).with_name('rgnes_probe_build_env.py')
spec = importlib.util.spec_from_file_location('rgnes_probe_build_env', SRC)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class TestProbe(TestCase):
    def test_linux_no_mingw_does_not_claim_windows_compiler(self):
        with mock.patch.object(module.platform, 'system', return_value='Linux'), \
             mock.patch.object(module.shutil, 'which', return_value=None):
            result = module.probe()
        self.assertEqual(result['recommended_first_attempt'], 'external-windows-runner-needed')
        self.assertTrue(result['blockers_or_required_checks'])

    def test_mingw_cross_is_only_candidate(self):
        def available(tool):
            return '/usr/bin/'+tool if tool == 'x86_64-w64-mingw32-g++' else None
        with mock.patch.object(module.platform, 'system', return_value='Linux'), \
             mock.patch.object(module.shutil, 'which', side_effect=available):
            result = module.probe()
        self.assertEqual(result['recommended_first_attempt'], 'chatgpt-cross-mingw-candidate')
        self.assertIn('Verify MinGW-w64', ' '.join(result['blockers_or_required_checks']))

    def test_windows_compiler_detection(self):
        def available(tool):
            return 'C:/VC/cl.exe' if tool == 'cl' else None
        with mock.patch.object(module.platform, 'system', return_value='Windows'), \
             mock.patch.object(module.shutil, 'which', side_effect=available):
            result = module.probe()
        self.assertEqual(result['recommended_first_attempt'], 'chatgpt-native-windows')


if __name__ == '__main__':
    main()
