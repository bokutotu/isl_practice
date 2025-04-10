#include <stdio.h>
#include <stdlib.h>
#include <isl/ctx.h>
#include <isl/set.h>
#include <isl/union_set.h>
#include <isl/schedule.h>
#include <isl/ast.h>
#include <isl/ast_build.h>
#include <isl/val.h>

/* バンド(ループバンド)に対してタイル化を行うヘルパー関数 */
static isl_schedule_node *tileBand(isl_schedule_node *node, int Ti, int Tj) {
    // node は band ノードであることを想定
    isl_ctx *ctx = isl_schedule_node_get_ctx(node);

    // タイルサイズを isl_multi_val に変換
    // 2次元バンドの場合、[Ti, Tj] のように作成
    isl_val *vTi = isl_val_int_from_si(ctx, Ti);
    isl_val *vTj = isl_val_int_from_si(ctx, Tj);
    isl_multi_val *tileSizes = isl_multi_val_zero(isl_schedule_node_band_get_space(node));
    tileSizes = isl_multi_val_set_val(tileSizes, 0, vTi);
    tileSizes = isl_multi_val_set_val(tileSizes, 1, vTj);

    // バンドにタイル化を適用
    node = isl_schedule_node_band_tile(node, tileSizes);

    return node;
}

int main(void)
{
    // (1) ISL コンテキストの生成
    isl_ctx *ctx = isl_ctx_alloc();

    // 例として、パラメータN, Mを持つ 2重ループ領域
    // [N, M] -> { [i, j] : 0 <= i < N and 0 <= j < M }
    const char *domainStr = "[N, M] -> { [i, j] : 0 <= i < N and 0 <= j < M }";

    // (2) 反復領域(Iteration Domain) の作成
    isl_set *domain = isl_set_read_from_str(ctx, domainStr);

    // パラメータ N, M をある具体的な値で固定したい場合は、以下のように設定する
    //   例: N=64, M=128
    //   isl_set *paramSet = isl_set_read_from_str(ctx, "{ : N=64 and M=128 }");
    //   domain = isl_set_intersect_params(domain, paramSet);
    // ここではあえてシンボリックなままとしておきます

    // (3) ドメインから初期スケジュールを生成（identity スケジュール）
    //     identity スケジュール = i, j という順序でイテレーションする単純なもの
    isl_union_set *domainUnion = isl_union_set_from_set(domain);
    isl_schedule *schedule = isl_schedule_from_domain(domainUnion);

    // (4) Schedule のルートノードを取り出す -> バンドノードをタイル化
    isl_schedule_node *root = isl_schedule_get_root(schedule);

    // バンドノードを探す（シンプルな場合は root が即バンドノードなことが多い）
    // ただし実際にはフィルターノードなどが入っている場合があるため、必要に応じて
    // isl_schedule_node_get_child() をたどることがある
    //
    // ここでは単純化のため、root がバンドノードであると仮定
    // （実際にはチェックや必要に応じたトラバースを行う必要があります）
    if (isl_schedule_node_get_type(root) == isl_schedule_node_band) {
        // タイルサイズを仮に (Ti=32, Tj=32) とする
        root = tileBand(root, 32, 32);
    } else {
        fprintf(stderr, "Error: Root node is not a band node.\n");
    }

    // タイル化したノードをスケジュールに戻す
    schedule = isl_schedule_set_root(schedule, root);

    // (5) タイル化したスケジュールから AST(抽象構文木) を生成
    //     まずはコンテキスト（パラメータなど）から isl_ast_build を作成
    isl_set *context = isl_set_read_from_str(ctx, "[N, M] -> { : }"); // 空制約 (N, M は未定義)
    isl_ast_build *build = isl_ast_build_from_context(context);

    // スケジュール全体をノード (ast_node) に落とし込む
    isl_ast_node *ast_node = isl_ast_build_node_from_schedule(build, schedule);

    // (6) 生成した AST(抽象構文木) を C コードとして文字列出力
    char *codeStr = isl_ast_node_to_C_str(ast_node);
    if (codeStr) {
        printf("Generated Tiled Code:\n");
        printf("%s\n", codeStr);
        free(codeStr);
    }

    // 後処理
    isl_ast_node_free(ast_node);
    isl_ast_build_free(build);
    isl_schedule_free(schedule);
    isl_ctx_free(ctx);

    return 0;
}
