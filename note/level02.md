ISLでのread/write依存解析

## 主要なデータ構造

- **isl_union_map / isl.UnionMap**: read/writeアクセスとスケジュールを表現
- **依存の種類**: RAW (flow), WAR (anti), WAW (output)

## 基本的な使い方

### アクセス関係とスケジュールの定義

**C:**
```c
isl_union_map *writes = isl_union_map_read_from_str(ctx,
    "{ S0[i] -> A[i]; S1[i] -> A[i-1] }");
isl_union_map *reads = isl_union_map_read_from_str(ctx,
    "{ S1[i] -> A[i]; S2[j] -> A[j+1] }");
isl_union_map *schedule = isl_union_map_read_from_str(ctx,
    "{ S0[i] -> [0,i]; S1[i] -> [1,i]; S2[j] -> [2,j] }");
```

**Python:**
```python
writes = isl.UnionMap("{ S0[i] -> A[i]; S1[i] -> A[i-1] }")
reads = isl.UnionMap("{ S1[i] -> A[i]; S2[j] -> A[j+1] }")
schedule = isl.UnionMap("{ S0[i] -> [0,i]; S1[i] -> [1,i]; S2[j] -> [2,j] }")
```

### 依存関係の計算

**C:**
```c
// RAW (flow)
isl_union_map *raw = isl_union_map_apply_range(
    isl_union_map_reverse(isl_union_map_copy(writes)), 
    isl_union_map_copy(reads));

// WAR (anti)
isl_union_map *war = isl_union_map_apply_range(
    isl_union_map_reverse(isl_union_map_copy(reads)),
    isl_union_map_copy(writes));

// WAW (output)
isl_union_map *waw = isl_union_map_apply_range(
    isl_union_map_reverse(isl_union_map_copy(writes)),
    isl_union_map_copy(writes));
```

**Python:**
```python
# RAW (flow)
raw = writes.reverse().apply_range(reads)

# WAR (anti)
war = reads.reverse().apply_range(writes)

# WAW (output)
waw = writes.reverse().apply_range(writes)
```

### スケジュールによる時間順序の確認

**C:**
```c
isl_union_map *timed = isl_union_map_apply_domain(
    isl_union_map_copy(raw), isl_union_map_copy(schedule));
timed = isl_union_map_apply_range(timed, schedule);
isl_union_map *valid = isl_union_map_lex_lt_union_map(timed);
```

**Python:**
```python
timed = raw.apply_domain(schedule).apply_range(schedule)
valid = timed.lex_lt_union_map(timed)
```

## 主要関数

| 機能 | C | Python |
|------|---|--------|
| 逆写像 | `isl_union_map_reverse()` | `.reverse()` |
| 合成 | `isl_union_map_apply_range()` | `.apply_range()` |
| 定義域適用 | `isl_union_map_apply_domain()` | `.apply_domain()` |
| 辞書式比較 | `isl_union_map_lex_lt_union_map()` | `.lex_lt_union_map()` |
| 和集合 | `isl_union_map_union()` | `.union()` |

## 簡単な例

```c
// for (i = 0; i < N; i++) {
//   A[i] = ...;      // S0[i]
//   ... = A[i-1];    // S1[i]
// }

writes = "{ S0[i] -> A[i] }"
reads = "{ S1[i] -> A[i-1] }"
schedule = "{ S0[i] -> [i,0]; S1[i] -> [i,1] }"

raw_deps = writes.reverse().apply_range(reads)
// 結果: { S0[i] -> S1[i+1] }
```
