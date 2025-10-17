import unittest

import islpy as isl

from src.isl_practice import level02_dependence_analysis as lvl02


class Level02DependenceAnalysisTest(unittest.TestCase):
    def test_construct_flow_dependences_simple_raw(self):
        iteration_domain = "{ S[i] : 0 <= i < 4 }"
        read_accesses = "{ S[i] -> A[i - 1] : 1 <= i <= 3 }"
        write_accesses = "{ S[i] -> A[i] : 0 <= i < 4 }"

        result = lvl02.construct_flow_dependences(
            iteration_domain,
            read_accesses,
            write_accesses,
        )

        self.assertIsInstance(result, isl.UnionMap)
        expected = isl.UnionMap.read_from_str(
            isl.DEFAULT_CONTEXT,
            "{ S[i] -> S[i + 1] : 0 <= i < 3 }",
        )
        self.assertTrue(
            result.is_equal(expected),
            "RAW 依存が期待通りに構築されていません",
        )

    def test_construct_flow_dependences_no_overlap(self):
        iteration_domain = "{ S[i] : 0 <= i < 3 }"
        read_accesses = "{ S[i] -> A[i] }"
        write_accesses = "{ S[i] -> B[i] }"

        result = lvl02.construct_flow_dependences(
            iteration_domain,
            read_accesses,
            write_accesses,
        )

        self.assertIsNone(
            result,
            "重なりの無いアクセスでは依存が存在しないため None を返してください",
        )

    def test_simplify_dependence_domain(self):
        dependences = isl.UnionMap.read_from_str(
            isl.DEFAULT_CONTEXT,
            "{ S[i] -> S[j] : j = i + 1 and 0 <= i < 3 and j <= 3 }",
        )

        simplified = lvl02.simplify_dependence_domain(dependences)

        self.assertIsInstance(simplified, isl.UnionMap)
        expected = dependences.coalesce().detect_equalities()
        self.assertTrue(
            simplified.is_equal(expected),
            "依存多面体の簡約結果が期待と一致しません",
        )

    def test_simplify_dependence_domain_empty(self):
        empty_dependence = isl.UnionMap.read_from_str(
            isl.DEFAULT_CONTEXT,
            "{ }",
        )

        self.assertIsNone(
            lvl02.simplify_dependence_domain(empty_dependence),
            "空の依存集合は None を返してください",
        )

    def test_compute_min_distance_vector(self):
        dependence = isl.Map.read_from_str(
            isl.DEFAULT_CONTEXT,
            "{ [i, j] -> [i + 1, j + 2] : 0 <= i < 4 and 0 <= j < 4 }",
        )

        self.assertEqual(
            (1, 2),
            lvl02.compute_min_distance_vector(dependence),
            "最小距離ベクトルが期待と異なります",
        )

    def test_compute_min_distance_vector_empty(self):
        empty_map = isl.Map.read_from_str(
            isl.DEFAULT_CONTEXT,
            "{ [i] -> [j] : 0 <= i < 1 and 0 <= j < 1 and i < 0 }",
        )

        self.assertIsNone(
            lvl02.compute_min_distance_vector(empty_map),
            "空写像は None を返してください",
        )

    def test_validate_schedule_legality_identity(self):
        dependences = isl.UnionMap.read_from_str(
            isl.DEFAULT_CONTEXT,
            "{ S[i] -> S[i + 1] : 0 <= i < 3 }",
        )
        schedule_map = "{ S[i] -> [i] : 0 <= i < 4 }"

        self.assertTrue(
            lvl02.validate_schedule_legality(dependences, schedule_map),
            "依存を保つスケジュールを合法と判定してください",
        )

    def test_validate_schedule_legality_violation(self):
        dependences = isl.UnionMap.read_from_str(
            isl.DEFAULT_CONTEXT,
            "{ S[i] -> S[i + 1] : 0 <= i < 3 }",
        )
        reversed_schedule = "{ S[i] -> [-i] : 0 <= i < 4 }"

        self.assertFalse(
            lvl02.validate_schedule_legality(dependences, reversed_schedule),
            "依存を壊すスケジュールは非法と判定してください",
        )


if __name__ == "__main__":
    unittest.main()
