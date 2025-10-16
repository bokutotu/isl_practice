import unittest

import islpy as isl

from src.isl_practice import level01_iteration_sets as lvl01


class Level01IterationSetsTest(unittest.TestCase):
    def test_canonical_intersection_simple_overlap(self):
        domain_a = "{ [i, j] : 0 <= i < 4 and 0 <= j < 4 }"
        domain_b = "{ [i, j] : i = j and 0 <= i < 6 }"
        result = lvl01.canonical_intersection(domain_a, domain_b)
        self.assertIsNotNone(result, "共通部分が存在するので None であってはならない")
        self.assertEqual(
            isl.Set(domain_a).intersect(isl.Set(domain_b)),
            isl.Set(result),
            "交差の結果が期待通りではありません",
        )

    def test_canonical_intersection_empty(self):
        domain_a = "{ [i] : 0 <= i <= 2 }"
        domain_b = "{ [i] : i >= 5 }"
        result = lvl01.canonical_intersection(domain_a, domain_b)
        self.assertIsNone(result, "空集合の場合は None を返してください")

    def test_eliminate_dim(self):
        domain = "{ [i, j] : 0 <= i < 4 and j = i + 1 }"
        projected = lvl01.eliminate_dim(domain, "j")
        self.assertIsNotNone(projected, "射影結果が空ではないため None を返さないでください")
        expected = isl.Set("{ [i] : 0 <= i < 4 }")
        self.assertEqual(expected, isl.Set(projected))

    def test_eliminate_dim_empty(self):
        domain = "{ [i, j] : i = j and i < j }"
        self.assertIsNone(
            lvl01.eliminate_dim(domain, "j"),
            "矛盾した制約で空集合になる場合は None を返してください",
        )

    def test_find_lexmin_point(self):
        domain = "{ [i, j] : 0 <= i < 3 and 0 <= j < 3 and i + j >= 2 }"
        self.assertEqual((0, 2), lvl01.find_lexmin_point(domain))

    def test_find_lexmin_point_empty(self):
        domain = "{ [i, j] : i > j and j > i }"
        self.assertIsNone(
            lvl01.find_lexmin_point(domain),
            "存在しない領域の場合は None を返してください",
        )


if __name__ == "__main__":
    unittest.main()
