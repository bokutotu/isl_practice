## 主要なデータ型

### 1. **isl_set / isl.Set（集合）**
整数点の集合を表現します。

**C言語:**
```c
isl_set *set = isl_set_read_from_str(ctx, 
    "{ [i, j] : 0 <= i < 10 and 0 <= j < 20 }");
```

**Python (islpy):**
```python
set = isl.Set("{ [i, j] : 0 <= i < 10 and 0 <= j < 20 }")
```

**主な操作:**
- C: `isl_set_union(set1, set2)` / Python: `set1.union(set2)`  
  同じ空間に属する集合を結合し、含まれる全整数点をまとめた集合を返す。
- C: `isl_set_intersect(set1, set2)` / Python: `set1.intersect(set2)`  
  共通部分のみを抽出する。反復領域の制約を追加で課したいときに使う。
- C: `isl_set_subtract(set1, set2)` / Python: `set1.subtract(set2)`  
  `set1` から `set2` の領域を除外する。境界処理や例外ケースの切り出しに便利。
- C: `isl_set_is_empty(set)` / Python: `set.is_empty()`  
  集合が空（合法な反復が存在しない）かを判定する。
- C: `isl_set_is_subset(set1, set2)` / Python: `set1.is_subset(set2)`  
  依存解析で「すべてのアクセスが別集合に含まれるか」を確認するときに使う。

### 2. **isl_map / isl.Map（写像/関係）**
入力空間から出力空間への関係を表現します。

**C言語:**
```c
isl_map *map = isl_map_read_from_str(ctx,
    "{ [i, j] -> [i+1, j+2] }");
```

**Python (islpy):**
```python
map = isl.Map("{ [i, j] -> [i+1, j+2] }")
```

**主な操作:**
- C: `isl_map_apply_domain(map, set)` / Python: `map.apply_domain(set)`  
  関係の入力側を集合でフィルタする。特定のループ反復のみに写像を適用したいときに使用。
- C: `isl_map_apply_range(map, set)` / Python: `map.apply_range(set)`  
  出力側を制限する。到達可能なメモリアドレスを特定領域内に絞り込む用途。
- C: `isl_map_compose(map1, map2)` / Python: `map1.compose(map2)`  
  2 つの関係を合成して依存チェーンを追跡する。たとえば load→store の連結。
- C: `isl_map_reverse(map)` / Python: `map.reverse()`  
  ドメインとレンジを入れ替える。逆依存（consumer→producer）を調べる際に便利。
- C: `isl_map_domain(map)` / Python: `map.domain()`  
  関係の入力側の集合を取り出す。アクセス元ループの反復領域を確認できる。
- C: `isl_map_range(map)` / Python: `map.range()`  
  関係の出力側の集合を取り出す。アクセス先（配列インデックスなど）の範囲を把握する。

### 3. **isl_basic_set / isl.BasicSet**
単一の制約条件の論理積で表現される基本集合。

**C言語:**
```c
isl_basic_set *bset = isl_basic_set_read_from_str(ctx,
    "{ [i] : 0 <= i < 10 }");
```

**Python (islpy):**
```python
bset = isl.BasicSet("{ [i] : 0 <= i < 10 }")
```

### 4. **isl_basic_map / isl.BasicMap**
単一の制約条件の論理積で表現される基本写像。

### 5. **isl_union_set / isl.UnionSet**
異なる次元の空間にまたがる集合の和集合。

単一の `isl.Set` では次元が揃っている必要があるが、`isl.UnionSet` なら `A[i]` と `B[i, j]` のように異なるタイルや配列を一度に扱える。依存解析や schedule 構築では、複数ステートメントの反復領域をまとめるために頻繁に利用する。

**C言語:**
```c
isl_union_set *uset = isl_union_set_read_from_str(ctx,
    "{ A[i] : 0 <= i < 10; B[i, j] : 0 <= i, j < 5 }");
```

**Python (islpy):**
```python
uset = isl.UnionSet("{ A[i] : 0 <= i < 10; B[i, j] : 0 <= i, j < 5 }")
```

### 6. **isl_union_map / isl.UnionMap**
異なる次元の空間にまたがる写像の和集合。

読み書きアクセスを一つのオブジェクトに集約し、`isl_union_access_info_compute_flow` などの高水準 API に渡すと RAW/WAR/WAW 依存を一括で解析できる。

**C言語:**
```c
isl_union_map *umap = isl_union_map_read_from_str(ctx,
    "{ A[i] -> B[i+1]; C[i,j] -> D[i,j+1] }");
```

**Python (islpy):**
```python
umap = isl.UnionMap("{ A[i] -> B[i+1]; C[i,j] -> D[i,j+1] }")
```

### 7. **isl_space / isl.Space**
`isl.Space` は集合や写像が生きる「座標系」です。パラメータ軸・集合軸・写像の入出力軸をどれだけ持ち、それぞれにどんな名前が付いているのかを一か所にまとめて管理します。`isl.Set` や `isl.Map` は必ず内部に `Space` を抱えており、`set.get_space()` / `map.get_space()` で同じインスタンスを取得できます。

`Space` が提供する代表的なメタ情報:
- `dim(isl.dim_type.param)` – パラメータ軸の本数（ループ境界など式の外側で共有される値）
- `dim(isl.dim_type.set)` – 集合側の本体次元数（`{ [i, j] : … }` なら 2）
- `dim(isl.dim_type.in_)` / `dim(isl.dim_type.out)` – 写像の入力・出力次元数
- `get_dim_name(type, pos)` / `find_dim_by_name(type, name)` – 軸に付けた名前の参照と検索

この座標系が一致していなければ、`intersect` や `union` はエラーになりますし、射影で消したい軸も特定できません。したがって isl で計算する際は、まず `get_space()` で対象の軸タイプと位置を確認し、それに対応した API（例: 集合軸なら `project_out(isl.dim_type.set, …)`、パラメータ軸なら `project_out_param_id`）を選ぶ、という流れを徹底することになります。

**C言語:**
```c
isl_space *space = isl_space_alloc(ctx, 1, 2, 3);
// 1個のパラメータ、2次元の入力、3次元の出力
space = isl_space_set_dim_name(space, isl_dim_param, 0, "N");
space = isl_space_set_dim_name(space, isl_dim_set, 0, "i");
```

**Python (islpy):**
```python
space = isl.Space.alloc(1, 2, 0)
space = space.set_dim_name(isl.dim_type.param, 0, "N")
space = space.set_dim_name(isl.dim_type.set, 0, "i")
```

### 8. **isl_point / isl.Point**
特定の整数座標点を表現。

### 9. **isl_ctx / コンテキスト**
ISLの全操作に必要なグローバルコンテキスト。

**C言語:**
```c
isl_ctx *ctx = isl_ctx_alloc();
// ... 操作 ...
isl_ctx_free(ctx);
```

**Python (islpy):**
Pythonでは通常自動管理されますが、明示的にも作成可能:
```python
ctx = isl.Context()
```

## よく使われる操作例

### 集合の交差

**C言語:**
```c
isl_set *s1 = isl_set_read_from_str(ctx, "{ [i] : 0 <= i < 10 }");
isl_set *s2 = isl_set_read_from_str(ctx, "{ [i] : 5 <= i < 15 }");
isl_set *intersection = isl_set_intersect(s1, s2);
// 結果: { [i] : 5 <= i < 10 }
isl_set_free(intersection);
```

**Python (islpy):**
```python
s1 = isl.Set("{ [i] : 0 <= i < 10 }")
s2 = isl.Set("{ [i] : 5 <= i < 15 }")
intersection = s1.intersect(s2)
# 結果: { [i] : 5 <= i < 10 }
```

### 写像の適用

**C言語:**
```c
isl_map *m = isl_map_read_from_str(ctx, "{ [i] -> [2*i] }");
isl_set *domain = isl_set_read_from_str(ctx, "{ [i] : 0 <= i < 5 }");
isl_set *image = isl_set_apply(domain, m);
// 結果: { [j] : j = 0, 2, 4, 6, 8 }
isl_set_free(image);
```

**Python (islpy):**
```python
m = isl.Map("{ [i] -> [2*i] }")
domain = isl.Set("{ [i] : 0 <= i < 5 }")
image = domain.apply(m)
# 結果: { [j] : j = 0, 2, 4, 6, 8 }
```

## メモリ管理の違い

**C言語:**
- 明示的にメモリ解放が必要: `isl_set_free()`, `isl_map_free()`, `isl_ctx_free()`
- 参照カウント方式: `isl_set_copy()` で参照を増やす

**Python (islpy):**
- ガベージコレクションで自動管理
- メモリ解放を気にする必要なし
