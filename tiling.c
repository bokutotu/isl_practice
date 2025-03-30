#include <stdio.h>
#include <stdlib.h>

#include <isl/ctx.h>
#include <isl/ast.h>
#include <isl/schedule.h>
#include <isl/ast_build.h>
#include <isl/printer.h>
#include <isl/set.h>

int main(void) {
    // ISL コンテキストの作成
    isl_ctx *ctx = isl_ctx_alloc();
    if (!ctx) {
        fprintf(stderr, "Error allocating ISL context\n");
        return 1;
    }

    // 反復変数 i のループを表す整数集合の定義
    // 例: n がパラメータのとき、0 <= i < n のループ
    isl_set *domain = isl_set_read_from_str(ctx, "[n] -> { S[i] : 0 <= i < n }");
    if (!domain) {
        fprintf(stderr, "Error reading ISL set from string\n");
        isl_ctx_free(ctx);
        return 1;
    }

    // isl_schedule_from_domain() は isl_union_set を要求するので変換する
    isl_union_set *domain_us = isl_union_set_from_set(domain);

    // 与えられた反復空間に対するスケジュールを生成
    isl_schedule *schedule = isl_schedule_from_domain(domain_us);
    if (!schedule) {
        fprintf(stderr, "Error creating schedule from domain\n");
        isl_ctx_free(ctx);
        return 1;
    }

    // AST ビルドオブジェクトの作成
    isl_ast_build *ast_build = isl_ast_build_alloc(ctx);
    if (!ast_build) {
        fprintf(stderr, "Error allocating AST build\n");
        isl_schedule_free(schedule);
        isl_ctx_free(ctx);
        return 1;
    }

    // スケジュールから AST（ループの内部表現）を構築する
    isl_ast_node *root = isl_ast_build_node_from_schedule(ast_build, schedule);
    if (!root) {
        fprintf(stderr, "Error building AST from schedule\n");
        isl_ast_build_free(ast_build);
        isl_ctx_free(ctx);
        return 1;
    }

    // isl_printer を利用して AST を C コード形式で出力する
    isl_printer *printer = isl_printer_to_file(ctx, stdout);
    printer = isl_printer_set_output_format(printer, ISL_FORMAT_C);
    printer = isl_printer_print_ast_node(printer, root);
    printf("\n");

    // 後始末
    isl_printer_free(printer);
    isl_ast_node_free(root);
    isl_ast_build_free(ast_build);
    isl_ctx_free(ctx);

    return 0;
}

