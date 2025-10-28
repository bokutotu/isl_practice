import unittest

import islpy as isl

from src.isl_practice import level03_scheduling as lvl03


class Level03SchedulingTest(unittest.TestCase):
    def test_build_multiband_schedule_success(self):
        domain = "{ S[i, j] : 0 <= i < 4 and 0 <= j < 8 }"
        schedule_map = "{ S[i, j] -> [i, j] : 0 <= i < 4 and 0 <= j < 8 }"

        schedule = lvl03.build_multiband_schedule(
            domain,
            schedule_map,
            split_at=1,
            outer_coincident=(0,),
            inner_coincident=(),
        )

        self.assertIsInstance(schedule, isl.Schedule)

        root = schedule.get_root()
        outer_band = root.get_child(0)
        self.assertEqual(
            outer_band.get_type(),
            isl.schedule_node_type.band,
            "外側ノードが band になっていません",
        )
        self.assertEqual(
            outer_band.band_n_member(),
            1,
            "分割後の外側バンドの次元数が期待と異なります",
        )
        self.assertTrue(
            outer_band.band_member_get_coincident(0),
            "外側バンドの coincident 指定が反映されていません",
        )

        inner = outer_band.get_child(0)
        while inner.get_type() != isl.schedule_node_type.band:
            self.assertGreater(
                inner.n_children(),
                0,
                "内側バンドが見つかりませんでした",
            )
            inner = inner.get_child(0)

        self.assertEqual(
            inner.band_n_member(),
            1,
            "内側バンドの次元数が期待と異なります",
        )
        self.assertFalse(
            inner.band_member_get_coincident(0),
            "内側バンドに coincident 指定を与えていない場合は False のままにしてください",
        )

    def test_build_multiband_schedule_invalid_split(self):
        # 単一次元スケジュールは分割できないため None を返す
        domain = "{ S[i] : 0 <= i < 4 }"
        schedule_map = "{ S[i] -> [i] : 0 <= i < 4 }"
        self.assertIsNone(
            lvl03.build_multiband_schedule(domain, schedule_map, split_at=1),
            "分割不能なケースでは None を返してください",
        )

    def test_apply_band_tiling(self):
        domain = "{ S[i, j] : 0 <= i < 4 and 0 <= j < 4 }"
        schedule_map = "{ S[i, j] -> [i, j] : 0 <= i < 4 and 0 <= j < 4 }"
        schedule = lvl03.arrange_filters_with_strategy(
            domain,
            schedule_map,
            filters=(domain,),
            mode="fusion",
        )
        self.assertIsInstance(schedule, isl.Schedule)

        tiled = lvl03.apply_band_tiling(schedule, band_path=(0,), tile_sizes=(2, 2))
        self.assertIsInstance(tiled, isl.Schedule)

        outer_band = tiled.get_root().get_child(0)
        self.assertEqual(
            outer_band.band_n_member(),
            2,
            "タイル化後の外側バンドはタイル座標の 2 次元を持つはずです",
        )

        inner_band = outer_band.get_child(0)
        while inner_band.get_type() != isl.schedule_node_type.band:
            self.assertGreater(
                inner_band.n_children(),
                0,
                "タイル化で生成されるポイントバンドが見つかりません",
            )
            inner_band = inner_band.get_child(0)

        self.assertEqual(
            inner_band.band_n_member(),
            2,
            "ポイント空間のバンドも 2 次元である必要があります",
        )

    def test_arrange_filters_with_strategy(self):
        domain = "{ S[i] : 0 <= i < 4; T[i] : 0 <= i < 4 }"
        schedule_map = "{ S[i] -> [i] : 0 <= i < 4; T[i] -> [i] : 0 <= i < 4 }"
        filters = (
            "{ S[i] : 0 <= i < 4 }",
            "{ T[i] : 0 <= i < 4 }",
        )

        fused = lvl03.arrange_filters_with_strategy(
            domain,
            schedule_map,
            filters=filters,
            mode="fusion",
        )
        self.assertIsInstance(fused, isl.Schedule)
        self.assertEqual(
            fused.get_root().get_child(0).get_type(),
            isl.schedule_node_type.band,
            "fusion モードでは単一の band ノードを生成してください",
        )

        fissioned = lvl03.arrange_filters_with_strategy(
            domain,
            schedule_map,
            filters=filters,
            mode="fission",
        )
        self.assertIsInstance(fissioned, isl.Schedule)
        seq = fissioned.get_root().get_child(0)
        self.assertEqual(
            seq.get_type(),
            isl.schedule_node_type.sequence,
            "fission モードでは sequence ノードを用意してください",
        )
        self.assertEqual(
            seq.n_children(),
            2,
            "フィルタ数と sequence の子ノード数が一致していません",
        )
        for idx in range(seq.n_children()):
            child = seq.get_child(idx)
            self.assertEqual(
                child.get_type(),
                isl.schedule_node_type.filter,
                "sequence の各子は filter ノードである必要があります",
            )
            sub = child.get_child(0)
            while sub.get_type() != isl.schedule_node_type.band:
                self.assertGreater(
                    sub.n_children(),
                    0,
                    "フィルタ配下に band ノードが存在しません",
                )
                sub = sub.get_child(0)

    def test_collect_vectorization_candidates(self):
        domain = (
            "{ S[i, j] : 0 <= i < 2 and 0 <= j < 4; "
            "T[i, j] : 0 <= i < 2 and 0 <= j < 4 }"
        )
        schedule_map = (
            "{ S[i, j] -> [i, j] : 0 <= i < 2 and 0 <= j < 4; "
            "T[i, j] -> [i, j] : 0 <= i < 2 and 0 <= j < 4 }"
        )
        schedule = lvl03.build_multiband_schedule(
            domain,
            schedule_map,
            split_at=1,
            outer_coincident=(),
            inner_coincident=(0,),
        )
        self.assertIsInstance(schedule, isl.Schedule)

        result = lvl03.collect_vectorization_candidates(schedule)
        self.assertEqual(
            result,
            {"S": [(1, 0)], "T": [(1, 0)]},
            "coincident 情報の抽出結果が期待と一致しません",
        )

    def test_collect_vectorization_candidates_empty(self):
        domain = "{ S[i] : 0 <= i < 4 }"
        schedule_map = "{ S[i] -> [i] : 0 <= i < 4 }"
        schedule = lvl03.arrange_filters_with_strategy(
            domain,
            schedule_map,
            filters=(domain,),
            mode="fusion",
        )
        self.assertIsInstance(schedule, isl.Schedule)

        self.assertEqual(
            lvl03.collect_vectorization_candidates(schedule),
            {},
            "coincident 指定が無ければ空辞書を返してください",
        )


if __name__ == "__main__":
    unittest.main()
