#include <stdio.h>
#include <stdlib.h>
#include <isl/ctx.h>
#include <isl/set.h>
#include <isl/union_set.h>
#include <isl/map.h>
#include <isl/union_map.h>
#include <isl/schedule.h>
#include <isl/schedule_node.h>
#include <isl/ast.h>
#include <isl/ast_build.h>
#include <isl/val.h>

/* バンド(ループバンド)に対してタイル化を行うヘルパー関数 */
static isl_schedule_node *tileBand(isl_schedule_node *node, int Ti, int Tj) {
    isl_ctx *ctx = isl_schedule_node_get_ctx(node);

    // タイルサイズを isl_multi_val に変換 (2次元バンド想定)
    isl_val *vTi = isl_val_int_from_si(ctx, Ti);
    isl_val *vTj = isl_val_int_from_si(ctx, Tj);

    // band の空間を取得
    isl_space *space = isl_schedule_node_band_get_space(node);
    isl_multi_val *tileSizes = isl_multi_val_zero(space);
    tileSizes = isl_multi_val_set_val(tileSizes, 0, vTi);
    tileSizes = isl_multi_val_set_val(tileSizes, 1, vTj);

    // バンドにタイル化を適用
    node = isl_schedule_node_band_tile(node, tileSizes);

    return node;
}

int main(void)
{
    isl_ctx *ctx = isl_ctx_alloc();

    // (A) ドメイン (例: [N,M] -> { [i,j]: 0<=i<N && 0<=j<M })
    const char *domainStr = "[N, M] -> { [i, j] : 0 <= i < N and 0 <= j < M }";
    isl_set *domain = isl_set_read_from_str(ctx, domainStr);
    isl_union_set *domainUnion = isl_union_set_from_set(domain);

    // (B) ドメイン全体のユニバースをとり、その Identity マップ(= i->i, j->j)を作る
    //     これはループ変数(i, j)をそのまま使う「単純なスケジュール」を表す。
    isl_union_set *domainUniverse = isl_union_set_universe(isl_union_set_copy(domainUnion));
    isl_union_map *identityUMap = isl_union_map_identity(domainUniverse);
    // これで [i,j] -> [i,j] という変換が定義される(バンド2次元)

    // (C) スケジュール生成
    //     from_domain_and_schedule(domain, identityUMap) により
    //     「domain → band(2次元)」のスケジュールツリーが作られる
    isl_schedule *schedule = isl_schedule_from_domain_and_schedule(
        isl_union_set_copy(domainUnion),
        identityUMap
    );

    // (D) スケジュールのルートノードを取得
    isl_schedule_node *node = isl_schedule_get_root(schedule);

    // デバッグ用: スケジュールツリー構造をダンプ
    // isl_schedule_node_dump(node);

    // domain ノードを取得し、子どもを見る
    if (isl_schedule_node_get_type(node) == isl_schedule_node_domain) {
        // 通常ここは domain → band という階層になっている
        node = isl_schedule_node_get_child(node, 0);
    }

    // ここで band のはず
    if (isl_schedule_node_get_type(node) != isl_schedule_node_band) {
        fprintf(stderr, "Error: Did not find a band node.\n");
        goto cleanup;
    }

    // タイル化
    node = tileBand(node, 32, 32);

    // タイル化したノードをスケジュールに反映
    schedule = isl_schedule_set_root(schedule, node);

    // (E) AST 化して C コード生成
    isl_set *context = isl_set_read_from_str(ctx, "[N, M] -> { : }");
    isl_ast_build *build = isl_ast_build_from_context(context);
    isl_ast_node *ast_node = isl_ast_build_node_from_schedule(build, schedule);
    char *codeStr = isl_ast_node_to_C_str(ast_node);
    if (codeStr) {
        printf("Generated Tiled Code:\n");
        printf("%s\n", codeStr);
        free(codeStr);
    }

cleanup:
    // 後処理
    isl_ast_node_free(ast_node);
    isl_ast_build_free(build);
    isl_schedule_free(schedule);
    isl_union_set_free(domainUnion);
    isl_ctx_free(ctx);
    return 0;
}

