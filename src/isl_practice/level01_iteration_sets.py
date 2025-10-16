"""
最初のステップとして、反復領域の基礎操作を isl で体験するための問題群です。

ここでは関数本体をあえて未実装（`return None`）にしてあります。テストが要求する
振る舞いを満たすように、`isl.Set` や `isl.BasicSet` などの API を使って実装を
書き換えてください。
"""

from __future__ import annotations


def canonical_intersection(domain_a: str, domain_b: str) -> str | None:
    """
    2 つの反復領域の交差を正規化した文字列で返します。

    テストは、この関数が `isl.Set.read_from_str` で再読込できる文字列表現を返し、
    かつ与えられた領域の厳密な共通部分を表していることを期待します。
    """
    return None


def eliminate_dim(domain: str, dimension: str) -> str | None:
    """
    指定した次元をドメインから射影除去します。

    `dimension` には除去したい軸名（例: `"j"`）が渡されます。結果の集合を文字列で
    返し、空集合になる場合は None を返してください。
    """
    return None


def find_lexmin_point(domain: str) -> tuple[int, ...] | None:
    """
    与えられた領域に存在する辞書式最小の整数点を返します。

    isl のデフォルト順序での lexmin を想定しており、領域が空なら None を返します。
    """
    return None
