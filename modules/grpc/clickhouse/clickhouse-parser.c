/*
 * Copyright (c) 2024 Axoflow
 * Copyright (c) 2024 Attila Szakacs <attila.szakacs@axoflow.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include "driver.h"
#include "cfg-parser.h"
#include "clickhouse-grammar.h"
#include "grpc-parser.h"

extern int clickhouse_debug;

int clickhouse_parse(CfgLexer *lexer, LogDriver **instance, gpointer arg);

static CfgLexerKeyword clickhouse_keywords[] =
{
  GRPC_KEYWORDS,
  { "clickhouse", KW_CLICKHOUSE },
  { "database", KW_DATABASE },
  { "table", KW_TABLE },
  { "user", KW_USER },
  { "password", KW_PASSWORD },
  { "server_side_schema", KW_SERVER_SIDE_SCHEMA },
  { "json_var", KW_JSON_VAR },
  { NULL }
};

CfgParser clickhouse_parser =
{
#if SYSLOG_NG_ENABLE_DEBUG
  .debug_flag = &clickhouse_debug,
#endif
  .name = "clickhouse",
  .keywords = clickhouse_keywords,
  .parse = (gint (*)(CfgLexer *, gpointer *, gpointer)) clickhouse_parse,
  .cleanup = (void (*)(gpointer)) log_pipe_unref,
};

CFG_PARSER_IMPLEMENT_LEXER_BINDING(clickhouse_, CLICKHOUSE_, LogDriver **)
