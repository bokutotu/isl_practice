"""
Level 03 では、スケジュール木を直接操作するための問題群を扱います。

Level 01/02 と同様に、仕様はユニットテストで定義されています。ここでは
問題の意図を明確化するため、関数本体をあえて未実装ではなく骨組みから書いて
ありますが、必要に応じて isl の API を調べつつ読み進めてください。
"""

from __future__ import annotations

from collections import defaultdict
from typing import Iterable

import islpy as isl


def _all_indices_valid(indices: Iterable[int], size: int) -> bool:
    """ヘルパー: 指定された添字がバンドの次元範囲内かを検証します。"""
    raise NotImplementedError


def build_multiband_schedule(
    iteration_domain: str,
    schedule_map: str,
    split_at: int,
    outer_coincident: Iterable[int] = (),
    inner_coincident: Iterable[int] = (),
) -> isl.Schedule | None:
    """
    与えられたスケジュール写像を multi-band 構成に変換し、coincident 指示子を設定します。

    `split_at` が示す位置でバンドを分割し、外側バンドと内側バンドの coincident
    メンバーをそれぞれ `outer_coincident` / `inner_coincident` の添字集合で指定します。
    条件に合わない場合は None を返します。
    """
    raise NotImplementedError


def apply_band_tiling(
    schedule: isl.Schedule,
    band_path: Iterable[int],
    tile_sizes: Iterable[int],
) -> isl.Schedule | None:
    """
    指定したバンドにタイル化を適用し、タイル空間とポイント空間を明示化します。

    `band_path` はルートから子ノードへ辿るインデックス列であり、対象ノードが
    バンドでない場合や、`tile_sizes` の長さがバンド次元と一致しない場合は None を返します。
    """
    raise NotImplementedError


def arrange_filters_with_strategy(
    iteration_domain: str,
    schedule_map: str,
    filters: Iterable[str],
    mode: str,
) -> isl.Schedule | None:
    """
    ループの fusion/fission を制御するため、フィルタ列を与えてスケジュール木を構築します。

    - mode が `"fusion"` の場合: フィルタを挟まず、単一のバンドにすべてのステートメントを配置します。
    - mode が `"fission"` の場合: `filters` で与えた順序に従って `insert_sequence` で分割し、
      それぞれに部分スケジュールを挿入します。
    条件を満たせない場合は None を返します。
    """
    raise NotImplementedError


def collect_vectorization_candidates(
    schedule: isl.Schedule,
) -> dict[str, list[tuple[int, int]]]:
    """
    バンドに設定された coincident 情報を走査し、SIMD 化しやすい次元候補を抽出します。

    戻り値は `{ ステートメント名: [(バンド深さ, メンバー添字), ...] }` の辞書です。
    同じ候補は重複しないよう正規化され、深さ順にソートされます。
    """
    raise NotImplementedError
