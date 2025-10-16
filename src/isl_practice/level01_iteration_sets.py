"""
最初のステップとして、反復領域の基礎操作を isl で体験するための問題群です。

ここでは関数本体をあえて未実装（`return None`）にしてあります。テストが要求する
振る舞いを満たすように、`isl.Set` や `isl.BasicSet` などの API を使って実装を
書き換えてください。
"""

from __future__ import annotations
import islpy as isl


def canonical_intersection(domain_a: str, domain_b: str) -> str | None:
    """
    2 つの反復領域の交差を正規化した文字列で返します。

    テストは、この関数が `isl.Set.read_from_str` で再読込できる文字列表現を返し、
    かつ与えられた領域の厳密な共通部分を表していることを期待します。
    """
    domain_a_set = isl.Set(domain_a)
    domain_b_set = isl.Set(domain_b)

    out_domain = domain_a_set.intersect(domain_b_set)
    if out_domain.is_empty():
        return None
    return str(out_domain)


def eliminate_dim(domain: str, dimension: str) -> str | None:
    """
    指定した次元をドメインから射影除去します。

    `dimension` には除去したい軸名（例: `"j"`）が渡されます。結果の集合を文字列で
    返し、空集合になる場合は None を返してください。
    """
    domain_set = isl.Set(domain)
    idx = domain_set.get_space().find_dim_by_name(isl.dim_type.set, dimension)
    if idx < 0:
        return None
    projected = domain_set.project_out(isl.dim_type.set, idx, 1)
    if projected.is_empty():
        return None
    return str(projected)


def find_lexmin_point(domain: str) -> isl.Point | None:
    """
    与えられた領域に存在する辞書式最小の整数点を `isl.Point` として返します。

    isl のデフォルト順序での lexmin を想定しており、領域が空なら None を返します。
    """
    domain_set = isl.Set(domain).lexmin()
    if domain_set.is_empty():
        return None
    return domain_set.sample_point()
