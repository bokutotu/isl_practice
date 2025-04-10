#include <stdio.h>
#include <stdlib.h>

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

int main() {
    isl_ctx *ctx = isl_ctx_alloc();
    check_pointer(ctx, "Failed to allocate isl_ctx", NULL);

    // 1. ドメインの作成
    const char *domain_str = "{ S[i,j] : 0 <= i, j < 100 }";
    isl_union_set *domain = isl_union_set_read_from_str(ctx, domain_str);
    check_pointer(domain, "Failed to create domain", ctx);

    // 2. スケジュールの作成 (まずドメインから)
    isl_schedule *schedule = isl_schedule_from_domain(isl_union_set_copy(domain)); // ドメインをコピー
    check_pointer(schedule, "Failed to create schedule from domain", ctx);

    // --- ★ バンドノードを挿入 ---
    // スケジュールマップを作成: S[i,j] -> [i,j]
    const char *schedule_map_str = "{ S[i,j] -> [i,j] }";
    isl_union_map *schedule_map = isl_union_map_read_from_str(ctx, schedule_map_str);
    check_pointer(schedule_map, "Failed to create schedule map", ctx);

    // スケジュールマップを multi_union_pw_aff に変換
    // isl_multi_union_pw_aff_from_union_map は schedule_map の所有権を取る
    isl_multi_union_pw_aff *mupa = isl_multi_union_pw_aff_from_union_map(schedule_map);
    check_pointer(mupa, "Failed to convert schedule map to multi_union_pw_aff", ctx);

    // スケジュールに部分スケジュール (バンド) を挿入
    // isl_schedule_insert_partial_schedule は schedule と mupa の所有権を取り、新しい schedule を返す
    schedule = isl_schedule_insert_partial_schedule(schedule, mupa);
    check_pointer(schedule, "Failed to insert partial schedule (band)", ctx);

    // 3. ルートノード取得
    isl_schedule_node *root = isl_schedule_get_root(schedule);
    check_pointer(root, "Failed to get schedule root node", ctx);

    // 4. タイリング対象のバンドノード取得 (ドメインのすぐ下、挿入されたバンド)
    isl_schedule_node *band_node_orig = isl_schedule_node_child(root, 0); // root は変更されない
    check_pointer(band_node_orig, "Failed to get band node (after insertion)", ctx);
    // band_node_orig の所有権を持つ

    // --- ノードタイプの確認 (デバッグ用) ---
    enum isl_schedule_node_type node_type = isl_schedule_node_get_type(band_node_orig);
    if (node_type != isl_schedule_node_band) {
         fprintf(stderr, "Error: Node at child 0 is not a band node (type: %d)\n", node_type);
         // リソース解放処理
         isl_schedule_node_free(band_node_orig);
         isl_schedule_node_free(root);
         isl_schedule_free(schedule);
         isl_union_set_free(domain);
         isl_ctx_free(ctx);
         return 1;
    }

    // 5. タイルサイズを作成
    isl_space *band_space = isl_schedule_node_band_get_space(band_node_orig);
    check_pointer(band_space, "Failed to get band space", ctx);
    isl_multi_val *tile_sizes = isl_multi_val_zero(band_space); // band_space の所有権は tile_sizes に移る
    check_pointer(tile_sizes, "Failed to create multi_val for tile sizes", ctx);

    isl_val *v32 = isl_val_int_from_si(ctx, 32);
    check_pointer(v32, "Failed to create isl_val from int 32", ctx);

    tile_sizes = isl_multi_val_set_val(tile_sizes, 0, isl_val_copy(v32));
    check_pointer(tile_sizes, "Failed to set tile size for dim 0", ctx);
    tile_sizes = isl_multi_val_set_val(tile_sizes, 1, v32); // v32の所有権を渡す
    check_pointer(tile_sizes, "Failed to set tile size for dim 1", ctx);

    // 6. バンドノードをタイル化
    // isl_schedule_node_band_tile は band_node_orig と tile_sizes の所有権を取る
    isl_schedule_node *tiled_node = isl_schedule_node_band_tile(band_node_orig, tile_sizes);
    if (!tiled_node) {
        fprintf(stderr, "Failed to tile the schedule node band\n");
        // エラー発生時、band_node_origとtile_sizesは消費されたと仮定
        isl_schedule_node_free(root);
        isl_schedule_free(schedule);
        isl_union_set_free(domain);
        isl_ctx_free(ctx);
        return 1;
    }
    // band_node_orig と tile_sizes は消費された

    // 7. タイリングされた新しいスケジュールを取得
    isl_schedule *tiled_schedule = isl_schedule_node_get_schedule(tiled_node); // tiled_node は変更されない
    check_pointer(tiled_schedule, "Failed to get tiled schedule from node", ctx);

    // 8. 結果のスケジュールを表示
    printf("Original Schedule (with band inserted):\n");
    isl_printer *p = isl_printer_to_file(ctx, stdout);
    check_pointer(p, "Failed to create printer", ctx);
    p = isl_printer_set_output_format(p, ISL_FORMAT_ISL);
    p = isl_printer_print_schedule(p, schedule);
    printf("\n\nTiled Schedule:\n");
    p = isl_printer_print_schedule(p, tiled_schedule);
    p = isl_printer_flush(p);
    isl_printer_free(p);

    // 9. リソースの解放
    isl_schedule_node_free(tiled_node);
    isl_schedule_node_free(root);
    isl_schedule_free(schedule);
    isl_schedule_free(tiled_schedule);
    isl_union_set_free(domain);

    isl_ctx_free(ctx);

    return 0;
}
