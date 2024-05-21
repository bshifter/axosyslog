/*
 * Copyright (c) 2024 Balazs Scheidler <balazs.scheidler@axoflow.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */
#include "expr-plus.h"
#include "object-string.h"
#include "filterx-eval.h"
#include "scratch-buffers.h"

typedef struct FilterXOperatorPlus
{
  FilterXBinaryOp super;
} FilterXOperatorPlus;

static FilterXObject *
_eval(FilterXExpr *s)
{
  FilterXOperatorPlus *self = (FilterXOperatorPlus *) s;

  FilterXObject *lhs_object = filterx_expr_eval(self->super.lhs);
  if (!lhs_object)
    {
      return NULL;
    }

  FilterXObject *rhs_object = filterx_expr_eval(self->super.rhs);
  if (!rhs_object)
    {
      filterx_object_unref(lhs_object);
      return NULL;
    }

  if (filterx_object_is_type(lhs_object, &FILTERX_TYPE_NAME(string)) &&
      filterx_object_is_type(rhs_object, &FILTERX_TYPE_NAME(string)))
    {
      gsize lhs_len, rhs_len;
      const gchar *lhs_value = filterx_string_get_value(lhs_object, &lhs_len);
      const gchar *rhs_value = filterx_string_get_value(rhs_object, &rhs_len);
      GString *buffer = scratch_buffers_alloc();

      g_string_append_len(buffer, lhs_value, lhs_len);
      g_string_append_len(buffer, rhs_value, rhs_len);
      /* FIXME: support taking over the already allocated space */
      return filterx_string_new(buffer->str, buffer->len);
    }

  filterx_eval_push_error("operator+ only works on strings", s, NULL);
  filterx_object_unref(lhs_object);
  filterx_object_unref(rhs_object);
  return NULL;
}

FilterXExpr *
filterx_operator_plus_new(FilterXExpr *lhs, FilterXExpr *rhs)
{
  FilterXOperatorPlus *self = g_new0(FilterXOperatorPlus, 1);
  filterx_binary_op_init_instance(&self->super, lhs, rhs);
  self->super.super.eval = _eval;
  return &self->super.super;
}
