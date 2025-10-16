import io
import unittest
from contextlib import redirect_stdout

import main


class MainTestCase(unittest.TestCase):
    def test_main_prints_greeting(self):
        buffer = io.StringIO()
        with redirect_stdout(buffer):
            main.main()
        self.assertEqual(
            buffer.getvalue().strip(),
            "Hello from isl-practice!",
            "メイン関数の出力が期待と異なります",
        )


if __name__ == "__main__":
    unittest.main()
