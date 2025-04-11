#include <stdio.h>
#include <stdlib.h> // For exit(), free()
#include <isl/ctx.h>
#include <isl/union_set.h>
#include <isl/union_map.h>
#include <isl/schedule.h>
#include <isl/schedule_node.h>
#include <isl/space.h>
#include <isl/val.h>
#include <isl/val.h>
#include <isl/aff.h>
#include <isl/printer.h>
#include <isl/ast_build.h>
#include <isl/ast.h>

// エラーチェックと終了を行うヘルパー関数
static void check_pointer(void *ptr, const char *msg, isl_ctx *ctx) {
    if (!ptr) {
        fprintf(stderr, "Error: %s\n", msg);
        if (ctx) {
             isl_ctx_free(ctx);
        }
        exit(EXIT_FAILURE);
    }
}

// ASTノードからCコード文字列を生成して表示するヘルパー関数
static void print_ast_node_as_c(isl_ast_node *node, isl_ctx *ctx) {
    isl_printer *p_c = isl_printer_to_str(ctx);
    check_pointer(p_c, "Failed to create C printer", ctx);
    p_c = isl_printer_set_output_format(p_c, ISL_FORMAT_C); // C言語形式に設定
    p_c = isl_printer_print_ast_node(p_c, node);
    check_pointer(p_c, "Failed to print AST node to C string", ctx);
    char *c_code = isl_printer_get_str(p_c);
    check_pointer(c_code, "Failed to get C string from printer", ctx);
    printf("%s\n", c_code);
    free(c_code); // 文字列を解放 (isl_printer_get_strが返す文字列はfreeが必要 [117])
    isl_printer_free(p_c); // プリンタを解放
}


int main() {
    isl_ctx *ctx = isl_ctx_alloc();
    check_pointer(ctx, "Failed to allocate isl_ctx", NULL);

    // --- ステップ 1-7: スケジュール作成とタイリング (前回と同様) ---
    const char *domain_str = "{ S[i,j] : 0 <= i, j < 100 }";
    isl_union_set *domain = isl_union_set_read_from_str(ctx, domain_str);
    check_pointer(domain, "Failed to create domain", ctx);

    isl_schedule *schedule = isl_schedule_from_domain(isl_union_set_copy(domain));
    check_pointer(schedule, "Failed to create schedule from domain", ctx);

    const char *schedule_map_str = "{ S[i,j] -> [i,j] }";
    isl_union_map *schedule_map = isl_union_map_read_from_str(ctx, schedule_map_str);
    check_pointer(schedule_map, "Failed to create schedule map", ctx);

    isl_multi_union_pw_aff *mupa = isl_multi_union_pw_aff_from_union_map(schedule_map);
    check_pointer(mupa, "Failed to convert schedule map to multi_union_pw_aff", ctx);

    schedule = isl_schedule_insert_partial_schedule(schedule, mupa);
    check_pointer(schedule, "Failed to insert partial schedule (band)", ctx);

    isl_schedule_node *root = isl_schedule_get_root(schedule);
    check_pointer(root, "Failed to get schedule root node", ctx);

    isl_schedule_node *band_node_orig = isl_schedule_node_child(root, 0);
    check_pointer(band_node_orig, "Failed to get band node (after insertion)", ctx);

    if (isl_schedule_node_get_type(band_node_orig) != isl_schedule_node_band) {
         fprintf(stderr, "Error: Node at child 0 is not a band node\n");
         // ... (省略: エラー時のリソース解放)
         return 1;
    }

    isl_space *band_space = isl_schedule_node_band_get_space(band_node_orig);
    check_pointer(band_space, "Failed to get band space", ctx);
    isl_multi_val *tile_sizes = isl_multi_val_zero(band_space);
    check_pointer(tile_sizes, "Failed to create multi_val for tile sizes", ctx);

    isl_val *v32 = isl_val_int_from_si(ctx, 32);
    check_pointer(v32, "Failed to create isl_val from int 32", ctx);

    tile_sizes = isl_multi_val_set_val(tile_sizes, 0, isl_val_copy(v32));
    check_pointer(tile_sizes, "Failed to set tile size for dim 0", ctx);
    tile_sizes = isl_multi_val_set_val(tile_sizes, 1, v32);
    check_pointer(tile_sizes, "Failed to set tile size for dim 1", ctx);

    isl_schedule_node *tiled_node = isl_schedule_node_band_tile(band_node_orig, tile_sizes);
    if (!tiled_node) {
        fprintf(stderr, "Failed to tile the schedule node band\n");
        // ... (省略: エラー時のリソース解放)
        return 1;
    }

    isl_schedule *tiled_schedule = isl_schedule_node_get_schedule(tiled_node);
    check_pointer(tiled_schedule, "Failed to get tiled schedule from node", ctx);

    // --- ステップ 8: スケジュールの中間表現を表示 ---
    printf("Original Schedule (with band inserted):\n");
    isl_printer *p = isl_printer_to_file(ctx, stdout);
    check_pointer(p, "Failed to create printer", ctx);
    p = isl_printer_set_output_format(p, ISL_FORMAT_ISL);
    p = isl_printer_print_schedule(p, schedule);
    printf("\n\nTiled Schedule:\n");
    p = isl_printer_print_schedule(p, tiled_schedule);
    p = isl_printer_flush(p);
    isl_printer_free(p);

    // --- ★ ステップ 8.5: AST生成とCコード出力 ---
    printf("\n--- AST Generation ---\n");

    // ASTビルドオブジェクトを作成 (パラメータコンテキストなし)
    isl_ast_build *build = isl_ast_build_alloc(ctx);
    check_pointer(build, "Failed to allocate AST build", ctx);

    // 元のスケジュールのASTを生成し、Cコードを出力
    printf("\nC code for Original Schedule:\n");
    // isl_ast_build_node_from_schedule は schedule の所有権を取らない (_isl_keep build)
    // しかし、安全のためコピーを渡すのが良い場合がある (buildが内部で変更する可能性)
    // マニュアル [2088] では build は _isl_keep, schedule は _isl_take となっているため、
    // schedule の所有権は関数に渡される。コピーが必要。
    isl_ast_node *ast_original = isl_ast_build_node_from_schedule(build, isl_schedule_copy(schedule));
    check_pointer(ast_original, "Failed to generate AST for original schedule", ctx);
    print_ast_node_as_c(ast_original, ctx);
    isl_ast_node_free(ast_original); // 生成したASTノードを解放

    // タイル化されたスケジュールのASTを生成し、Cコードを出力
    printf("\nC code for Tiled Schedule:\n");
    isl_ast_node *ast_tiled = isl_ast_build_node_from_schedule(build, isl_schedule_copy(tiled_schedule));
    check_pointer(ast_tiled, "Failed to generate AST for tiled schedule", ctx);
    print_ast_node_as_c(ast_tiled, ctx);
    isl_ast_node_free(ast_tiled); // 生成したASTノードを解放

    // ASTビルドオブジェクトを解放
    isl_ast_build_free(build);
    // --- ★ AST生成ここまで ---


    // 9. リソースの解放
    isl_schedule_node_free(tiled_node);
    isl_schedule_node_free(root);
    isl_schedule_free(schedule);
    isl_schedule_free(tiled_schedule);
    isl_union_set_free(domain);

    isl_ctx_free(ctx);

    return 0;
}
