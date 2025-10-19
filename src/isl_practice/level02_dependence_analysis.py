"""
Level 02 では、依存解析の基礎操作を isl で体験するための問題群を扱います。

ここで扱う読み／書きアクセスはどちらも「反復 → メモリ位置」の写像です。RAW
依存は「書いた反復 → 読む反復」を意味するので、`construct_flow_dependences` では
`write_accesses` を反転してから `read_accesses` に合成し、Write→Read の向きを
必ず守ってください。空集合を得た場合は None を返します。
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
    read = isl.UnionMap(read_accesses)
    write = isl.UnionMap(write_accesses)
    deps = read.apply_range(write.reverse()).reverse()
    if deps.is_empty():
        return None
    return deps.intersect_domain(isl.UnionSet(iteration_domain))


def simplify_dependence_domain(dependences: isl.UnionMap) -> isl.UnionMap | None:
    """
    依存多面体を簡約（`coalesce` や `detect_equalities`）し、正規化します。

    空集合の場合は None を返してください。
    """
    if dependences.is_empty():
        return None

    return dependences.coalesce().detect_equalities()


def compute_min_distance_vector(dependence: isl.Map) -> tuple[int, ...] | None:
    """
    単一依存写像から辞書式最小の距離ベクトル（整数タプル）を抽出します。

    有効な点が存在しない場合は None を返してください。
    """
    if dependence.is_empty():
        return None

    deltas = dependence.deltas()
    if deltas.is_empty():
        return None

    point = deltas.lexmin().sample_point()
    space = point.get_space()
    dims = space.dim(isl.dim_type.set)
    return tuple(
        point.get_coordinate_val(isl.dim_type.set, i).to_python()
        for i in range(dims)
    )


def validate_schedule_legality(
    dependences: isl.UnionMap,
    schedule: isl.Schedule | str,
) -> bool | None:
    """
    依存写像と候補スケジュールを照合し、合法性を真偽値で返します。

    判定不能な場合は None を返してください。
    """
    theta = None
    if isinstance(schedule, isl.Schedule):
        theta = schedule.get_map()
    elif isinstance(schedule, str):
        theta = isl.UnionMap(schedule)

    # src -> time
    theta_src = theta.intersect_domain(dependences.domain())
    # dst -> time
    theta_dst = theta.intersect_domain(dependences.range())

    # dependences :: src -> dst
    # ほしいのは、src_time -> dst_time
    # theta_src :: src -> src_time
    # theta_dst :: dst -> dst_time
    # (src_time -> src) と、dependencesを合成すれば、src_time -> dstになる。
    # src_time -> dstに dst -> dst_timeを合成すれば
    # src_time -> dst_timeになる。
    # apply_rangeは写像の合成と考えていい。
    deps = theta_src.reverse().apply_range(dependences).apply_range(theta_dst)

    deltas = deps.deltas()

    if deltas.is_empty():
        return None

    min_vec = deltas.lexmin()
    point = min_vec.sample_point()
    space = point.get_space()
    dims = space.dim(isl.dim_type.set)

    for i in range(dims):
        val = point.get_coordinate_val(isl.dim_type.set, i).to_python()
        if val < 0:
            return False
        if val > 0:
            return True

    return False
