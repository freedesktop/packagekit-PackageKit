/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2008 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __PK_PACKAGE_IDS_H
#define __PK_PACKAGE_IDS_H

#include <glib-object.h>

G_BEGIN_DECLS

gchar		**pk_package_ids_from_array		(GPtrArray	*array);
gchar		**pk_package_ids_from_va_list		(const gchar	*package_id_first,
							 va_list	*args);
gboolean	 pk_package_ids_check			(gchar		**package_ids)
							 G_GNUC_WARN_UNUSED_RESULT;
gboolean	 pk_package_ids_print			(gchar		**package_ids);
guint		 pk_package_ids_size			(gchar		**package_ids);
gchar		*pk_package_ids_to_text			(gchar		**package_ids,
							 const gchar	*delimiter)
							 G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS

#endif /* __PK_PACKAGE_IDS_H */