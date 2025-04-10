#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <isl/ctx.h>
#include <isl/ast.h>
#include <isl/schedule.h>
#include <isl/ast_build.h>
#include <isl/id.h>
#include <isl/set.h>
#include <isl/union_set.h>
#include <isl/union_map.h>
#include <isl/val.h>
#include <isl/printer.h>


int main(int argc, char **argv) {
    isl_ctx *ctx = isl_ctx_alloc();
    if (!ctx) return 1;

    long n_val = 5;
    if (argc > 1) {
        n_val = strtol(argv[1], NULL, 10);
    }
    printf("Using n = %ld\n", n_val);

    const char *domain_str = "[n] -> { S[i] : 0 <= i < n }";
    isl_set *domain = isl_set_read_from_str(ctx, domain_str);
    if (!domain) goto error_ctx;

    isl_id *param_id = isl_id_alloc(ctx, "n", NULL);
    isl_val *n_isl_val = isl_val_int_from_si(ctx, n_val);
    if (!n_isl_val) {
        isl_id_free(param_id);
        goto error_ctx;
    }
    domain = isl_set_fix_val(domain, isl_dim_param, 0, n_isl_val);
    isl_id_free(param_id);
    if (!domain) goto error_set;


    isl_union_set *domain_us = isl_union_set_from_set(domain);

    isl_schedule *schedule = isl_schedule_from_domain(domain_us);
    if (!schedule) goto error_ctx;

    isl_ast_build *ast_build = isl_ast_build_alloc(ctx);
    if (!ast_build) goto error_schedule;


    printf("Building AST...\n");

    isl_ast_node *root = isl_ast_build_node_from_schedule(ast_build, schedule);
    if (!root) {
        fprintf(stderr, "Error building AST from schedule\n");
        isl_ast_build_free(ast_build);
        isl_ctx_free(ctx);
        return 1;
    }

    printf("Printing AST as C code...\n");

    isl_printer *printer = isl_printer_to_file(ctx, stdout);
    if (!printer) goto error_ast_node;
    printer = isl_printer_set_output_format(printer, ISL_FORMAT_C);
    printer = isl_printer_print_ast_node(printer, root);
    printf("\n");
    isl_printer_free(printer);


    isl_ast_node_free(root);
    isl_ast_build_free(ast_build);
    isl_ctx_free(ctx);

    return 0;

error_ast_node:
    isl_ast_node_free(root);
error_build:
    isl_ast_build_free(ast_build);
error_schedule:
    isl_schedule_free(schedule);
    goto error_ctx;
error_set:

error_ctx:
    isl_ctx_free(ctx);
    return 1;
}
