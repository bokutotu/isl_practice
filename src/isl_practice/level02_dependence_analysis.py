"""
Level 02 では、依存解析の基礎操作を isl で体験するための問題群を扱います。

ここで定義する関数はすべて未実装（`return None`）のままです。テストが期待する
振る舞いを参考に、`isl.UnionMap` や `isl.Schedule` の API を用いた本実装へと
書き換えてください。各関数では順に、依存関係の構築、依存多面体の簡約、最小
距離ベクトルの抽出、スケジュール合法性の検証を担う想定です。
"""

from __future__ import annotations

import islpy as isl


def construct_flow_dependences(
    iteration_domain: str,
    read_accesses: str,
    write_accesses: str,
) -> isl.UnionMap | None:
    """
    読み／書きアクセス情報からフロー依存を `isl.UnionMap` として構築します。

    依存が存在しない場合は None を返してください。
    """
    domain = isl.Set(iteration_domain)
    read = isl.Map(read_accesses)
    write = isl.Map(write_accesses)
    return read.apply_range(write).apply_domain(domain)


def simplify_dependence_domain(dependences: isl.UnionMap) -> isl.UnionMap | None:
    """
    依存多面体を簡約（`coalesce` や `detect_equalities`）し、正規化します。

    空集合の場合は None を返してください。
    """

    return None


def compute_min_distance_vector(dependence: isl.Map) -> tuple[int, ...] | None:
    """
    単一依存写像から辞書式最小の距離ベクトル（整数タプル）を抽出します。

    有効な点が存在しない場合は None を返してください。
    """

    return None


def validate_schedule_legality(
    dependences: isl.UnionMap,
    schedule: isl.Schedule | str,
) -> bool | None:
    """
    依存写像と候補スケジュールを照合し、合法性を真偽値で返します。

    判定不能な場合は None を返してください。
    """

    return None
